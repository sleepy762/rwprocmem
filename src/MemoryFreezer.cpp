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

void MemoryFreezer::AddAddress(MemAddress memAddress, const std::string& typeStr, const std::string& dataStr, 
        std::vector<uint8_t>& data, const std::string& note)
{
    // Prevent adding read-only addresses
    if (!memAddress.memRegion.perms.writeFlag)
    {
        throw std::runtime_error("Cannot add read-only address.");
    }

    // Check if the address already exists
    for (auto it = this->m_FrozenAddresses.cbegin(); it != this->m_FrozenAddresses.cend(); it++)
    {
        if (it->memAddress.address == memAddress.address)
        {
            throw std::runtime_error("The address is already in the list.");
        }
    }

    // Add the address but have it disabled
    FrozenMemAddress frozenAddr = { memAddress, false, typeStr, dataStr, data, note };

    this->m_MemoryFreezerMutex.lock();

    this->m_FrozenAddresses.push_back(frozenAddr);

    this->m_MemoryFreezerMutex.unlock();
}

void MemoryFreezer::RemoveAddress(size_t index)
{
    if (index >= this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.cbegin();
        std::advance(iter, index);

        this->m_MemoryFreezerMutex.lock();

        // Make sure to decrement the enabled addresses, if the erased address was enabled
        if (iter->enabled)
        {
            this->m_EnabledAddressesAmount -= 1;
        }
        this->m_FrozenAddresses.erase(iter);

        // Prevents a possible crash where the thread might attempt to use a deleted iterator
        this->m_FrozenAddressIter = this->m_FrozenAddresses.begin();

        this->m_MemoryFreezerMutex.unlock();
    }
}

void MemoryFreezer::RemoveAllAddresses()
{
    this->m_MemoryFreezerMutex.lock();

    this->m_FrozenAddresses.clear();
    this->m_EnabledAddressesAmount = 0;

    this->m_MemoryFreezerMutex.unlock();
}

void MemoryFreezer::EnableAddress(size_t index)
{
    if (index >= this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.begin();
        std::advance(iter, index);

        if (!iter->enabled)
        {
            this->m_MemoryFreezerMutex.lock();

            iter->enabled = true;
            this->m_EnabledAddressesAmount += 1;

            this->m_MemoryFreezerMutex.unlock();

            // We may need to start/restart the thread if there were no addresses before
            // and a new address was added now
            this->StartThreadLoopIfNeeded();
        }
    }
}

void MemoryFreezer::DisableAddress(size_t index)
{
    if (index >= this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.begin();
        std::advance(iter, index);

        if (iter->enabled)
        {
            this->m_MemoryFreezerMutex.lock();

            this->m_EnabledAddressesAmount -= 1;
            iter->enabled = false;

            this->m_MemoryFreezerMutex.unlock();
        }
    }
}

void MemoryFreezer::EnableAllAddresses()
{
    for (auto it = this->m_FrozenAddresses.begin(); it != this->m_FrozenAddresses.end(); it++)
    {
        this->m_MemoryFreezerMutex.lock();

        // Only count (and enable) addresses which are currently disabled
        if (!it->enabled)
        {
            it->enabled = true;
            this->m_EnabledAddressesAmount += 1;
        }

        this->m_MemoryFreezerMutex.unlock();
    }
    this->StartThreadLoopIfNeeded();
}

void MemoryFreezer::DisableAllAddresses()
{
    this->m_MemoryFreezerMutex.lock();

    for (auto it = this->m_FrozenAddresses.begin(); it != this->m_FrozenAddresses.end(); it++)
    {
        it->enabled = false;
    }
    this->m_EnabledAddressesAmount = 0;

    this->m_MemoryFreezerMutex.unlock();
}

void MemoryFreezer::ModifyAddress(size_t index, const std::string& typeStr, const std::string& dataStr,
        std::vector<uint8_t>& data, const std::string& note)
{
    if (index >= this->m_FrozenAddresses.size())
    {
        throw std::runtime_error("Index out of bounds.");
    }
    else
    {
        auto iter = this->m_FrozenAddresses.begin();
        std::advance(iter, index);
        
        this->m_MemoryFreezerMutex.lock();

        iter->typeStr = typeStr;
        iter->dataStr = dataStr;
        iter->data = data;
        if (!note.empty()) // Only replace the note if it is not empty
        {
            iter->note = note;
        }

        this->m_MemoryFreezerMutex.unlock();
    }
}

void MemoryFreezer::ModifyAllAddresses(const std::string& typeStr, const std::string& dataStr,
        std::vector<uint8_t>& data, const std::string& note)
{
    for (size_t i = 0; i < this->m_FrozenAddresses.size(); i++)
    {
        this->ModifyAddress(i, typeStr, dataStr, data, note);
    }
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

    this->m_MemoryFreezerMutex.lock();

    this->m_FrozenAddresses.clear();
    this->m_EnabledAddressesAmount = 0;

    this->m_MemoryFreezerMutex.unlock();
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

void MemoryFreezer::ThreadLoop()
{
    this->m_FrozenAddressIter = this->m_FrozenAddresses.begin();
    while (true)
    {
        this->m_MemoryFreezerMutex.lock();

        // Stopping condition
        if (this->m_EnabledAddressesAmount <= 0)
        {
            this->m_MemoryFreezerMutex.unlock();
            break;
        }

        auto& it = this->m_FrozenAddressIter;
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
                    this->m_EnabledAddressesAmount -= 1;
                }
            }
            catch (const std::exception& e)
            {
                const std::string msg = fmt::format("Error writing to memory location {:#018x}: {}", 
                        it->memAddress.address, e.what());
                this->m_MessageQueue.push(msg);
                it->enabled = false; // Disable the address which caused an exception
                this->m_EnabledAddressesAmount -= 1;
            }
        }
        this->m_MemoryFreezerMutex.unlock();
        // Move forward
        std::advance(it, 1);
    }
    this->m_ThreadRunning = false;
}

size_t MemoryFreezer::GetMessageQueueSize() const
{
    return this->m_MessageQueue.size();
}

std::string MemoryFreezer::MessageQueuePop()
{
    std::string msg = this->m_MessageQueue.front();
    this->m_MessageQueue.pop();
    return msg;
}

