#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>

namespace Utils
{
    std::vector<std::string> SplitString(const std::string& str, const char delim);
    std::string GetProcessCommand(pid_t pid);

    std::vector<uint8_t> ReadProcessMemory(pid_t pid, unsigned long baseAddr,
        unsigned long length);
}

