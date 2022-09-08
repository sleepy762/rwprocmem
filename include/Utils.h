#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>

namespace Utils
{
    std::vector<std::string> SplitString(const std::string& str, const char delim);
    std::string GetProcessCommand(const pid_t pid);

    std::vector<uint8_t> ReadProcessMemory(const pid_t pid, const unsigned long baseAddr,
        const unsigned long length);
    ssize_t WriteToProcessMemory(const pid_t pid, const unsigned long baseAddr, 
            const size_t dataSize, void* data);
}

