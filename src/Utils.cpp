#include "Utils.h"
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <sys/uio.h>

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

// Returns the contents of /proc/.../cmdline if it's not empty
// Otherwise returns the contents of /proc/.../comm
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

// This function should be used when the requested memory region is readable (r permission is set)
// process_vm_readv will fail if the region is not readable and ptrace should be used instead
std::vector<uint8_t> Utils::ReadReadableProcMemory(pid_t pid, unsigned long baseAddr,
        unsigned long length)
{
    iovec local[1];
    local[0].iov_base = new uint8_t[length];
    local[0].iov_len = length;

    iovec remote[1];
    remote[0].iov_base = (void*)baseAddr;
    remote[0].iov_len = length;

    ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
    if (nread < 0)
    {
        std::string errMsg;
        switch (errno)
        {
            case EFAULT:
                errMsg = "Memory address is outside the accessible space of the process.";
                break;

            case EINVAL:
                errMsg = "Invalid arguments.";
                break;

            case ENOMEM:
                errMsg = "Failed to allocate memory for iovec structures.";
                break;

            case EPERM:
                errMsg = "Permission denied.";
                break;

            case ESRCH:
                errMsg = "Invalid PID (process doesn't exist).";
                break;

            default:
                errMsg = "Unknown error.";
                break;
        }
        delete[] (uint8_t*)local[0].iov_base;
        throw std::runtime_error(errMsg);
    }

    std::vector<uint8_t> dataVec;
    // Copy the data given from process_vm_readv into the byte vector
    dataVec.insert(dataVec.end(), &((uint8_t*)local[0].iov_base)[0], &((uint8_t*)local[0].iov_base)[length]);

    return dataVec;
}

