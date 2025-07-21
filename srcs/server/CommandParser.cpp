#include "CommandParser.hpp"
#include <iostream>
#include <algorithm>
#include "Utils.hpp"

std::vector<std::string> CommandParser::split(const std::string& str, char delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos)
    {
        std::string token = str.substr(start, end - start);
        if (!token.empty())
            tokens.push_back(token);
        start = end + 1;
    }
    std::string token = str.substr(start);
    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

std::string CommandParser::getMessage(const std::string& command)
{
    size_t colonPos = command.find(':');
    if (colonPos == std::string::npos)
        return "";
    return command.substr(colonPos + 1);
}

void CommandParser::process(int clientFd, const std::string& command,
                            std::map<int, Client*>& clients,
                            ChannelManager& channelManager,
                            const std::string& password,
                            Server &server)
{
    if (command.empty())
        return;
    Client* client = clients[clientFd];
    if (!client)
        return;
    //std::cout << "[DEBUG] Command received from fd:" << clientFd << " -> '" << command << "'" << std::endl;
    std::vector<std::string> parts = split(command, ' ');
    if (parts.empty())
        return;
    std::string cmd = parts[0];
    std::vector<std::string> params(parts.begin() + 1, parts.end());
    //std::cout << "[DEBUG] Parsed command: '" << cmd << "' with " << params.size() << " parameters" << std::endl;
    if (cmd == "PASS")
        handlePass(client, params, password);
    else if (cmd == "NICK")
        handleNick(client, params, clients);
    else if (cmd == "USER")
        handleUser(client, command);
    else if (cmd == "CAP")
    {
        if (params.size() > 0 && params[0] == "LS")
            client->reply(":localhost CAP * LS :");
        else if (params.size() > 0 && params[0] == "END")
            return;
        return;
    }
    else if (cmd == "PING")
        handlePing(client, params);
    else if (cmd == "WHO")
        handleWho(client, params, clients, channelManager);
    else if (client->isRegistered()) {
        std::cout << client->getNickname() << " called " << cmd << std::endl; // debug
        if (cmd == "JOIN")
            handleJoin(client, params, channelManager);
        else if (cmd == "PART")
            handlePart(client, params, channelManager);
        else if (cmd == "PRIVMSG")
            handlePrivmsg(client, command, clients, channelManager);
        else if (cmd == "INVITE")
            handleInvite(client, params, clients, channelManager);
        else if (cmd == "QUIT")
            handleQuit(client, command, channelManager, server);
        else if (cmd == "KICK")
            handleKick(client, params, channelManager);
        else if (cmd == "MODE")
            handleMode(client, params, channelManager);
        else if (cmd == "TOPIC")
            handleTopic(client, params, channelManager);
        else
            client->reply(":localhost " + std::string(ERR_UNKNOWNCOMMAND) + " " + cmd + " :Unknown command");
    } else {
        if (cmd != "PASS" && cmd != "NICK" && cmd != "USER" && cmd != "CAP")
            client->reply(":localhost " + std::string(ERR_NOTREGISTERED) + " :You have not registered");
    }
    if (!client->isRegistered() &&
        client->isPassValidated() &&
        !client->getNickname().empty() &&
        !client->getUsername().empty() &&
        !client->hasNickConflict())
    {
        bool nickStillAvailable = true;
        for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second != client && it->second->getNickname() == client->getNickname())
            {
                nickStillAvailable = false;
                break;
            }
        }
        if (nickStillAvailable)
        {
            client->welcome();
            client->setRegistered(true);
        }
        else
        {
            client->setNickConflict(true);
        }
    }
    if (!client->isRegistered() && client->hasNickConflict())
    {
        client->reply(":localhost ERROR :Nickname already in use, closing connection");
        client->setToDisconnect(true);
    }
}

