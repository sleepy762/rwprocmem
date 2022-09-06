#pragma once
#include <string>
#include <vector>
#include <sys/types.h>

typedef struct mem_region_t
{
    std::string addressRange;

    unsigned long startAddr;
    unsigned long endAddr;
    unsigned long rangeLength;

    std::string perms;
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
    std::vector<mem_region_t> GetMemoryRegions(); 

private:
    pid_t m_pid;
    std::vector<mem_region_t> m_memRegions;

    void UpdateMemoryRegions();
};

