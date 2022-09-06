#include "Utils.h"
#include <sstream>
#include <string>
#include <algorithm>

std::vector<std::string> Utils::SplitString(const std::string& str, const char delim)
{
    std::vector<std::string> tokens;
    
    std::stringstream stream(str);
    std::string intermediate;

    while (std::getline(stream, intermediate, delim))
    {
        if (intermediate.size() != 0)
        {
            tokens.push_back(intermediate);
        }
    }

    return tokens;
}

