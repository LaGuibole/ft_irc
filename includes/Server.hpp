#pragma once

#include <vector>
#include <map>
#include <string>
#include <poll.h>
#include "Client.hpp"
#include "ChannelManager.hpp"
#include "CommandParser.hpp"

/**
 * @brief Classe Server pour gérer le réseau et les iencli
 */
class Server {
    private:
        int _port;                            /** Port d'écoute */
        std::string _password;                /** Mot de passe serveur */
        int _serverFileDescriptor;            /** FD du socket serveur */
        std::vector<struct pollfd> _pollFds;  /** Tableau pour poll() */
        std::map<int, Client*> _clients;      /** Map des clients par FD */
        ChannelManager _channelManager;       /** Gestionnaire de channels */
        volatile bool _running;               /** Flag pour loop actif */
        static Server* _instance;             /** Instance unique (pour signals) */

        void setupServerSocket();
        void mainLoop();
        void handleNewConnection();
        void shutdown();

    public:
        Server(int port, const std::string& password);
        ~Server();

        /**
         * @brief Lance le serveur
         */
        void start();

        /**
         * @brief Set le flag running
         * @param running Nouvelle valeur
         */
        void setRunning(bool running) { _running = running; }

        /**
         * @brief Récupère l'instance unique
         * @return Pointeur sur Server
         */
        static Server* getInstance() { return _instance; }

        /**
         * @brief Set l'instance unique
         * @param server Pointeur sur Server
         */
        static void setInstance(Server* server) { _instance = server; }

        /**
         * @brief Gère les données d'un iencli
         * @param clientFd FD du iencli
         */
        void handleClientData(int clientFd);

        /**
         * @brief Tej un iencli
         * @param clientFd FD du iencli
         */
        void removeClient(int clientFd);
};
