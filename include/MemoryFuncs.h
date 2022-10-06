#pragma once
#include <cstring>
#include <exception>
#include <fmt/core.h>
#include <stdexcept>
#include <vector>
#include <sys/types.h>
#include <cstdint>
#include "MemoryStructs.h"
#include "ComparisonType.h"

namespace MemoryFuncs
{
    // Wrappers for process_vm_readv/process_vm_writev respectively
    std::vector<uint8_t> ReadProcessMemory(pid_t pid, unsigned long baseAddr, long length);
    ssize_t WriteToProcessMemory(pid_t pid, unsigned long baseAddr, long dataSize, void* data);
    
    // Compares two values based on the given comparison type
    template <typename T>
    bool CompareData(const void* lhs, const void* rhs, size_t dataSize, ComparisonType cmpType);

    template <>
    bool CompareData<std::string>(const void* lhs, const void* rhs, size_t dataSize, 
            ComparisonType cmpType);

    // Returns a vector of the memory addresses where the given data was found
    // dataToFind can be of any type
    // dataSize is the size of the type / length of string (if string type is used)
    // This overload checks a region of addresses
    template <typename T>
    std::vector<MemAddress> FindDataInMemory(pid_t pid, const std::vector<MemRegion>& memRegions, 
            size_t dataSize, const void* dataToFind, ComparisonType cmpType); 

    // This overload checks a vector of addresses
    template <typename T>
    std::vector<MemAddress> FindDataInMemory(pid_t pid, const std::vector<MemAddress>& memAddrs, 
            size_t dataSize, const void* dataToFind, ComparisonType cmpType); 
}


template <typename T>
bool MemoryFuncs::CompareData(const void* lhs, const void* rhs, size_t dataSize,
        ComparisonType cmpType)
{
    (void)dataSize; // Used only in the std::string specialization

    // Cast the void pointers into values
    T lhsVal = *(T*)lhs;
    T rhsVal = *(T*)rhs;
    switch (cmpType)
    {
        case ComparisonType::Equal:
            return lhsVal == rhsVal;

        case ComparisonType::NotEqual:
            return lhsVal != rhsVal;

        case ComparisonType::Greater:
            return lhsVal > rhsVal;

        case ComparisonType::Less:
            return lhsVal < rhsVal;

        case ComparisonType::GreaterEqual:
            return lhsVal >= rhsVal;

        case ComparisonType::LessEqual:
            return lhsVal <= rhsVal;

        default:
            throw std::runtime_error("Invalid comparison type in CompareMemoryValue()");
    }
}

template <typename T>
std::vector<MemAddress> MemoryFuncs::FindDataInMemory(pid_t pid, const std::vector<MemRegion>& memRegions, 
        size_t dataSize, const void* dataToFind, ComparisonType cmpType)
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
        std::vector<uint8_t> regMemory;
        try
        {
            regMemory = MemoryFuncs::ReadProcessMemory(pid, it->startAddr, it->rangeLength);
        }
        catch (const std::exception& e)
        {
            fmt::print(stderr, "WARNING: Error reading memory region {:#018x} ({}): {}\n", 
                    it->startAddr, it->pathName, e.what());
            continue;
        }

        // Check if there was a partial read
        if (regMemory.size() != it->rangeLength)
        {
            fmt::print("WARNING: Partial read of {}/{} bytes at memory address {:#018x}.\n",
                    regMemory.size(), it->rangeLength, it->startAddr);
        }

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
            // offsetDataPtr should be the lhs, dataToFind should be rhs
            if (MemoryFuncs::CompareData<T>((void*)offsetDataPtr, dataToFind, dataSize, cmpType))
            {
                // Store the memory address where the data was found
                MemAddress addrStruct = { it->startAddr + i, *it };
                addrs.push_back(addrStruct);
            }
        }
    }
    return addrs;
}

// This overload checks a vector of addresses
template <typename T>
std::vector<MemAddress> MemoryFuncs::FindDataInMemory(pid_t pid, const std::vector<MemAddress>& memAddrs, 
        size_t dataSize, const void* dataToFind, ComparisonType cmpType)
{
    // Vector of memory addresses with the found data
    std::vector<MemAddress> addrs;

    for (auto it = memAddrs.cbegin(); it != memAddrs.cend(); it++)
    {
        // Skip unreadable addresses
        if (!it->memRegion.perms.readFlag)
        {
            continue;
        }

        // When trying to read from individual addresses, some addresses may no longer be used by
        // the process we read them from.
        std::vector<uint8_t> addrMemory;
        try
        {
            addrMemory = MemoryFuncs::ReadProcessMemory(pid, it->address, dataSize);
        }
        catch (const std::exception& e)
        {
            fmt::print(stderr, "WARNING: Error reading memory address {:#018x}: {}\n", 
                    it->address, e.what());
            continue;
        }

        // Check if there was a partial read
        if (addrMemory.size() != dataSize)
        {
            fmt::print("WARNING: Partial read of {}/{} at memory address {:#018x}.\n",
                    addrMemory.size(), dataSize, it->address);
        }

        // The target data should be in the rhs
        if (MemoryFuncs::CompareData<T>((void*)&addrMemory[0], dataToFind, dataSize, cmpType))
        {
            addrs.push_back(*it);
        }
    }
    return addrs;
}

