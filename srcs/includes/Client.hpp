#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <string>
# include <netinet/in.h>

class Client {
private:
    int         _fd;
    std::string _nickname;
    std::string _username;
    std::string _hostname;
    std::string _buffer;
    bool        _is_registered;

    Client(const Client& other);
    Client& operator=(const Client& other);

public:
    Client(int fd, const struct sockaddr_in& address);
    ~Client();

    int         getFd() const;
    const std::string&       getNickname() const;
    const std::string&       getUsername() const;
    std::string getPrefix() const;
    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void reply(const std::string& message);
    void welcome();
};

#endif
