/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guphilip <guphilip@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 23:08:43 by guillaumeph       #+#    #+#             */
/*   Updated: 2025/07/23 10:13:25 by guphilip         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "Channel.hpp"

class Client;

class Bot
{
    private:
        std::vector<std::string> _wordlist;
        std::map<Client *, int> _warnings;

    public:
        Bot();
        ~Bot();

        bool inspectMessages(Client* client, const std::string& message, Channel* channel, ChannelManager* channelManager);
        void loadWordlist();
};
