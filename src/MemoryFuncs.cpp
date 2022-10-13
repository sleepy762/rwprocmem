#include "MemoryFuncs.h"
#include <sys/uio.h>
#include <fmt/core.h>
#include <vector>

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
    // Creates a vector of the size given in `length`
    std::vector<uint8_t> buffer(length);

    iovec local[1];
    local[0].iov_base = &buffer[0]; // Pass a pointer to the beginning of the buffer
    local[0].iov_len = length;

    iovec remote[1];
    remote[0].iov_base = (void*)baseAddr;
    remote[0].iov_len = length;

    ssize_t nread = process_vm_readv(pid, local, 1, remote, 1, 0);
    if (nread < 0)
    {
        throw std::runtime_error(GetErrorMessage(errno));
    }

    return buffer;
}

ssize_t MemoryFuncs::WriteToProcessMemory(pid_t pid, unsigned long baseAddr, long dataSize, void* data)
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
    return nread;
}

template <>
bool MemoryFuncs::CompareData<std::string>(const void* lhs, const void* rhs, 
            size_t dataSize, ComparisonType cmpType)
{
    if (cmpType != ComparisonType::Equal)
    {
        throw std::runtime_error("Comparing strings for equality is the only supported comparison type.");
    }
    return std::memcmp(lhs, rhs, dataSize) == 0;
}

