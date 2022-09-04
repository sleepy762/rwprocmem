#include "Utils.h"
#include <sstream>
#include <string>

std::vector<std::string> Utils::SplitString(const std::string& str, const char delim)
{
    std::vector<std::string> tokens;
    
    std::stringstream stream(str);
    std::string intermediate;

    while (std::getline(stream, intermediate, delim))
    {
        tokens.push_back(intermediate);
    }

    return tokens;
}

