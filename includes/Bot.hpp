/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: guillaumephilippe <guillaumephilippe@st    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/22 23:08:43 by guillaumeph       #+#    #+#             */
/*   Updated: 2025/07/22 23:22:13 by guillaumeph      ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>
#include <vector>
#include <map>

class Client;

class Bot
{
    private:
        std::vector<std::string> _wordlist;
        std::map<Client *, int> _warnings;
        
    public:
        Bot();
        ~Bot();

        void inspectMessages(Client* client, const std::string& message);
        void loadWordlist();
};