#include "MemoryScanner.h"
#include "MemoryStructs.h"
#include <exception>
#include <stdexcept>

MemoryScanner::MemoryScanner()
{
    this->m_UndoFlag = false;
    this->m_ScanStartedFlag = false;
}

MemoryScanner::~MemoryScanner() {}


void MemoryScanner::Clear()
{
    this->m_CurrScanVector.clear();
    this->m_PrevScanVector.clear();

    this->m_UndoFlag = false;
    this->m_ScanStartedFlag = false;
}

void MemoryScanner::Undo()
{
    if (this->m_PrevScanVector.size() == 0)
    {
        throw std::runtime_error("Nothing to undo.");
    }
    else if (this->m_UndoFlag)
    {
        throw std::runtime_error("Undo has already been called.");
    }
    else
    {
        this->m_CurrScanVector = this->m_PrevScanVector;
        this->m_UndoFlag = true;
    }
}

void MemoryScanner::SetPid(pid_t pid)
{
    this->m_pid = pid;
    this->Clear();
}

const std::vector<MemAddress>& MemoryScanner::GetCurrScanVector() const
{
    return this->m_CurrScanVector;
}

bool MemoryScanner::GetScanStartedFlag() const
{
    return this->m_ScanStartedFlag;
}

