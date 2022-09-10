#pragma once
#include <string>
#include <vector>
#include <sys/types.h>

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

    mem_region_perms_t perms;
    std::string permsStr;

    std::string pathName;
} mem_region_t;

class Process
{
public:
    Process();
    Process(pid_t pid);
    ~Process();

    void SetProcessPid(pid_t pid);

    pid_t GetCurrentPid() const;
    const std::vector<mem_region_t> GetMemoryRegions() const;

private:
    pid_t m_pid;

    void UpdateMemoryRegions();

    void SetMemoryRangeBoundaries(mem_region_t& reg, const std::string& addressRange) const;
    void SetMemoryRegionPerms(mem_region_t& reg, const std::string& perms) const;
};