void CommandParser::handleTopic(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager) {
	if (params.empty())
	{
		client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " TOPIC :Not enough parameters");
		return;
	}
	for (int i = 0; i < int(params.size()); i++)
		std::cout << params[i] << " : " << i << std::endl;
	const std::string channel_name = params[0];
	if (!channelManager.validateChannelName(channel_name, client))
		return;
	Channel* channel = channelManager.getChannel(channel_name);
	if (!channel) {
		client->reply(":localhost " + std::string(ERR_NOTONCHANNEL) + " " +channel_name + " :You're not on that channel");
		return;
	}
	if (params.size() == 1) {
		if (channel->getTopic().empty())
			client->reply(":localhost " + std::string(RPL_NOTOPIC) + " :No topic is set");
		else
			client->reply(":localhost " + std::string(RPL_TOPIC) + " :" + channel->getTopic() + " " + channel_name);
		return;
	}
	else if (params.size() == 2 && ((channel->isTopicRestricted() && channel->isOperator(client)) || !channel->isTopicRestricted())) {
		if (params[1] == ":")
			channel->setTopic(NULL);
		else
			channel->setTopic(params[1]);
	}
}

void CommandParser::handlePass(Client* client, const std::vector<std::string>& params, const std::string& password)
{
    if (params.empty())
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " PASS :Not enough parameters");
        return;
    }
    if (client->isPassValidated())
    {
        client->reply(":localhost " + std::string(ERR_ALREADYREGISTRED) + " :Already registered");
        return;
    }
    if (params[0] != password)
    {
        client->reply(":localhost " + std::string(ERR_PASSWDMISMATCH) + " :Password incorrect");
        return;
    }
    client->setPassValidated(true);
}

void CommandParser::handleNick(Client* client, const std::vector<std::string>& params, const std::map<int, Client*>& clients)
{
    if (!client->isPassValidated())
    {
        client->reply(":localhost " + std::string(ERR_NOTREGISTERED) + " :PASS required first");
        return;
    }
    if (params.empty())
    {
        client->reply(":localhost " + std::string(ERR_NONICKNAMEGIVEN) + " :No nickname given");
        return;
    }

    std::string newNick = params[0];
    if (client->getNickname() == newNick)
        return;

    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second != client && it->second->getNickname() == newNick)
        {
            client->reply(":localhost " + std::string(ERR_NICKNAMEINUSE) + " " + newNick + " :Nickname is already in use");
            if (!client->isRegistered())
            {
                client->setNickConflict(true);
                client->reply(":localhost ERROR :Nickname already in use, closing connection");
                client->setToDisconnect(true);
            }
            return;
        }
    }
    client->setNickConflict(false);
    if (client->isRegistered() && !client->getNickname().empty())
    {
        std::string oldNick = client->getNickname();
        client->setNickname(newNick);
        client->reply(":" + oldNick + "!" + client->getUsername() + "@" + client->getHostname() + " NICK :" + newNick);
    }
    else
        client->setNickname(newNick);
}

void CommandParser::handleUser(Client* client, const std::string& command)
{
    if (!client->isPassValidated())
    {
        client->reply(":localhost " + std::string(ERR_NOTREGISTERED) + " :PASS required first");
        return;
    }
    std::vector<std::string> parts = split(command, ' ');
    if (parts.size() < 5)
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " USER :Not enough parameters");
        return;
    }
    client->setUsername(parts[1]);
    size_t colonPos = command.find(':');
    if (colonPos != std::string::npos)
        client->setRealname(command.substr(colonPos + 1));
}

