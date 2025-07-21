#include "Server.hpp"
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <cerrno>
#include "Numerics.hpp"

Server* Server::_instance = NULL;

Server::Server(int port, const std::string& password)
    : _port(port), _password(password), _serverFileDescriptor(-1),
      _channelManager(), _running(true)
{
    setInstance(this);
}

Server::~Server()
{
    shutdown();
    std::cout << "Server shut down." << std::endl;
}

void Server::start()
{
    setupServerSocket();
    mainLoop();
}

void Server::setupServerSocket()
{
    _serverFileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverFileDescriptor < 0)
        throw std::runtime_error("Socket creation failed");

    int opt = 1;
    setsockopt(_serverFileDescriptor, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(_serverFileDescriptor, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_serverFileDescriptor, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Bind failed");
    if (listen(_serverFileDescriptor, SOMAXCONN) < 0)
        throw std::runtime_error("Listen failed");

    struct pollfd pfd;
    pfd.fd = _serverFileDescriptor;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollFds.push_back(pfd);

    std::cout << "Server listening on port " << _port << "..." << std::endl;
}

void Server::mainLoop()
{
    while (_running)
    {
        for (size_t i = 0; i < _pollFds.size(); ++i)
            _pollFds[i].revents = 0;

        int pollCount = poll(&_pollFds[0], _pollFds.size(), 1000);
        if (pollCount < 0)
        {
            if (errno == EINTR)
            {
                if (!_running)
                    break;
                continue;
            }
            throw std::runtime_error("Poll error");
        }
        for (size_t i = 0; i < _pollFds.size(); ++i)
        {
            if (_pollFds[i].revents & POLLIN)
            {
                if (_pollFds[i].fd == _serverFileDescriptor)
                {
                    handleNewConnection();
                } else {
                    handleClientData(_pollFds[i].fd);
                }
            }
        }
    }
    shutdown();
}

void Server::handleNewConnection()
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    memset(&clientAddr, 0, sizeof(clientAddr));

    int clientFd = accept(_serverFileDescriptor, (struct sockaddr*)&clientAddr, &addrLen);
    if (clientFd < 0)
    {
        std::cerr << "Accept failed" << std::endl;
        return;
    }
    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    Client* newClient = new Client(clientFd, clientAddr);
    _clients[clientFd] = newClient;

    struct pollfd pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _pollFds.push_back(pfd);

    std::cout << "New connection accepted: fd " << clientFd << std::endl;
}

void Server::handleClientData(int clientFd)
{
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0)
    {
        if (bytesRead == 0)
            std::cout << "Client " << clientFd << " disconnected" << std::endl;
        else
            std::cerr << "Recv error for " << clientFd << std::endl;
        removeClient(clientFd);
        return;
    }
    Client* client = _clients[clientFd];
    if (client)
        client->appendToBuffer(std::string(buffer));
    std::string& buf = client->getBuffer();
    size_t pos = 0;
    static std::map<int, int> realErrorCounts;
    static std::map<int, std::string> lastNickAttempt;
    
    while ((pos = buf.find("\r\n")) != std::string::npos)
    {
        std::string command = buf.substr(0, pos);
        if (!client->isRegistered() && !command.empty())
        {
            std::vector<std::string> parts;
            size_t start = 0, end = 0;
            while ((end = command.find(' ', start)) != std::string::npos)
            {
                std::string token = command.substr(start, end - start);
                if (!token.empty())
                    parts.push_back(token);
                start = end + 1;
            }
            std::string token = command.substr(start);
            if (!token.empty())
                parts.push_back(token);
            
            if (!parts.empty())
            {
                std::string cmd = parts[0];
                if (cmd != "PASS" && cmd != "NICK" && cmd != "USER" && cmd != "CAP")
                    realErrorCounts[clientFd]++;
                else if (cmd == "NICK" && parts.size() > 1)
                {
                    if (lastNickAttempt[clientFd] == parts[1])
                    {}
                    else
                        lastNickAttempt[clientFd] = parts[1];
                }
            }
        }
        
        CommandParser::process(clientFd, command, _clients, _channelManager, _password);
        client->eraseFromBuffer(0, pos + 2);

        if (client->shouldDisconnect())
        {
            std::cout << "[DEBUG] Client marked for disconnection due to nickname conflict" << std::endl;
            removeClient(clientFd);
            realErrorCounts.erase(clientFd);
            lastNickAttempt.erase(clientFd);
            return;
        }
    }
    if (!client->isRegistered() && realErrorCounts[clientFd] > 10)
    {
        client->reply(":localhost ERROR :Too many invalid commands");
        removeClient(clientFd);
        realErrorCounts.erase(clientFd);
        lastNickAttempt.erase(clientFd);
    }
    if (client->isRegistered())
    {
        realErrorCounts.erase(clientFd);
        lastNickAttempt.erase(clientFd);
    }
}

void Server::removeClient(int clientFd)
{
    std::map<int, Client*>::iterator it = _clients.find(clientFd);
    if (it == _clients.end())
        return;
    Client* client = it->second;
    _channelManager.removeClientFromAll(client);
    close(clientFd);
    for (std::vector<struct pollfd>::iterator pIt = _pollFds.begin(); pIt != _pollFds.end(); ++pIt)
    {
        if (pIt->fd == clientFd)
        {
            _pollFds.erase(pIt);
            break;
        }
    }
    delete client;
    _clients.erase(it);

    std::cout << "Client disconnected. fd: " << clientFd << std::endl;
}

void Server::shutdown()
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); )
    {
        int fd = it->first;
        ++it;
        removeClient(fd);
    }
    if (_serverFileDescriptor != -1)
        close(_serverFileDescriptor);
    std::cout << "Server shutting down cleanly." << std::endl;
}