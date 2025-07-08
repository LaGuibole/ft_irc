#ifndef SERVER_HPP
# define SERVER_HPP

# include <string>
# include <vector>
# include <map>
# include <poll.h>
# include "Client.hpp"
# include "Channel.hpp"

class Server {
private:
    int                         _port;
    std::string                 _password;
    int                         _server_fd;
    std::vector<struct pollfd>  _poll_fds;
    std::map<int, Client*>      _clients;
    std::map<std::string, Channel*> _channels;

    Server();
    Server(const Server& other);
    Server& operator=(const Server& other);

    void setupServerSocket();
    void mainLoop();
    void handleNewConnection();
    void handleClientData(int client_fd);
    void removeClient(int client_fd);
    void processCommand(int client_fd, const std::string& command);

public:
    Server(int port, const std::string& password);
    ~Server();

    void start();
    void init();
};

#endif
