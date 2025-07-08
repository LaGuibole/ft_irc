#include "Client.hpp"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

// La fonction est appelé par Server lorsqu'une nouvelle connexion est good.
// Initialise les informations de base du client.
Client::Client(int fd, const struct sockaddr_in& address)
    : _fd(fd), _is_registered(false)
{
    this->_hostname = inet_ntoa(address.sin_addr);
    this->_nickname = "*";
    this->_username = "*";

    std::cout << "New client connected. fd: " << this->_fd 
              << ", hostname: " << this->_hostname << std::endl;
}

Client::~Client()
{
    std::cout << "Client disconnected. fd: " << this->_fd << std::endl;
}

int Client::getFd() const
{
    return this->_fd;
}

const std::string& Client::getNickname() const
{
    return this->_nickname;
}

const std::string& Client::getUsername() const
{
    return this->_username;
}

void Client::setNickname(const std::string& nickname)
{
    this->_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
    this->_username = username;
}

/**
 * @brief Construit le préfixe complet du client.
 * Format: "nickname!username@hostname"
 * @return Le préfixe sous forme de std::string.
 */
std::string Client::getPrefix() const
{
    if (_nickname.empty() || _username.empty() || _hostname.empty())
        return "";
    return _nickname + "!" + _username + "@" + _hostname;
}

/**
 * @brief Envoie un message formaté au client.
 * * C'est la fonction centrale pour communiquer avec ce client.
 * Elle ajoute automatiquement le délimiteur de fin de message IRC "\r\n".
 * @param message Le message complet à envoyer.
 */
void Client::reply(const std::string& message)
{
    std::string full_message = message + "\r\n";
    if (send(_fd, full_message.c_str(), full_message.length(), 0) < 0)
        std::cerr << "Error: Cannot send message " << _fd << std::endl;
}

/**
 * @brief Envoie les messages de bienvenue au client une fois enregistré.
 * * Appelée après que le client ait fourni PASS, NICK et USER correctement.
 */
void Client::welcome()
{
    if (_is_registered)
        return;
    this->_is_registered = true;
    reply(":localhost 001 " + _nickname + " :Welcome to our IRC server, " + getPrefix());
    std::cout << "Client " << _nickname << " is now registered." << std::endl;
}
