#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

/**
 * @brief Classe Client pour gérer les infos et l'état d'un client
 */
class Client {
private:
    int _fileDescriptor;          /** File descriptor de la socket */
    struct sockaddr_in _address;  /** Adresse IP */
    std::string _hostname;        /** Hostname */
    std::string _nickname;        /** Nickname */
    std::string _username;        /** Username */
    std::string _realname;        /** Realname */
    std::string _buffer;          /** Buffer pour données entrantes */
    bool _isPassValidated;        /** Si PASS validé */
    bool _isRegistered;           /** Si fully registered */

public:
    Client(int fd, const struct sockaddr_in& addr);
    ~Client();

    int getFileDescriptor() const { return _fileDescriptor; }
    const std::string& getHostname() const { return _hostname; }
    const std::string& getNickname() const { return _nickname; }
    const std::string& getUsername() const { return _username; }
    const std::string& getRealname() const { return _realname; }
    bool isPassValidated() const { return _isPassValidated; }
    bool isRegistered() const { return _isRegistered; }
    std::string& getBuffer() { return _buffer; }
    std::string getPrefix() const { return _nickname + "!" + _username + "@" + _hostname; }

    void setNickname(const std::string& nick) { _nickname = nick; }
    void setUsername(const std::string& user) { _username = user; }
    void setRealname(const std::string& real) { _realname = real; }
    void setPassValidated(bool validated) { _isPassValidated = validated; }
    void setRegistered(bool registered) { _isRegistered = registered; }

    /**
     * @brief Ajoute des données au buffer
     * @param data Données à ajouter
     */
    void appendToBuffer(const std::string& data) { _buffer += data; }

    /**
     * @brief Efface une partie du buffer
     * @param start Position de départ
     * @param length Longueur à effacer
     */
    void eraseFromBuffer(size_t start, size_t length) { _buffer.erase(start, length); }

    /**
     * @brief Envoie un message au iencli
     * @param message Message à envoyer
     */
    void reply(const std::string& message);

    /**
     * @brief Envoie le message de bienvenue(MOTD)
     */
    void welcome();
};
