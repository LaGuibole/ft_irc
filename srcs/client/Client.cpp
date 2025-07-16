#include "Client.hpp"
#include "Numerics.hpp"
#include <iostream>
#include <unistd.h> // Pour close
#include <arpa/inet.h> // Pour inet_ntop

Client::Client(int fd, const struct sockaddr_in& addr)
    : _fileDescriptor(fd), _address(addr), _hostname(""),
      _nickname(""), _username(""), _realname(""),
      _buffer(""), _isPassValidated(false), _isRegistered(false)
{
    char host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), host, INET_ADDRSTRLEN);
    _hostname = host;
    std::cout << "New client connected. fd: " << fd << ", hostname: " << _hostname << std::endl;
}

Client::~Client()
{
    if (_fileDescriptor != -1)
        close(_fileDescriptor);
}

void Client::reply(const std::string& message)
{
    std::string fullMessage = message + "\r\n";
    if (send(_fileDescriptor, fullMessage.c_str(), fullMessage.length(), 0) < 0)
        std::cerr << "Error sending to fd " << _fileDescriptor << std::endl;
}

void Client::welcome()
{
    reply(":localhost " + std::string(RPL_WELCOME) + " " + _nickname + " :Welcome to the IRC server!");
    std::cout << "Client " + _nickname + " is now registered." << std::endl;
}
