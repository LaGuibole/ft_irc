/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guillaumephilippe <guillaumephilippe@st    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 23:08:46 by guillaumeph       #+#    #+#             */
/*   Updated: 2025/07/22 23:26:38 by guillaumeph      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Bot.hpp"
#include "Client.hpp"
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

void Bot::inspectMessages(Client* client, const std::string& message)
{
    for (size_t i = 0; i < _wordlist.size(); ++i)
    {
        if (message.find(_wordlist[i]) != std::string::npos) // -> std::string::npos [const qui vaut -1]
        {
            _warnings[client]++;
            int count = _warnings[client];

            std::string nick = client->getNickname();
            if (count > 2)
            {
                client->reply(":localhost NOTICE " + nick + " :[BOT] You have been banned for inappropriate language.");
                client->setToDisconnect(true);
            }
            else
                client->reply(":localhost NOTICE " + nick + " :[BOT] Warning ! Inappropriate language detected.");
            std::cout << "[BOT] Warning " << count << "/ 2 for " << nick << std::endl;
            return;
        }
    }
}