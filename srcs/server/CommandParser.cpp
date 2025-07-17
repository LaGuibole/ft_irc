/**
 * @file CommandParser.cpp
 * @brief Implémentation du parsing des commandes.
 */

#include "CommandParser.hpp"
#include <iostream>
#include <algorithm>

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
                            const std::string& password)
{
    if (command.empty())
        return;
    Client* client = clients[clientFd];
    if (!client)
        return;

    std::vector<std::string> parts = split(command, ' ');
    if (parts.empty())
        return;
    std::string cmd = parts[0];
    std::vector<std::string> params(parts.begin() + 1, parts.end());

    if (cmd == "PASS")
        handlePass(client, params, password);
    else if (cmd == "NICK")
        handleNick(client, params, clients);
    else if (cmd == "USER")
        handleUser(client, command);
    else if (client->isRegistered()) {
        if (cmd == "JOIN")
            handleJoin(client, params, channelManager);
        else if (cmd == "PART")
            handlePart(client, params, channelManager);
        else if (cmd == "PRIVMSG")
            handlePrivmsg(client, command, clients, channelManager);
        else if (cmd == "QUIT")
            handleQuit(client, command, channelManager);
        else if (cmd == "KICK")
            handleKick(client, params, channelManager);
        // Ici faudra ajouter  d'autres else if pour TOPIC, MODE, KICK, INVITE | JONAS & GUIGUI
        else
            client->reply(":localhost " + std::string(ERR_UNKNOWNCOMMAND) + " " + cmd + " :Unknown command");
    } else
        client->reply(":localhost " + std::string(ERR_NOTREGISTERED) + " :You have not registered");

    if (client->isPassValidated() && !client->getNickname().empty() &&
        !client->getUsername().empty() && !client->isRegistered())
    {
        client->welcome();
        client->setRegistered(true);
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
    for (std::map<int, Client*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
    {
        if (it->second->getNickname() == newNick)
        {
            client->reply(":localhost " + std::string(ERR_NICKNAMEINUSE) + " " + newNick + " :Nickname in use");
            return;
        }
    }
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
    Channel* channel = channelManager.getOrCreateChannel(channelName);
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
    channel->removeMember(client);
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

void CommandParser::handleQuit(Client* client, const std::string& command, ChannelManager& channelManager)
{
    std::string message = getMessage(command);
    std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + (message.empty() ? "Quit" : message);
    // A add un Broadcast ici pour plus tard
    client->reply(":localhost quit.");
    channelManager.removeClientFromAll(client);
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
    channel->broadcast(kickMessage, target); // display kick message to all channel members
    channel->removeMember(target); // kicks target from channel
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
        std::string modeLine = channel->getModeString(); // to do
        client->reply(":localhost " + std::string(RPL_CHANNELMODEIS))
    }
}
