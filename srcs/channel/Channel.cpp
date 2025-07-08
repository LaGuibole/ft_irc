#include "Channel.hpp"
#include <iostream>

// Est appelé par Server lorsqu'un client rejoint un canal qui n'existe pas encore.
Channel::Channel(const std::string& name) : _name(name), _topic("")
{
    std::cout << "New channel created: " << this->_name << std::endl;
}

Channel::~Channel()
{
    std::cout << "Canal " << this->_name << " destroyed :'( ." << std::endl;
}

const std::string& Channel::getName() const
{
    return this->_name;
}

const std::string& Channel::getTopic() const
{
    return this->_topic;
}

void Channel::setTopic(const std::string& topic)
{
    this->_topic = topic;
}

/**
 * @brief Ajoute un client au canal.
 * * Si le canal est vide, le premier client devient automatiquement opérateur.
 * @param client Pointeur vers l'objet Client à ajouter.
 */
void Channel::addClient(Client* client)
{
    if (client == NULL)
        return;
    _clients[client->getFd()] = client;
    if (_clients.size() == 1)
        _operators[client->getFd()] = client;
    std::cout << "Client " << client->getNickname() << " join the chat " << _name << std::endl;
}

/**
 * @brief Retire un client du canal.
 * @param client Pointeur vers l'objet Client à retirer.
 */
void Channel::removeClient(Client* client)
{
    if (client == NULL)
        return;
    _clients.erase(client->getFd());
    _operators.erase(client->getFd());
    std::cout << "Client " << client->getNickname() << " left the chat " << _name << std::endl;
}

/**
 * @brief Diffuse un message à tous les membres du canal.
 * @param message Le message à envoyer.
 * @param exclude_client Le client qui a envoyé le message original (pour ne pas le lui renvoyer).
 */
void Channel::broadcast(const std::string& message, Client* exclude_client)
{
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        Client* current_client = it->second;
        if (current_client != exclude_client)
        {
            current_client->reply(message);
            std::cout << "DEBUG >> BroadCast sended in :" << _name << std::endl;
        }
    }
}

/**
 * @brief Vérifie si un client est un opérateur du canal.
 * @param client Le client à vérifier.
 * @return true si le client est opérateur, false sinon.
 */
bool Channel::isOperator(Client* client)
{
    if (client == NULL)
        return false;
    return _operators.find(client->getFd()) != _operators.end();
}
