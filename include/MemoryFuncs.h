#pragma once
#include <vector>
#include <sys/types.h>
#include <cstdint>
#include "MemoryStructs.h"

namespace MemoryFuncs
{
    // Wrappers for process_vm_readv/process_vm_writev respectively
    std::vector<uint8_t> ReadProcessMemory(pid_t pid, unsigned long baseAddr, long length);
    void WriteToProcessMemory(pid_t pid, unsigned long baseAddr, long dataSize, void* data);
    
    std::vector<MemAddress> FindDataInMemory(pid_t pid, 
            const std::vector<MemRegion>& memRegions, size_t dataSize, const void* dataToFind);
}

