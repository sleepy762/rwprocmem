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
        std::string errMsg = "Failed to open the maps for pid " + std::to_string(this->m_pid)
            + ": " + strerror(errno);
        throw std::runtime_error(errMsg);
    }

    this->m_memRegions.clear();
    
    std::string line;
    while (std::getline(processMap, line))
    {
        mem_region_t reg = { "", 0, "", "" }; // Empty initialization
        std::vector<std::string> tokens = Utils::SplitString(line, ' ');
        
        // The /proc/pid/maps file always has the same pattern, therefore:
        // tokens[0] has the memory address range
        // tokens[1] holds the permissions
        // tokens[2] holds the offset (unused)
        // tokens[3] holds the device (unused)
        // tokens[4] holds the inode (unused)
        // tokens[5] holds the pathname (optional)
        
        // Set the memory address range
        reg.addressRange = tokens[0];

        // Calculate the address range length
        reg.rangeLength = this->CalculateAddressRangeLength(tokens[0]);

        // Set the perms string
        reg.perms = tokens[1];
        
        // We have to make sure that the pathname token exists (sometimes it doesn't)
        // If pathname doesnt exist then the last token is "0"
        if (tokens.back() != "0")
        {
            // Set the pathname
            reg.pathName = tokens.back();
        }
        else
        {
            reg.pathName = "unknown";
        }

        this->m_memRegions.push_back(reg);
    }
}

// This method is specific to this class, hence the parameter
// It is used to calculate the length of the address range when reading
// address regions from a /proc/pid/maps file
unsigned long Process::CalculateAddressRangeLength(const std::string& rangeStr)
{
    // The delimiter in the address range is '-', as seen in any maps file
    std::vector<std::string> tokens = Utils::SplitString(rangeStr, '-');

    // Now tokens[0] is the start address, tokens[1] is the end address
    // The addresses are in hexadecimal
    unsigned long startAddr = std::stoul(tokens[0], nullptr, 16);
    unsigned long endAddr = std::stoul(tokens[1], nullptr, 16);

    return endAddr - startAddr;
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

