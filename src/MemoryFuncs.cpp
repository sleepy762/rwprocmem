#include "MemoryFuncs.h"
#include <memory>
#include <sys/uio.h>
#include <fmt/core.h>

// Returns an error message when process_vm_readv/process_vm_writev fail
// Parameter expects errno
static std::string GetErrorMessage(int err)
{
    std::string errMsg;
    switch (err)
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
    return errMsg;
}

// This function should be used when the requested memory region is readable (r permission is set)
// process_vm_readv will fail if the region is not readable and ptrace should be used instead
// TODO: create a ptrace alternative for both read and write functions
std::vector<uint8_t> MemoryFuncs::ReadProcessMemory(pid_t pid, unsigned long baseAddr, long length)
{
    std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(length);

    iovec local[1];
    local[0].iov_base = buffer.get();
    local[0].iov_len = length;

    iovec remote[1];
    remote[0].iov_base = (void*)baseAddr;
    remote[0].iov_len = length;

    ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
    if (nread < 0)
    {
        throw std::runtime_error(GetErrorMessage(errno));
    }
    else if (nread != length)
    {
        fmt::print("WARNING: Partial read of {}/{} bytes.\n", nread, length);
    }

    std::vector<uint8_t> dataVec;
    // Copy the data given from process_vm_readv into the byte vector
    dataVec.insert(dataVec.end(), &buffer.get()[0], &buffer.get()[nread]);

    return dataVec;
}

void MemoryFuncs::WriteToProcessMemory(pid_t pid, unsigned long baseAddr, long dataSize, void* data)
{
    iovec local[1];
    local[0].iov_base = data;
    local[0].iov_len = dataSize;

    iovec remote[1];
    remote[0].iov_base = (void*)baseAddr;
    remote[0].iov_len = dataSize;

    ssize_t nread = process_vm_writev(pid, local, 1, remote, 1, 0);
    if (nread < 0)
    {
        throw std::runtime_error(GetErrorMessage(errno));
    }
    else if (nread != dataSize)
    {
        fmt::print("WARNING: Partial write of {}/{} bytes.\n", nread, dataSize);
    }
}

// Returns a vector of the memory regions where the given data was found
// dataToFind can be of any type
// dataSize is the size of the type / length of string (if string type is used)
std::vector<MemAddress> MemoryFuncs::FindDataInMemory(pid_t pid, 
        const std::vector<MemRegion>& memRegions, size_t dataSize, const void* dataToFind)
{
    // Vector of the memory addresses with the found data
    std::vector<MemAddress> addrs;

    for (auto it = memRegions.cbegin(); it != memRegions.cend(); it++)
    {
        // Skip unreadable memory regions
        if (!it->perms.readFlag)
        {
            continue;
        }
        
        // TODO: implement a limit on how much memory can be read at a time
        std::vector<uint8_t> regMemory = MemoryFuncs::ReadProcessMemory(pid, it->startAddr, it->rangeLength);

        // The vector is a contiguous array in memory so we can do this
        const unsigned char* dataPtr = &regMemory[0];
        for (unsigned long i = 0; i < regMemory.size(); i++)
        {
            // We always want to have at least dataTypeSize bytes
            if (regMemory.size() - i < dataSize)
            {
                break;
            }

            const unsigned char* offsetDataPtr = dataPtr + i;
            if (std::memcmp(dataToFind, offsetDataPtr, dataSize) == 0) // Check if the value is the same
            {
                // Store the memory address where the data was found
                MemAddress addrStruct = { it->startAddr + i, *it };
                addrs.push_back(addrStruct);
            }
        }
    }
    return addrs;
}

