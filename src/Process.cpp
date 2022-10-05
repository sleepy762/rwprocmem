#include "Process.h"
#include "Utils.h"
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include <stdexcept>
#include <fstream>
#include <fmt/core.h>

Process::Process()
{
    this->m_pid = 0;
    this->m_MemoryScanner = MemoryScanner();
}

Process::Process(pid_t pid)
{
    this->SetProcessPid(pid);
}

Process::~Process() {}


void Process::SetProcessPid(pid_t pid)
{
    // Ensure the given pid is valid
    // This doesn't check if the pid actually exists
    if (pid > 0 && pid != getpid())
    {
        this->m_pid = pid;
        this->m_MemoryScanner.SetPid(pid);
    }
    else
    {
        throw std::invalid_argument("Invalid pid.");
    }
}

void Process::SetMemoryRangeBoundaries(MemRegion& reg, const std::string& addressRange) const
{
    // The delimiter in the address range is '-', as seen in any maps file
    std::vector<std::string> addressTokens = Utils::SplitString(addressRange, '-');

    // The start address is in the 1st index, the end address is in the 2nd index
    // The addresses are in hexadecimal
    reg.startAddr = std::stoul(addressTokens[0], nullptr, 16);
    reg.endAddr = std::stoul(addressTokens[1], nullptr, 16);
}

void Process::SetMemoryRegionPerms(MemRegion& reg, const std::string& perms) const
{
    reg.permsStr = perms;
    reg.perms = { false, false, false, false }; // Default initialization

    // The structure of the permissions string is constant and when all
    // permissions are set looks like this: rwxp
    // The meaning of each permission from left to right is
    // Readable, writable, executable, private (this can be either private(p) or shared (s))
    // In this case, we check if the region is shared memory
    if (perms[0] == 'r')
    {
        reg.perms.readFlag = true;
    }
    if (perms[1] == 'w')
    {
        reg.perms.writeFlag = true;
    }
    if (perms[2] == 'x')
    {
        reg.perms.executeFlag = true;
    }
    if (perms[3] == 's')
    {
        reg.perms.sharedFlag = true;
    }
}

pid_t Process::GetCurrentPid() const
{
    return this->m_pid;
}

const std::vector<MemRegion> Process::GetMemoryRegions() const
{
    // Make sure we are returning the most up to date memory region structs
    std::string processMapPath = fmt::format("/proc/{}/maps", this->m_pid);
    std::ifstream processMap(processMapPath);

    if (!processMap.is_open())
    {
        const std::string errMsg = fmt::format("Failed to open the maps for pid {}: {}.", 
                this->m_pid, std::strerror(errno));
        throw std::runtime_error(errMsg);
    }
    
    std::vector<MemRegion> memRegions;
    std::string line;
    while (std::getline(processMap, line))
    {
        MemRegion reg;
        std::vector<std::string> tokens = Utils::SplitString(line, ' ');
        
        // The /proc/pid/maps file always has the same pattern, therefore:
        // tokens[0] has the memory address range
        // tokens[1] holds the permissions
        // tokens[2] holds the offset (unused)
        // tokens[3] holds the device (unused)
        // tokens[4] holds the inode (unused)
        // tokens[5] holds the pathname (optional)

        // Sets the start and end addresses of the region
        this->SetMemoryRangeBoundaries(reg, tokens[0]);

        // Calculate the address range length
        reg.rangeLength = reg.endAddr - reg.startAddr;

        // Set the actual permissions
        this->SetMemoryRegionPerms(reg, tokens[1]);

        // This is the minimum amount of fields in the maps file
        // when there is no "pathname" field
        constexpr int MIN_AMOUNT_OF_FIELDS = 5;

        // We have to make sure that the pathname token exists (sometimes it doesn't)
        if (tokens.size() > MIN_AMOUNT_OF_FIELDS)
        {
            reg.pathName = Utils::JoinVectorOfStrings(tokens, MIN_AMOUNT_OF_FIELDS, ' ');
        }
        else
        {
            reg.pathName = "unknown";
        }

        // This is here in order to prevent errors when running commands that scan all memory regions
        // The [vvar] region, even though marked as readable in the maps file, in reality
        // is not readable, so it is safe to disable this flag here
        if (reg.pathName == "[vvar]")
        {
            reg.perms.readFlag = false;
        }
        memRegions.push_back(reg);
    }
    return memRegions;
}

MemoryScanner& Process::GetMemoryScanner()
{
    return this->m_MemoryScanner;
}

