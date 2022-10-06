#include "MemoryFreezer.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <thread>
#include <vector>
#include "MemoryFuncs.h"
#include <fmt/core.h>

MemoryFreezer::MemoryFreezer()
{
    this->m_EnabledAddressesAmount = 0;
    this->m_ThreadRunning = false;
    this->m_pid = 0;
}

MemoryFreezer::~MemoryFreezer() {}

void MemoryFreezer::AddAddress(MemAddress address, std::string& typeStr, std::string& dataStr, 
        std::vector<uint8_t>& data)
{
    FrozenMemAddress frozenAddr = { address, true, typeStr, dataStr, data };
    this->m_FrozenAddresses.push_back(frozenAddr);
    this->m_EnabledAddressesAmount += 1;

    // We may need to start/restart the thread if there were no addresses before
    // and a new address was added now
    this->StartThreadLoopIfNeeded();
}

void MemoryFreezer::RemoveAddress(size_t index)
{
    if (index > this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.cbegin();
        std::advance(iter, index);

        this->m_FrozenAddresses.erase(iter);
        this->m_EnabledAddressesAmount -= 1;
    }
}

void MemoryFreezer::RemoveAllAddresses()
{
    this->m_FrozenAddresses.clear();
    this->m_EnabledAddressesAmount = 0;
}

void MemoryFreezer::EnableAddress(size_t index)
{
    if (index > this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.begin();
        std::advance(iter, index);

        iter->enabled = true;
        this->m_EnabledAddressesAmount += 1;

        this->StartThreadLoopIfNeeded();
    }
}

void MemoryFreezer::DisableAddress(size_t index)
{
    if (index > this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.begin();
        std::advance(iter, index);

        iter->enabled = false;
        this->m_EnabledAddressesAmount -= 1;
    }
}

void MemoryFreezer::EnableAllAddresses()
{
    for (auto it = this->m_FrozenAddresses.begin(); it != this->m_FrozenAddresses.end(); it++)
    {
        // Only count (and enable) addresses which are currently disabled
        if (!it->enabled)
        {
            it->enabled = true;
            this->m_EnabledAddressesAmount += 1;
        }
    }
    this->StartThreadLoopIfNeeded();
}

void MemoryFreezer::DisableAllAddresses()
{
    for (auto it = this->m_FrozenAddresses.begin(); it != this->m_FrozenAddresses.end(); it++)
    {
        it->enabled = false;
    }
    this->m_EnabledAddressesAmount = 0;
}

const std::list<FrozenMemAddress>& MemoryFreezer::GetFrozenAddresses() const
{
    return this->m_FrozenAddresses;
}

int MemoryFreezer::GetEnabledAddressesAmount() const
{
    return this->m_EnabledAddressesAmount;
}

void MemoryFreezer::SetPid(pid_t pid)
{
    this->m_pid = pid;
    this->m_FrozenAddresses.clear();
    this->m_EnabledAddressesAmount = 0;
}

void MemoryFreezer::StartThreadLoopIfNeeded()
{
    // Start a thread only if there is no thread running already and if there are enabled addresses
    if (!this->m_ThreadRunning && this->m_EnabledAddressesAmount > 0)
    {
        this->m_ThreadRunning = true;

        std::thread th(&MemoryFreezer::ThreadLoop, this);
        th.detach();
    }
}

// TODO: Add mutex locks
// TODO: Add exception handling
void MemoryFreezer::ThreadLoop()
{
    auto it = this->m_FrozenAddresses.begin();
    while (true)
    {
        // Stopping condition
        if (this->m_EnabledAddressesAmount <= 0)
        {
            break;
        }

        // Restart the iterator if it reached the end
        if (it == this->m_FrozenAddresses.end())
        {
            it = this->m_FrozenAddresses.begin();
        }

        // Only freeze enabled addresses
        if (it->enabled)
        {
            const std::vector<uint8_t>& data = it->data; 
            const long dataSize = data.size();
            try
            {
                ssize_t nread = MemoryFuncs::WriteToProcessMemory(this->m_pid, it->memAddress.address, dataSize, (void*)&data[0]);
                // Check for partial write
                if (nread != dataSize)
                {
                    const std::string msg = fmt::format(
                            "WARNING: Disabling address {:#018x} due to a partial write of {}/{}.",
                            it->memAddress.address, nread, data.size());
                    this->m_MessageQueue.push(msg);
                    it->enabled = false;
                }
            }
            catch (const std::exception& e)
            {
                const std::string msg = fmt::format("Error writing to memory location {:#018x}: {}", 
                        it->memAddress.address, e.what());
                this->m_MessageQueue.push(msg);
                it->enabled = false; // Disable the address which caused an exception
            }
        }
        // Move forward
        std::advance(it, 1);
    }
    this->m_ThreadRunning = false;
}

