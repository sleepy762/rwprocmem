#include "Utils.h"
#include <sstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <fstream>
#include <cstring>

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

// Returns cmdline if it's not empty
// Otherwise returns comm
std::string Utils::GetProcessCommand(pid_t pid)
{
    std::string basePath = "/proc/" + std::to_string(pid);
    std::string cmdLinePath = basePath + "/cmdline";

    std::ifstream cmdLineFile(cmdLinePath);
    if (!cmdLineFile.is_open())
    {
        const std::string err = "Failed to open file '" + cmdLinePath + "': " + std::strerror(errno);
        throw std::runtime_error(err);
    }

    std::stringstream cmdLine;
    cmdLine << cmdLineFile.rdbuf();

    // Return the command line string if it's not empty
    // if it is empty then the "comm" will be returned
    if (cmdLine.str().size() != 0)
    {
        return cmdLine.str();
    }
    cmdLineFile.close();

    std::string commPath = basePath + "/comm";

    std::ifstream commFile(commPath);
    if (!commFile.is_open())
    {
        const std::string err = "Failed to open file '" + commPath + "': " + std::strerror(errno);
        throw std::runtime_error(err);
    }

    std::stringstream comm;
    comm << commFile.rdbuf();

    std::string commStr = "[]";
    if (!comm.str().empty())
    {
        commStr = '[' + comm.str();
        commStr.pop_back(); // Remove the 0x0A character (newline)
        commStr += ']';
    }
    return commStr;
}

