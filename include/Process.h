#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include "MemoryStructs.h"
#include "MemoryScanner.h"

class Process
{
public:
    Process();
    Process(pid_t pid);
    ~Process();

    void SetProcessPid(pid_t pid);

    pid_t GetCurrentPid() const;
    const std::vector<MemRegion> GetMemoryRegions() const;
    MemoryScanner& GetMemoryScanner();

private:
    pid_t m_pid;
    MemoryScanner m_MemoryScanner;

    void UpdateMemoryRegions();

    void SetMemoryRangeBoundaries(MemRegion& reg, const std::string& addressRange) const;
    void SetMemoryRegionPerms(MemRegion& reg, const std::string& perms) const;
};

