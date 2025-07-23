#pragma once

#include <string>
#include <map>
#include <vector>
#include "Client.hpp"
#include "Numerics.hpp"
#include "ChannelManager.hpp"

class ChannelManager;

/**
 * @brief Classe Channel pour gérer les membres et opérateur
 */
class Channel {
    private:
        std::string _name;                  /** Nom du channel */
        std::string _topic;                 /** Topic du channel */
        std::map<int, Client*> _members;    /** Membres du channel */
        std::map<int, Client*> _operators;  /** Opérateurs du channel */
        std::map<int, Client*> _pendingInvites; /** Liste des clients invites en invite only channel */

        bool _inviteOnly;
        bool _topicRestricted;
        std::string _password;
        bool _hasPassword;
        size_t _userLimit;                  /** Limite d'utilisateurs */
        bool _hasUserLimit;                 /** Si la limite est activée */


    public:
        Channel(const std::string& name);
        ~Channel();

        const std::string& getName() const { return _name; }
        const std::string& getTopic() const { return _topic; }
        bool hasUserLimit() const { return _hasUserLimit; }
        size_t getUserLimit() const { return _userLimit; }
        bool isInviteOnly() const { return _inviteOnly; }
        bool isTopicRestricted() const { return _topicRestricted; }

        std::vector<Client*> getMembers() const;
        std::string getModeString() const;
        Client* getMemberByNickname(const std::string& nickname) const;
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

        const std::map<int, Client*>& getOperators() const { return _operators; }

        /**
         * @brief Ajoute un operateur
         * @param client le client qui sera promu operateur
         */
        void addOperator(Client* client);

        /**
         * @brief Retire un opérateur
         * @param client le client à dégrader
         */
        void removeOperator(Client* client);

        /**
         * @brief Définit la limite d'utilisateurs
         * @param limit nombre maximum d'utilisateurs
         */
        void setUserLimit(int limit);

        /**
         * @brief Supprime la limite d'utilisateurs
         */
        void unsetUserLimit();

        /**
         * @brief Add un client au channel
         * @param client Client à add
         */
        void addMember(Client* client);

        /**
         * @brief Tej un client du channel
         * @param client Client à tej
         */
        void removeMember(Client* client, ChannelManager* channel);


        void changeTopicMode();

        /**
         * @brief Broadcast un message à tous les membres
         * @param message Message à broadcaster
         * @param exclude Client à exclure (ex. sender pour PRIVMSG)
         */
        void broadcast(const std::string& message, Client* exclude = NULL);

        void setTopic(const std::string& topic) { _topic = topic; }

        void setInviteOnly(bool enabled) { this->_inviteOnly = enabled; }

        bool isInvited(Client* client) const;

        void addPendingInvite(Client* client);

        void removeInvite(Client* client);

        void setPassword(const std::string& password);

        void unsetPassword();

        bool hasPassword() const { return _hasPassword; }

        const std::string& getPassword() const { return _password; }

        void setTopicRestricted(bool enabled) { this->_topicRestricted = enabled; }

        void sendNamesListTo(Client* client);

        void sendNamesListToAll();
};