void CommandParser::handleJoin(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager)
{
    if (params.empty())
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " JOIN :Not enough parameters");
        return;
    }
    std::string channelName = params[0];
    if (channelName[0] != '#')
    {
        client->reply(":localhost " + std::string(ERR_NOSUCHCHANNEL) + " " + channelName + " :Invalid channel name");
        return;
    }
    Channel* channel = NULL;
    channel = channelManager.getOrCreateChannel(channelName);
    // verif de la limite d'user pour mode -l
    if (channel->hasUserLimit() && channel->getMembers().size() >= channel->getUserLimit())
    {
        client->reply(":localhost " + std::string(ERR_CHANNELISFULL) + " " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)");
        return ;
    }
    channel->addMember(client);

    // si premier utilisateur : promu automatiquement en operateur
    if (channel->getMembers().size() == 1)
        channel->addOperator(client);
    std::string joinMsg = ":" + client->getPrefix() + " JOIN " + channelName;
    channel->broadcast(joinMsg);

    if (!channel->getTopic().empty())
        client->reply(":localhost " + std::string(RPL_TOPIC) + " " + client->getNickname() + " " + channelName + " :" + channel->getTopic());

    std::string names = ":localhost " + std::string(RPL_NAMREPLY) + " " + client->getNickname() + " = " + channelName + " :";
    std::vector<Client*> members = channel->getMembers();

    for (size_t i = 0; i < members.size(); ++i)
    {
        if (channel->isOperator(members[i]))
            names += "@" + members[i]->getNickname() + " ";
        else
            names += members[i]->getNickname() + " ";
    }
    client->reply(names);
    client->reply(":localhost " + std::string(RPL_ENDOFNAMES) + " " + client->getNickname() + " " + channelName + " :End of NAMES list");
}

void CommandParser::handlePart(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager)
{
    if (params.empty())
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " PART :Not enough parameters");
        return;
    }
    std::string channelName = params[0];
    Channel* channel = channelManager.getChannel(channelName);
    if (!channel)
    {
        client->reply(":localhost " + std::string(ERR_NOSUCHCHANNEL) + " " + channelName + " :No such channel");
        return;
    }
    if (!channel->isMember(client))
    {
        client->reply(":localhost " + std::string(ERR_NOTONCHANNEL) + " " + channelName + " :You're not on that channel");
        return;
    }
    std::string partMsg = ":" + client->getPrefix() + " PART " + channelName + " :" + (params.size() > 1 ? params[1] : "Leaving");
    channel->broadcast(partMsg);
    channel->removeMember(client, &channelManager);
}

void CommandParser::handlePrivmsg(Client* client, const std::string& command, const std::map<int, Client*>& clients, ChannelManager& channelManager)
{
    std::vector<std::string> parts = split(command, ' ');
    if (parts.size() < 3)
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " PRIVMSG :Not enough parameters");
        return;
    }
    std::string target = parts[1];
    std::string message = getMessage(command);
    if (message.empty())
    {
        client->reply(":localhost " + std::string(ERR_NOTEXTTOSEND) + " :No text to send");
        return;
    }
    std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + message;

    if (target[0] == '#')
    {
        Channel* channel = channelManager.getChannel(target);
        if (!channel)
        {
            client->reply(":localhost " + std::string(ERR_NOSUCHCHANNEL) + " " + target + " :No such channel");
            return;
        }
        if (!channel->isMember(client))
        {
            client->reply(":localhost " + std::string(ERR_CANNOTSENDTOCHAN) + " " + target + " :Cannot send to channel");
            return;
        }
        channel->broadcast(msg, client);
    } else {
        bool found = false;
        for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second->getNickname() == target)
            {
                it->second->reply(msg);
                found = true;
                break;
            }
        }
        if (!found)
            client->reply(":localhost " + std::string(ERR_NOSUCHNICK) + " " + target + " :No such nick");
    }
}

void CommandParser::handleQuit(Client* client, const std::string& command, ChannelManager& channelManager, Server& server)
{
    std::string message = getMessage(command);
    std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + (message.empty() ? "Quit" : message);
    // A add un Broadcast ici pour plus tard
    client->reply(":localhost quit.");
    server.removeClient(client->getFileDescriptor());
}

