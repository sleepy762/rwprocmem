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
        throw std::runtime_error("Invalid pid.");
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
        mem_region_t reg = { "", "", "" }; // Empty initialization
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

std::vector<mem_region_t> Process::GetMemoryRegions()
{
    if (this->m_pid == 0)
    {
        throw std::runtime_error("A process pid has not been given. (see command 'pid')");
    }
    // Make sure we are returning the most up to date memory region structs
    this->UpdateMemoryRegions();
    return this->m_memRegions;
}

