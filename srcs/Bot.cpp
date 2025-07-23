/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guphilip <guphilip@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 23:08:46 by guillaumeph       #+#    #+#             */
/*   Updated: 2025/07/23 13:48:40 by guphilip         ###   ########.fr       */
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

// bool Bot::inspectMessages(Client* client, const std::string& message, Channel* channel, ChannelManager* channelManager)
// {
//     for (size_t i = 0; i < _wordlist.size(); ++i)
//     {
//         if (message.find(_wordlist[i]) != std::string::npos)
//         {
//             _warnings[client]++;
//             int count = _warnings[client];
//             std::string nick = client->getNickname();

//             if (!channel)
//                 return false;

//             if (count > 2)
//             {
//                 client->reply(":BOT NOTICE " + nick + " :[BOT] You have been removed from " + channel->getName() + " for repeated inappropriate language.");

//                 bool wasOperator = channel->isOperator(client);

//                 std::string partMsg = ":" + client->getPrefix() + " PART " + channel->getName() + " :[BOT] Repeated inappropriate language.";
//                 channel->broadcast(partMsg);

//                 channel->removeMember(client, channelManager);

//                 if (wasOperator && channel->getOperators().empty() && !channel->getMembers().empty())
//                 {
//                     Client* newOp = channel->getMembers().front();
//                     channel->addOperator(newOp);
//                     channel->sendNamesListTo(newOp);

//                     std::stringstream opMsg;
//                     opMsg << ":BOT PRIVMSG " << channel->getName()
//                           << " :[BOT] " << newOp->getNickname() << " has been promoted to channel operator.";
//                     channel->broadcast(opMsg.str());
//                 }
//             }
//             else
//             {
//                 std::stringstream warnMsg;
//                 warnMsg << ":BOT PRIVMSG " << channel->getName()
//                         << " :[BOT] Warning " << count << "/2 for " << nick << " : inappropriate language detected.";
//                 channel->broadcast(warnMsg.str());
//             }

//             std::cout << "[BOT] Warning " << count << "/2 for " << nick << std::endl;
//             return false;
//         }
//     }
//     return true;
// }

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

                std::string partMsg = ":" + client->getPrefix() + " PART " + channel->getName() + " :[BOT] Repeated inappropriate language.";
                channel->broadcast(partMsg);

                channel->removeMember(client, channelManager);

                if (wasOperator && channel->getOperators().empty() && !channel->getMembers().empty())
                {
                    Client* newOp = channel->getMembers().front();
                    channel->addOperator(newOp);

                    std::string joinMsg = ":localhost " + newOp->getPrefix() + " JOIN " + channel->getName();
                    newOp->reply(joinMsg);

                    channel->sendNamesListTo(newOp);

                    std::string modeMsg = ":localhost MODE " + channel->getName() + " +o " + newOp->getNickname();
                    channel->broadcast(modeMsg);

                    std::stringstream opMsg;
                    opMsg << ":BOT PRIVMSG " << channel->getName()
                          << " :[BOT] " << newOp->getNickname() << " has been promoted to channel operator.";
                    channel->broadcast(opMsg.str());
                }
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
