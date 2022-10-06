#include "cmds/FreezeCommand.h"
#include "MemoryFreezer.h"
#include <fmt/core.h>
#include <stdexcept>
#include "Utils.h"
#include "DataType.h"

template <typename T>
std::vector<uint8_t> DataToByteVector(const std::string& data)
{
    constexpr size_t dataSize = sizeof(T);
    // Convert the data to the correct type (for the correct binary representation)
    T dataValue = Utils::StrToNumber<T>(data);

    // Get a pointer to the dataValue in memory
    uint8_t* dataValueBytePtr = (uint8_t*)&dataValue;

    std::vector<uint8_t> byteVector;
    // Construct the vector of bytes
    byteVector.insert(byteVector.end(), &dataValueBytePtr[0], &dataValueBytePtr[dataSize]);

    return byteVector;
}

template <>
std::vector<uint8_t> DataToByteVector<std::string>(const std::string& data)
{
    return std::vector<uint8_t>(data.begin(), data.end());
}

static void ListFrozenMemoryAddresses(const std::list<FrozenMemAddress>& frozenAddrs)
{
    if (frozenAddrs.size() == 0)
    {
        fmt::print("No memory addresses to list.");
        return;
    }

    const size_t indexWidth = std::to_string(frozenAddrs.size()).size();
    int index = 0;
    for (auto it = frozenAddrs.cbegin(); it != frozenAddrs.cend(); it++)
    {
        fmt::print("[{:{}}] [{}] {:#018x} (in {}) [{}: {}]\n",
            index, indexWidth,
            it->enabled ? 'X' : ' ', // If the address is enabled, mark it with an X
            it->memAddress.address,
            it->memAddress.memRegion.pathName,
            it->typeStr,
            it->dataStr);
        index++;
    }
}

void FreezeCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        throw std::runtime_error("Missing keyword argument.");
    }

    MemoryFreezer& memFreezer = proc.GetMemoryFreezer();

    const std::string& keywordStr = args[1];
    if (keywordStr == "list")
    {
        const std::list<FrozenMemAddress>& frozenAddrs = memFreezer.GetFrozenAddresses();
        ListFrozenMemoryAddresses(frozenAddrs);
    }
    // Keywords that require 1 arg
    else if (keywordStr == "remove" || keywordStr == "enable" || keywordStr == "disable")
    {
        if (args.size() < 3)
        {
            throw std::runtime_error("Missing arguments.");
        }

        bool allFlag = false;
        size_t index = -1; // Sets the maximum value
        const std::string& indexStr = args[2];
        if (indexStr == "all")
        {
            allFlag = true;
        }
        else
        {
            index = Utils::StrToNumber<size_t>(indexStr);
        }

        if (keywordStr == "remove")
        {
            if (allFlag)
            {
                memFreezer.RemoveAllAddresses();
            }
            else
            {
                memFreezer.RemoveAddress(index);
            }
        }
        else if (keywordStr == "enable")
        {
            if (allFlag)
            {
                memFreezer.EnableAllAddresses();
            }
            else
            {
                memFreezer.EnableAddress(index);
            }
        }
        else if (keywordStr == "disable")
        {
            if (allFlag)
            {
                memFreezer.DisableAllAddresses();
            }
            else
            {
                memFreezer.DisableAddress(index);
            }
        }
    }
    // This keyword requires 3 args
    else if (keywordStr == "add")
    {
        if (args.size() < 5)
        {
            throw std::runtime_error("Missing arguments.");
        }

        unsigned long address = Utils::StrToNumber<unsigned long>(args[2]);
        MemRegion memRegion = Utils::FindRegionOfAddress(proc.GetMemoryRegions(), address);
        const std::string& typeStr = args[3];
        const std::string& dataStr = args[4];
        std::vector<uint8_t> dataVector;

        switch (ParseDataType(typeStr))
        {
            case DataType::int8:   dataVector = DataToByteVector<int8_t>(dataStr);      break;
            case DataType::int16:  dataVector = DataToByteVector<int16_t>(dataStr);     break;
            case DataType::int32:  dataVector = DataToByteVector<int32_t>(dataStr);     break;
            case DataType::int64:  dataVector = DataToByteVector<int64_t>(dataStr);     break;
            case DataType::uint8:  dataVector = DataToByteVector<uint8_t>(dataStr);     break;
            case DataType::uint16: dataVector = DataToByteVector<uint16_t>(dataStr);    break;
            case DataType::uint32: dataVector = DataToByteVector<uint32_t>(dataStr);    break;
            case DataType::uint64: dataVector = DataToByteVector<uint64_t>(dataStr);    break;
            case DataType::f32:    dataVector = DataToByteVector<float>(dataStr);       break;
            case DataType::f64:    dataVector = DataToByteVector<double>(dataStr);      break;
            case DataType::string: dataVector = DataToByteVector<std::string>(dataStr); break;
        }
        MemAddress memAddress = { address, memRegion };

        memFreezer.AddAddress(memAddress, typeStr, dataStr, dataVector);
    }
    else
    {
        throw std::runtime_error("Invalid keyword.");
    }
}

std::string FreezeCommand::Help()
{
    return std::string(
        "Usage: freeze <keyword> [args...]\n\n"

        "Continuously overwrites values in memory addresses, effectively \"freezing\" them.\n\n"

        "Keywords with no args required:\n"
        "list -- Lists the frozen memory addresses in the following format:\n"
            "\t[index] [enabled/disabled] [address] [pathname] [type] [data]\n\n"

        "Keywords that require 1 argument:\n"
        "remove <index/all> -- Removes the address in the given index, or removes all addresses.\n"
        "enable <index/all> -- Enables the program to freeze the address in the given index, or all addresses.\n"
        "disable <index/all> -- Disables the program from freezing the address in the given index, or all addresses.\n"
            
        "Keywords that require 3 arguments:\n"
        "add <address> <type> <data> -- Adds the address to the freezing list which will write <data> to <address> continuously."
            " When addresses are added, they are disabled and have to be enabled manually.\n");
}

