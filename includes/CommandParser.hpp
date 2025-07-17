#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <string>
#include <vector>
#include <map>
#include "Client.hpp"
#include "ChannelManager.hpp"
#include "Numerics.hpp"

/**
 * @brief Classe pour parser et traiter les commandes
 */
class CommandParser {
    public:
        static void process(int clientFd, const std::string& command,
                            std::map<int, Client*>& clients,
                            ChannelManager& channelManager,
                            const std::string& password);

    private:
        static std::vector<std::string> split(const std::string& str, char delimiter);
        static std::string getMessage(const std::string& command);
        static void applyChannelMode(Client* client, Channel* channel, const std::string& modeFlags, std::vector<std::string>& modeParams);

        // Handlers pour commandes
        static void handlePass(Client* client, const std::vector<std::string>& params, const std::string& password);
        static void handleNick(Client* client, const std::vector<std::string>& params, const std::map<int, Client*>& clients);
        static void handleUser(Client* client, const std::string& command);
        static void handleJoin(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager);
        static void handlePart(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager);
        static void handlePrivmsg(Client* client, const std::string& command, const std::map<int, Client*>& clients, ChannelManager& channelManager);
        static void handleQuit(Client* client, const std::string& command, ChannelManager& channelManager);
        static void handleKick(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager);
        static void handleMode(Client* client, const std::vector<std::string>& params, ChannelManager& channelManager);
        // TODO: Ajouter handleTopic, handleMode, handleKick, handleInvite pour plus tard :)
};

#endif
