/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: david <david@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/08 14:35:40 by guphilip          #+#    #+#             */
/*   Updated: 2025/07/09 01:43:13 by david            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <csignal> // Pour signal, sig_atomic_t, SIGINT
#include <unistd.h> // Pour usleep

volatile sig_atomic_t g_shutdown = 0; // Global volatile pour signal

void signalHandler(int signum)
{
    if (signum == SIGINT)
    {
        Server* server = Server::getInstance();
        if (server)
        {
            server->setRunning(false);
            std::cout << "SIGINT received, shutting down..." << std::endl;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    signal(SIGINT, signalHandler);
    try
    {
        int port = std::atoi(argv[1]);
        std::string password = argv[2];
        Server server(port, password);
        server.start();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
