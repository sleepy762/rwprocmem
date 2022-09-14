#pragma once
#include <string>

struct MemRegionPerms
{
    bool readFlag;
    bool writeFlag;
    bool executeFlag;
    bool sharedFlag;
};

struct MemRegion
{
    std::string addressRangeStr;
    unsigned long startAddr;
    unsigned long endAddr;
    unsigned long rangeLength;

    std::string permsStr;
    MemRegionPerms perms;

    std::string pathName;
};

// Used to store a specific address along with the memory region it belongs to
struct MemAddress
{
    unsigned long address;
    MemRegion memRegion;
};