void CommandParser::handleInvite(Client* client, std::vector<std::string>& args, const std::map<int, Client*>& clients, ChannelManager& channelManager) {
	if (args.size() != 2) {
		Utils::sendError(client, ERR_NEEDMOREPARAMS, "NULL", ":Not enough parameters");
		return;
	}
    const std::string& channel_name = args[1];
    if (!channelManager.validateChannelName(channel_name, client)) {
        return;
    }
	Client* target = NULL;
	for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
		if (it->second->getNickname() == args[0]) {
			target = it->second;
			break;
		}
	}

	if (!target) {
		Utils::sendError(client, ERR_NOSUCHNICK, args[0], ":No such nick");
		return;
	}
    Channel* channel = channelManager.getChannel(channel_name);
	if (channel && !channel->isMember(client)) {
		Utils::sendError(client, ERR_NOTONCHANNEL, channel_name, ":You're not on that channel");
		return;
	}
	else if (channel && channel->isMember(target)) {
		Utils::sendError(client, ERR_USERONCHANNEL, channel_name, ":is already on channel");
		return;
	}
	if (channel && channel->isInviteOnly() && !channel->isOperator(client)) {
		Utils::sendError(client, ERR_CHANOPRIVSNEEDED, channel->getName(), ":You're not channel operator");
		return;
	}
	// voir pour ajouter le membre grace a l'invite en mode invite only avec guillaume
    client->reply(":localhost " + std::string(RPL_INVITING) + " " + client->getNickname() + " " + target->getNickname() + " " + args[1]);
	target->reply(":" + client->getNickname() + " INVITE " + target->getNickname() + " :" + channel_name);
}

void CommandParser::handleKick(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager)
{
    if (params.size() < 2)
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " KICK :Not enough parameters");
        return ;
    }

    const std::string& channelName = params[0];
    const std::string& targetNick = params[1];
    std::string comment = (params.size() >  2) ? params[2] : client->getNickname();

    Channel* channel = channelManager.getChannel(channelName);
    if (!channel)
    {
        client->reply(":localhost " + std::string(ERR_NOSUCHCHANNEL) + " " + channelName + " :No such channel.");
        return ;
    }

    if (!channel->isMember(client))
    {
        client->reply(":localhost " + std::string(ERR_NOTONCHANNEL) + " " + channelName + " :You're not on that channel");
        return ;
    }

    if (!channel->isOperator(client))
    {
        client->reply(":localhost " + std::string(ERR_CHANOPRIVSNEEDED) + " " + channelName + " :You're not channel operator");
        return ;
    }

    Client* target = channel->getMemberByNickname(targetNick);

    if (!target)
    {
        client->reply(":localhost " + std::string(ERR_USERNOTINCHANNEL) + " " + channelName + " :They aren't on that channel");
        return ;
    }
    std::string kickMessage = ":" + client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + comment;
    target->reply(kickMessage);
    channel->broadcast(kickMessage, target);
    channel->removeMember(target, &channelManager);
}

void CommandParser::applyChannelMode(Client* client, Channel* channel, const std::string& modeFlags, std::vector<std::string>& modeParams)
{
    bool adding = true;
    size_t paramIndex = 0;

    for (size_t i = 0; i < modeFlags.size(); ++i)
    {
        char mode = modeFlags[i];

        if (mode == '+')
        {
            adding = true;
            continue;
        }
        else if (mode == '-')
        {
            adding = false;
            continue;
        }

        switch (mode)
        {
            case 'o':
            {
                if (paramIndex >= modeParams.size())
                {
                    client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " MODE: Missing parameters for mode o");
                    return;
                }
                std::string targetNick = modeParams[paramIndex++];
                Client* target = channel->getMemberByNickname(targetNick);
                if (!target)
                {
                    client->reply(":localhost " + std::string(ERR_USERNOTINCHANNEL) + " " + targetNick + " " + channel->getName() + " :They aren't on that channel");
                    return ;
                }
                if (adding)
                    channel->addOperator(target);
                else
                    channel->removeOperator(target);

                std::string modeMsg = ":" + client->getPrefix() + " MODE " + channel->getName() + " " + (adding ? "+o " : "-o ") + targetNick;
                channel->broadcast(modeMsg);
                break;
            }
            case 'l':
            {
                if (adding)
                {
                    if (paramIndex >= modeParams.size())
                    {
                        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " MODE: Missing parameters for mode l");
                        return ;
                    }
                    int limit = atoi(modeParams[paramIndex++].c_str());
                    channel->setUserLimit(limit);
                }
                else
                    channel->unsetUserLimit();
                break;
            }
            default:
                client->reply(":localhost " + std::string(ERR_UNKNOWNMODE) + " " + mode + " :is not a supported mode (yet)");
                break;
        }
    }
}

