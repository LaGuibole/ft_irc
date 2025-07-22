#include "Channel.hpp"
#include <iostream>
#include <sstream>

Channel::Channel(const std::string& name)
    : _name(name), _topic(""), _inviteOnly(false), _topicRestricted(false),
      _password(""), _hasPassword(false), _userLimit(0), _hasUserLimit(false)
{
    std::cout << "New channel created: " << this->_name << std::endl;
}

Channel::~Channel()
{
    std::cout << "Canal " << this->_name << " destroyed :'( ." << std::endl;
}

std::vector<Client*> Channel::getMembers() const
{
    std::vector<Client*> members;
    for (std::map<int, Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it)
        members.push_back(it->second);
    return members;
}

Client* Channel::getMemberByNickname(const std::string& nickname) const
{
    for (std::map<int, Client*>::const_iterator it = _members.begin(); it != _members.end(); ++it)
    {
        if (it->second->getNickname() == nickname)
            return it->second;
    }
    return NULL;
}

bool Channel::isMember(Client* client) const
{
    if (!client)
        return false;
    return _members.find(client->getFileDescriptor()) != _members.end();
}

void Channel::addMember(Client* client)
{
    if (!client || isMember(client))
        return;
    _members[client->getFileDescriptor()] = client;
    if (_members.size() == 1)
        _operators[client->getFileDescriptor()] = client;
    std::cout << "Client " << client->getNickname() << " joined " << _name << std::endl;
}

void Channel::removeMember(Client* client, ChannelManager* channel)
{
    if (!client || !isMember(client))
        return;
    _members.erase(client->getFileDescriptor());
    _operators.erase(client->getFileDescriptor());
    std::cout << "Client " << client->getNickname() << " left " << _name << std::endl;
    if (this->getMembers().size() == 0 && channel)
        channel->removeChannel(this);
}

void Channel::broadcast(const std::string& message, Client* exclude)
{
    for (std::map<int, Client*>::iterator it = _members.begin(); it != _members.end(); ++it)
    {
        Client* current = it->second;
        if (current != exclude)
        {
            current->reply(message);
            std::cout << "Broadcast sent in " << _name << std::endl;
        }
    }
}

void Channel::addOperator(Client* client)
{
    if (!client)
        return ;
    _operators[client->getFileDescriptor()] = client;
}

void Channel::removeOperator(Client* client)
{
    if (!client)
        return ;
    _operators.erase(client->getFileDescriptor());
}

bool Channel::isOperator(Client* client) const
{
    if (!client)
        return false;
    return _operators.find(client->getFileDescriptor()) != _operators.end();
}

void Channel::setUserLimit(int limit)
{
    if (limit <= 0)
        return ;
    _hasUserLimit = true;
    _userLimit = limit;
}

void Channel::unsetUserLimit()
{
    _hasUserLimit = false;
    _userLimit = 0;
}

void Channel::changeInviteMode()
{
    if (this->_inviteOnly)
        this->_inviteOnly = false;
    else
        this->_inviteOnly = true;
}

void Channel::changeTopicMode()
{
    if (this->_topicRestricted)
        this->_topicRestricted = false;
    else
        this->_topicRestricted = true;
}

std::string Channel::getModeString() const
{
    std::string flags = "+";
    std::string params;

    if (_inviteOnly)
        flags += "i";
    if (_topicRestricted)
        flags += "t";
    if (_hasPassword)
    {
        flags += "k";
        params += " " + _password;
    }
    if (_hasUserLimit)
    {
        flags += "l";
        std::ostringstream oss;
        oss << " " << _userLimit;
        params += oss.str();
    }

    return flags + params;
}
