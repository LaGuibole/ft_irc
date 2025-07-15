#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>
#include "Client.hpp"

/**
 * @brief Classe Channel pour gérer les membres et opérateur
 */
class Channel {
    private:
        std::string _name;                  /** Nom du channel */
        std::string _topic;                 /** Topic du channel */
        std::map<int, Client*> _members;    /** Membres du channel */
        std::map<int, Client*> _operators;  /** Opérateurs du channel */

    public:
         Channel(const std::string& name);
        ~Channel();

        const std::string& getName() const { return _name; }
        const std::string& getTopic() const { return _topic; }
        std::vector<Client*> getMembers() const;

        /**
         * @brief Vérifie si un client est membre ou non
         * @param client Client à checker
         * @return True si membre
         */
        bool isMember(Client* client) const;

        /**
         * @brief Vérifie si un client est opérateur ou non
         * @param client Client à checker
         * @return True si opérateur
         */
        bool isOperator(Client* client) const;

        /**
         * @brief Add un client au channel
         * @param client Client à add
         */
        void addMember(Client* client);

        /**
         * @brief Tej un client du channel
         * @param client Client à tej
         */
        void removeMember(Client* client);

        /**
         * @brief Broadcast un message à tous les membres
         * @param message Message à broadcaster
         * @param exclude Client à exclure (ex. sender pour PRIVMSG)
         */
        void broadcast(const std::string& message, Client* exclude = NULL);

        void setTopic(const std::string& topic) { _topic = topic; }
};

#endif
