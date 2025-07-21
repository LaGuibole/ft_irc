#include "Utils.hpp"


std::string Utils::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}

void Utils::sendError(Client* client, const std::string& code, const std::string& target, const std::string& message) {
    std::string texte = ":localhost " + code + " " + client->getNickname();
    if (!target.empty() && target != "NULL")
        texte += " " + target;
    texte += " " + message;
    client->reply(texte);  // Utiliser reply() au lieu de send() direct
}