void CommandParser::handleMode(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager)
{
    if (params.empty())
    {
        client->reply(":localhost " + std::string(ERR_NEEDMOREPARAMS) + " MODE :Not enough parameters");
        return ;
    }

    const std::string& target = params[0];

    if (target[0] != '#')
    {
        client->reply(":localhost " + std::string(ERR_USERSDONTMATCH) + " :User mode change is not supported");
        return ;
    }

    Channel* channel = channelManager.getChannel(target);

    if (!channel)
    {
        client->reply(":localhost " + std::string(ERR_NOSUCHCHANNEL) + " :No such channel");
        return ;
    }

    if (!channel->isMember(client))
    {
        client->reply(":localhost " + std::string(ERR_NOTONCHANNEL) + " " + target + " :You're not on that channel");
        return ;
    }

    if (params.size() == 1)
    {
        std::string modeLine = channel->getModeString();
        client->reply(":localhost " + std::string(RPL_CHANNELMODEIS) + " " + client->getNickname() + " " + target + " " + modeLine);
        return ;
    }

    if (!channel->isOperator(client))
    {
        client->reply(":localhost " + std::string(ERR_CHANOPRIVSNEEDED) + " " + target + " :You're not channel operator");
        return ;
    }

    const std::string& modeFlags = params[1];
    std::vector<std::string> modeParams(params.begin() + 2, params.end());

    for (size_t i = 0; i < modeParams.size(); ++i)
        std::cout << " [" << modeParams[i] << "]";
    std::cout << std::endl;

    applyChannelMode(client, channel, modeFlags, modeParams);
}

void CommandParser::handlePing(Client* client, const std::vector<std::string>& params)
{
    if (params.empty())
        client->reply(":localhost PONG localhost");
    else
        client->reply(":localhost PONG localhost :" + params[0]);
}

void CommandParser::handleWho(Client* client, const std::vector<std::string>& params, const std::map<int, Client*>& clients, ChannelManager& channelManager)
{
    if (params.empty())
    {
        client->reply(":localhost " + std::string(RPL_ENDOFWHO) + " " + client->getNickname() + " * :End of WHO list");
        return;
    }

    std::string target = params[0];

    if (target[0] == '#')
    {
        // WHO pour un channel
        Channel* channel = channelManager.getChannel(target);
        if (!channel)
        {
            client->reply(":localhost " + std::string(ERR_NOSUCHCHANNEL) + " " + target + " :No such channel");
            return;
        }

        if (!channel->isMember(client))
        {
            client->reply(":localhost " + std::string(ERR_NOTONCHANNEL) + " " + target + " :You're not on that channel");
            return;
        }

        std::vector<Client*> members = channel->getMembers();
        for (size_t i = 0; i < members.size(); ++i)
        {
            Client* member = members[i];
            std::string flags = "H";
            if (channel->isOperator(member))
                flags += "@";
            client->reply(":localhost 352 " + client->getNickname() + " " + target + " " +
                         member->getUsername() + " " + member->getHostname() + " localhost " +
                         member->getNickname() + " " + flags + " :0 " + member->getRealname());
        }
        client->reply(":localhost " + std::string(RPL_ENDOFWHO) + " " + client->getNickname() + " " + target + " :End of WHO list");
    }
    else
    {
        for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
        {
            if (it->second->getNickname() == target)
            {
                Client* targetClient = it->second;
                client->reply(":localhost 352 " + client->getNickname() + " * " +
                             targetClient->getUsername() + " " + targetClient->getHostname() + " localhost " +
                             targetClient->getNickname() + " H :0 " + targetClient->getRealname());
                break;
            }
        }
        client->reply(":localhost " + std::string(RPL_ENDOFWHO) + " " + client->getNickname() + " " + target + " :End of WHO list");
    }
}
