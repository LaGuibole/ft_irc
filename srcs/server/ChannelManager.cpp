#include "ChannelManager.hpp"
#include "Numerics.hpp"
#include "Utils.hpp"
#include <iostream>

ChannelManager::ChannelManager() {}

ChannelManager::~ChannelManager()
{
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
        delete it->second;
    _channels.clear();
}

bool ChannelManager::validateChannelName(const std::string &name, Client* client) {
    if (name.empty()) {
		Utils::sendError(client, ERR_NOSUCHCHANNEL, "*", ":Empty channel name");
		return false;
	}
	if (name[0] != '&' && name[0] != '#' && name[0] != '+' && name[0] != '!') {
		Utils::sendError(client, ERR_BADCHANMASK, name, ":Bad Channel Mask");
		return false;
	}
	if (name.length() > 50) {
		Utils::sendError(client, ERR_BADCHANMASK, name, ":Channel name too long");
		return false;
	}
	int mask_count = 0;
	for (size_t i = 0; i < name.length(); ++i) {
		if (name[i] == ' ' || name[i] == ',') {
			Utils::sendError(client, ERR_BADCHANMASK, name, ":Channel name must not contain space or comma");
			return false;
		}
		if (name[i] == ':')
			++mask_count;
		if (mask_count == 2) {
			Utils::sendError(client, ERR_BADCHANMASK, name, ":Bad Channel Mask");
			return false;
		}
	}
	return true;
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
	std::vector<std::string> toDelete;

	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		Channel* channel = it->second;
		channel->removeMember(client, NULL);
		if (channel->getMembers().empty())
			toDelete.push_back(it->first);
	}

	for (size_t i = 0; i < toDelete.size(); ++i)
	{
		Channel* ch = _channels[toDelete[i]];
		delete ch;
		_channels.erase(toDelete[i]);
	}
	// Ici faudra faire la suite dans le cas où Delete le channel si y'a pas de ienclis
}

void ChannelManager::removeChannel(Channel* channel)
{
    _channels.erase(channel->getName());
    delete channel;
}
