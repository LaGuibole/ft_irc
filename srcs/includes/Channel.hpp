#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>
# include <map>
# include "Client.hpp"

class Channel {
private:
    std::string              _name;
    std::string              _topic;
    std::map<int, Client*>   _clients;
    std::map<int, Client*>   _operators;

    Channel(const Channel& other);
    Channel& operator=(const Channel& other);

public:
    Channel(const std::string& name);
    ~Channel();

    void addClient(Client* client);
    void removeClient(Client* client);
    void broadcast(const std::string& message, Client* exclude_client);
    const std::string& getName() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);
    bool isOperator(Client* client); 
};

#endif
