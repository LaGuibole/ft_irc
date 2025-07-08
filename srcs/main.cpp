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

int main(int argc, char **argv)
{
    if (argc != 3)
	{
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }
    int port = std::atoi(argv[1]);
    if (port <= 1024 || port > 65535)
	{
        std::cerr << "Error: Port must be between 1025 and 65535." << std::endl;
        return 1;
    }
    std::string password = argv[2];
    if (password.empty())
	{
        std::cerr << "Error: Password cannot be empty." << std::endl;
        return 1;
    }
    try
	{
        Server srv(port, password);
        srv.start();
    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}

