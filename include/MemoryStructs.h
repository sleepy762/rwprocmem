#pragma once
#include <string>

typedef struct mem_region_perms_t
{
    bool readFlag;
    bool writeFlag;
    bool executeFlag;
    bool sharedFlag;
} mem_region_perms_t;

typedef struct mem_region_t
{
    std::string addressRangeStr;
    unsigned long startAddr;
    unsigned long endAddr;
    unsigned long rangeLength;

    std::string permsStr;
    mem_region_perms_t perms;

    std::string pathName;
} mem_region_t;

// Used to store a specific address along with the memory region it belongs to
typedef struct mem_address_t
{
    unsigned long address;
    mem_region_t memRegion;
} mem_address_t;

