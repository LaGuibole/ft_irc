#pragma once

#include <string>
#include <vector>
#include "Client.hpp"

/**
 * @brief Namespace pour utilitaires
 */
namespace Utils {
    /**
     * @brief Trim espaces d'une string
     * @param str String à trim
     * @return String trimmée
     */
    void sendError(Client* client, const std::string& code, const std::string& target, const std::string& message);
	std::string trim(const std::string& str);
}

