/**
 * @file Utils.cpp
 * @brief Implémentation des utilitaires.
 */

#include "Utils.hpp"


std::string Utils::trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, last - first + 1);
}
