#ifndef CHANNEL_MANAGER_HPP
#define CHANNEL_MANAGER_HPP

#include <map>
#include <string>
#include "Channel.hpp"

/**
 * @brief Manager pour créer et gérer channels
 */
class ChannelManager {
    private:
        std::map<std::string, Channel*> _channels;  /** Map des channels par nom */

    public:
        ChannelManager();
        ~ChannelManager();

        /**
         * @brief Valide le nom d'un canal IRC selon les règles du protocole.
         * En cas d'erreur, un message d'erreur est envoyé au client.
         * @param name Nom du canal à valider.
         * @param client Pointeur vers le client à qui envoyer les messages d'erreur.
         * @return true si le nom est valide, false sinon.
         */
        bool validateChannelName(const std::string& name, Client* client);

        /**
         * @brief Récupère ou crée un channel
         * @param name Nom du channel
         * @return Pointeur sur Channel
         */
        Channel* getOrCreateChannel(const std::string& name);

        /**
         * @brief Récupère un channel existant
         * @param name Nom du channel
         * @return Pointeur sur Channel ou NULL
         */
        Channel* getChannel(const std::string& name);

        /**
         * @brief Tej un iencli de tous les channels
         * @param client Iencli à tej
         */
        void removeClientFromAll(Client* client);
        
};

#endif
