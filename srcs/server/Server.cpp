#include "Server.hpp"
#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>

Server::Server(int port, const std::string& password) 
    : _port(port), _password(password), _server_fd(-1) {}

Server::~Server()
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
        close(it->first);
        delete it->second;
    }
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    if (_server_fd != -1)
        close(_server_fd);
    std::cout << "Server shut down." << std::endl;
}

void Server::start()
{
    setupServerSocket();
    mainLoop();
}

void Server::init() {} // Bientot

void Server::setupServerSocket()
{
    _server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (_server_fd < 0)
		throw std::runtime_error("Failed to create socket");
    int opt = 1;
    if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Failed to set socket options");
    if (fcntl(_server_fd, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("Failed to set socket to non-blocking");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(_port);

    if (bind(_server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        throw std::runtime_error("Failed to bind socket");
    if (listen(_server_fd, SOMAXCONN) < 0)
        throw std::runtime_error("Failed to listen on socket");
    struct pollfd server_poll_fd;
    server_poll_fd.fd = _server_fd;
    server_poll_fd.events = POLLIN;
    _poll_fds.push_back(server_poll_fd);
    std::cout << "Server listening on port " << _port << "..." << std::endl;
}

void Server::mainLoop()
{
    while (true)
	{
        int poll_count = poll(&_poll_fds[0], _poll_fds.size(), -1);
        if (poll_count < 0)
			throw std::runtime_error("Poll error");
        for (size_t i = 0; i < _poll_fds.size(); ++i)
		{
            if (_poll_fds[i].revents & POLLIN)
			{
                if (_poll_fds[i].fd == _server_fd)
                    handleNewConnection();
                else
                    handleClientData(_poll_fds[i].fd);
            }
        }
    }
}

void Server::handleNewConnection()
{
    std::cout << "New connection attempt..." << std::endl;
}

void Server::handleClientData(int client_fd)
{
    std::cout << "Data received from client " << client_fd << std::endl;
}
