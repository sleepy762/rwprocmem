#include "Utils.h"
#include <sstream>
#include <stdexcept>
#include <string>
#include <fstream>
#include <fmt/core.h>

// Splits a string into a vector of strings
std::vector<std::string> Utils::SplitString(const std::string& str, char delim)
{
    std::vector<std::string> tokens;
    if (str.empty()) // Return an empty vector if the string is empty
    {
        return tokens;
    }

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

// Returns the contents of /proc/.../cmdline if it's not empty
// Otherwise returns the contents of /proc/.../comm
std::string Utils::GetProcessCommand(pid_t pid)
{
    const std::string basePath = fmt::format("/proc/{}", pid);
    const std::string cmdLinePath = fmt::format("{}/cmdline", basePath);

    std::ifstream cmdLineFile(cmdLinePath);
    if (!cmdLineFile.is_open())
    {
        const std::string err = fmt::format("Failed to open file '{}': {}.", cmdLinePath, std::strerror(errno));
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

    const std::string commPath = fmt::format("{}/comm", basePath);

    std::ifstream commFile(commPath);
    if (!commFile.is_open())
    {
        const std::string err = fmt::format("Failed to open file '{}': {}.", commPath, std::strerror(errno));
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

std::string Utils::JoinVectorOfStrings(const std::vector<std::string>& vec, int startIndex, char joinChar)
{
    // Merge all the strings into 1 string
    std::string fullString = "";
    for (auto it = vec.cbegin() + startIndex; it != vec.cend(); it++)
    {
        fullString += *it;
        // Add the given character in between the strings when joining them
        if (it + 1 != vec.cend())
        {
            fullString += joinChar;
        }
    }
    return fullString;
}

void Utils::PrintMemoryAddresses(const std::vector<MemAddress>& memAddrs)
{
    // Gets the amount of digits in the number of found addresses
    const size_t indexWidth = std::to_string(memAddrs.size()).size();

    int index = 0;
    for (auto it = memAddrs.cbegin(); it != memAddrs.cend(); it++)
    {
        fmt::print("[{:{}}] {:#018x} [{}] (in {})\n",
                index, indexWidth, it->address, it->memRegion.permsStr, it->memRegion.pathName);
        index++;
    }
}

MemRegion Utils::FindRegionOfAddress(const std::vector<MemRegion> &memRegions, unsigned long address)
{
    for (auto it = memRegions.cbegin(); it != memRegions.cend(); it++)
    {
        if (address >= it->startAddr && address <= it->endAddr)
        {
            return *it;
        }
    }

    const std::string err = fmt::format("Couldn't find the memory region of the address {:#018x}.", address);
    throw std::runtime_error(err);
}

