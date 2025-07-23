/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guphilip <guphilip@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 23:08:46 by guillaumeph       #+#    #+#             */
/*   Updated: 2025/07/23 14:31:18 by guphilip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <iostream>

Bot::Bot()
{
    loadWordlist();
}

Bot::~Bot() {}

void Bot::loadWordlist()
{
    _wordlist.push_back("vim");
    _wordlist.push_back("windows");
    _wordlist.push_back("jonas");
}

bool Bot::inspectMessages(Client* client, const std::string& message, Channel* channel, ChannelManager* channelManager)
{
    for (size_t i = 0; i < _wordlist.size(); ++i)
    {
        if (message.find(_wordlist[i]) != std::string::npos)
        {
            _warnings[client]++;
            int count = _warnings[client];
            std::string nick = client->getNickname();

            if (!channel)
                return false;

            if (count > 2)
            {
                client->reply(":BOT NOTICE " + nick + " :[BOT] You have been removed from " + channel->getName() + " for repeated inappropriate language.");

                bool wasOperator = channel->isOperator(client);
                bool shouldPromote = wasOperator && channel->getOperators().size() == 1 && channel->getMembers().size() > 1;

                Client* newOp = NULL;
                if (shouldPromote)
                {
                    std::vector<Client*> members = channel->getMembers();
                    for (size_t j = 0; j < members.size(); ++j)
                    {
                        if (members[j] != client)
                        {
                            newOp = members[j];
                            channel->addOperator(newOp);

                            std::string modeMsg = ":localhost MODE " + channel->getName() + " +o " + newOp->getNickname();
                            channel->broadcast(modeMsg);

                            std::stringstream opMsg;
                            opMsg << ":BOT PRIVMSG " << channel->getName()
                                  << " :[BOT] " << newOp->getNickname() << " has been promoted to channel operator.";
                            channel->broadcast(opMsg.str());
                            break;
                        }
                    }
                }

                std::string partMsg = ":" + client->getPrefix() + " PART " + channel->getName() + " :[BOT] Repeated inappropriate language.";
                channel->broadcast(partMsg);

                channel->removeMember(client, channelManager);
            }
            else
            {
                std::stringstream warnMsg;
                warnMsg << ":BOT PRIVMSG " << channel->getName()
                        << " :[BOT] Warning " << count << "/2 for " << nick << " : inappropriate language detected.";
                channel->broadcast(warnMsg.str());
            }

            std::cout << "[BOT] Warning " << count << "/2 for " << nick << std::endl;
            return false;
        }
    }
    return true;
}
