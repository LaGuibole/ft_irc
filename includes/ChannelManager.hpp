#pragma once

#include <map>
#include <string>
#include "Channel.hpp"

class Channel;

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

        void removeChannel(Channel* channel);
};
