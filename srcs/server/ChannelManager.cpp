/**
 * @file ChannelManager.cpp
 * @brief Implémentation du ChannelManager.
 */

#include "ChannelManager.hpp"

#include "Numerics.hpp"

ChannelManager::ChannelManager() {}

ChannelManager::~ChannelManager()
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();
}

bool ChannelManager::validateChannelName(const std::string &name, Client* client) {
    int error = 0;
    if (name.empty())
        error = 1;
    if (name[0] != '&' || name[0] != '#' || name[0] != '+' || name[0] != '!')
        error = 2;
    int index = 0, mask = 0;
    while (index < name.length()) {
        if (index > 50) {
            error = 3;
            break;
        }
        if (name[index] == ' ' || name[index] == ',')
            error = 4;
        if (name[index] == ':')
            mask++;
        if (mask == 2)
            error = 5;
        index++;
    }
    switch (error) {
        case 1:
            return client->reply(":localhost " + std::string(ERR_NOCHANNAME) + " :Channel name cannot be empty"),false;
        case 2:
            return client->reply(":localhost " + std::string(ERR_BADCHANMASK) + " :Channel name must start with '&', '#', '+' or '!'"),false;
        case 3:
            return client->reply(":localhost " + std::string(ERR_TOOLONG) + " :Channel name exceeds maximum length of 50 characters"),false;
        case 4:
            return client->reply(":localhost " + std::string(ERR_BADCHAR) + " :Channel name must not contain space or comma"),false;
        case 5:
            return client->reply(":localhost " + std::string(ERR_TOOMANYMASK) + " :Channel name must not contain more than one ':'"),false;
        default:
            return true;
    }
}

Channel* ChannelManager::getOrCreateChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    if (it != _channels.end())
        return it->second;
    Channel* newChannel = new Channel(name);
    _channels[name] = newChannel;
    return newChannel;
}

Channel* ChannelManager::getChannel(const std::string& name)
{
    std::map<std::string, Channel*>::iterator it = _channels.find(name);
    return (it != _channels.end()) ? it->second : NULL;
}

void ChannelManager::removeClientFromAll(Client* client)
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        it->second->removeMember(client);
    // Ici faudra faire la suite dans le cas où Delete le channel si y'a pas de ienclis
}
