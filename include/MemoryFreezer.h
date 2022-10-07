#pragma once
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>
#include <list>
#include "MemoryStructs.h"
#include <mutex>
#include <queue>

struct FrozenMemAddress
{
    MemAddress memAddress;
    bool enabled; // A flag which tells the program whether the address should be frozen
    std::string typeStr; // Only used for printing to the user
    std::string dataStr; // Also only used for printing to the user (removes the need for a template)
    std::vector<uint8_t> data; // The data which will be continually written to the address
    std::string note; // An optional user note about the saved memory address
};

class MemoryFreezer
{
public:
    MemoryFreezer();
    ~MemoryFreezer();

    void AddAddress(MemAddress memAddress, const std::string& typeStr, const std::string& dataStr,
            std::vector<uint8_t>& data, const std::string& note);
    void RemoveAddress(size_t index);
    void RemoveAllAddresses();

    void EnableAddress(size_t index);
    void DisableAddress(size_t index);
    void EnableAllAddresses();
    void DisableAllAddresses();

    void ModifyAddress(size_t index, const std::string& typeStr, const std::string& dataStr,
            std::vector<uint8_t>& data, const std::string& note);

    const std::list<FrozenMemAddress>& GetFrozenAddresses() const;
    int GetEnabledAddressesAmount() const;

    void SetPid(pid_t pid);

    std::string MessageQueuePop();
    size_t GetMessageQueueSize() const;

private:
    void StartThreadLoopIfNeeded();
    void ThreadLoop();

    int m_EnabledAddressesAmount;
    bool m_ThreadRunning;

    pid_t m_pid;
    std::list<FrozenMemAddress> m_FrozenAddresses;
    std::list<FrozenMemAddress>::iterator m_FrozenAddressIter;
    std::mutex m_MemoryFreezerMutex;

    std::queue<std::string> m_MessageQueue; // A queue for messages from the memory freezer thread
};

