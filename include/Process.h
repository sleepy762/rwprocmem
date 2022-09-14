#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include "MemoryStructs.h"

class Process
{
public:
    Process();
    Process(pid_t pid);
    ~Process();

    void SetProcessPid(pid_t pid);

    pid_t GetCurrentPid() const;
    const std::vector<MemRegion> GetMemoryRegions() const;

private:
    pid_t m_pid;

    void UpdateMemoryRegions();

    void SetMemoryRangeBoundaries(MemRegion& reg, const std::string& addressRange) const;
    void SetMemoryRegionPerms(MemRegion& reg, const std::string& perms) const;
};

