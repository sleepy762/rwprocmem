#include "Process.h"
#include "Utils.h"
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <cstring>

Process::Process()
{
    this->m_pid = 0;
}

Process::Process(pid_t pid)
{
    this->SetProcessPid(pid);
}

Process::~Process() {}


void Process::SetProcessPid(pid_t pid)
{
    // Ensure the given pid is valid
    if (pid > 0 && pid != getpid())
    {
        this->m_pid = pid;
    }
    else
    {
        throw std::invalid_argument("Invalid pid.");
    }
}

void Process::UpdateMemoryRegions()
{
    std::string processMapPath = "/proc/" + std::to_string(this->m_pid) + "/maps";
    std::ifstream processMap(processMapPath);

    if (!processMap.is_open())
    {
        const std::string errMsg = "Failed to open the maps for pid " + std::to_string(this->m_pid)
            + ": " + strerror(errno);
        throw std::runtime_error(errMsg);
    }

    this->m_memRegions.clear();
    
    std::string line;
    while (std::getline(processMap, line))
    {
        mem_region_t reg = { "", 0, 0, 0, "", "" }; // Empty initialization
        std::vector<std::string> tokens = Utils::SplitString(line, ' ');
        
        // The /proc/pid/maps file always has the same pattern, therefore:
        // tokens[0] has the memory address range
        // tokens[1] holds the permissions
        // tokens[2] holds the offset (unused)
        // tokens[3] holds the device (unused)
        // tokens[4] holds the inode (unused)
        // tokens[5] holds the pathname (optional)

        // Set the memory address range string
        reg.addressRange = tokens[0];

        // The delimiter in the address range is '-', as seen in any maps file
        std::vector<std::string> addressTokens = Utils::SplitString(tokens[0], '-');

        // The start address is in the 1st index, the end address is in the 2nd index
        // The addresses are in hexadecimal
        reg.startAddr = std::stoul(addressTokens[0], nullptr, 16);
        reg.endAddr = std::stoul(addressTokens[1], nullptr, 16);
       
        // Calculate the address range length
        reg.rangeLength = reg.endAddr - reg.startAddr;

        // Set the perms string
        reg.perms = tokens[1];
    
        // This is the minimum amount of fields in the maps file
        // when there is no "pathname" field
        constexpr int MIN_AMOUNT_OF_FIELDS = 5;

        // We have to make sure that the pathname token exists (sometimes it doesn't)
        if (tokens.size() > MIN_AMOUNT_OF_FIELDS)
        {
            // The initial iterator starts where the pathname begins
            for (auto it = tokens.begin() + MIN_AMOUNT_OF_FIELDS; it != tokens.end(); it++)
            {
                reg.pathName += *it;
                if (it + 1 != tokens.end())
                {
                    // Add a space in between the strings
                    reg.pathName += ' ';
                }
            }
        }
        else
        {
            reg.pathName = "unknown";
        }

        this->m_memRegions.push_back(reg);
    }
}

pid_t Process::GetCurrentPid() const
{
    return this->m_pid;
}

std::vector<mem_region_t> Process::GetMemoryRegions()
{
    // Make sure we are returning the most up to date memory region structs
    this->UpdateMemoryRegions();
    return this->m_memRegions;
}

