#pragma once
#include <vector>
#include <sys/types.h>
#include "MemoryStructs.h"
#include "ComparisonType.h"
#include <stdexcept>
#include "MemoryFuncs.h"

class MemoryScanner
{
public:
    MemoryScanner();
    ~MemoryScanner();

    void Clear();
    void Undo();

    template <typename T>
    size_t NewScan(const std::vector<MemRegion>& memRegions, size_t dataSize, const void* data,
            ComparisonType cmpType);
    
    template <typename T>
    size_t NextScan(size_t dataSize, const void* data, ComparisonType cmpType);

    void SetPid(pid_t pid);

    const std::vector<MemAddress>& GetCurrScanVector() const;
    bool GetScanStartedFlag() const;
    
private:
    bool m_UndoFlag;
    bool m_ScanStartedFlag;

    pid_t m_pid;
    std::vector<MemAddress> m_CurrScanVector;
    std::vector<MemAddress> m_PrevScanVector;
};


// Returns the amount of addresses where the data was found
template <typename T>
size_t MemoryScanner::NewScan(const std::vector<MemRegion>& memRegions, size_t dataSize, const void* data,
        ComparisonType cmpType)
{
    // This should never happen
    if (this->m_ScanStartedFlag)
    {
        throw std::runtime_error("Incorrect call to NewScan after a scan has already begun.");
    }

    this->m_CurrScanVector = MemoryFuncs::FindDataInMemory<T>(this->m_pid, memRegions, dataSize, 
            data, cmpType);
    this->m_UndoFlag = false; // Reset the undo flag
    this->m_ScanStartedFlag = true;

    return this->m_CurrScanVector.size();
}

// Also returns the amount of addresses where the data was found
template <typename T>
size_t MemoryScanner::NextScan(size_t dataSize, const void* data, ComparisonType cmpType)
{
    auto temporary = this->m_CurrScanVector;
    
    this->m_CurrScanVector = MemoryFuncs::FindDataInMemory<T>(this->m_pid, this->m_CurrScanVector,
            dataSize, data, cmpType);

    // Replace the previous scan vector only if the scan succeeded
    this->m_PrevScanVector = temporary;
    this->m_UndoFlag = false; // Reset the undo flag

    return this->m_CurrScanVector.size();
}

