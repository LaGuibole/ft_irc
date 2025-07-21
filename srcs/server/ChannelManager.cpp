/**
 * @file ChannelManager.cpp
 * @brief Implémentation du ChannelManager.
 */

#include "ChannelManager.hpp"

ChannelManager::ChannelManager() {}

ChannelManager::~ChannelManager()
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();
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
        it->second->removeMember(client, this);
    // Ici faudra faire la suite dans le cas où Delete le channel si y'a pas de ienclis
}

void ChannelManager::removeChannel(Channel* channel)
{
    _channels.erase(channel->getName());
    channel->~Channel();
}
