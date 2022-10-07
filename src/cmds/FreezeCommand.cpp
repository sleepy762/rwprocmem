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
        fmt::print("No memory addresses to list.\n");
        return;
    }

    const size_t indexWidth = std::to_string(frozenAddrs.size()).size();
    int index = 0;
    for (auto it = frozenAddrs.cbegin(); it != frozenAddrs.cend(); it++)
    {
        fmt::print("[{:{}}][{}] {:#018x} (in {}) [{}: {}]",
            index, indexWidth,
            it->enabled ? 'X' : ' ', // If the address is enabled, mark it with an X
            it->memAddress.address,
            it->memAddress.memRegion.pathName,
            it->typeStr,
            it->dataStr);
            
        if (!it->note.empty())
        {
            fmt::print(" Note: \"{}\"", it->note);
        }

        fmt::print("\n");
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
            index = Utils::StrToNumber<size_t>(indexStr, "index");
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
    else if (keywordStr == "add" || keywordStr == "modify")
    {
        if (args.size() < 5)
        {
            throw std::runtime_error("Missing arguments.");
        }

        std::string note = "";
        try
        {
            note = args.at(5);
        }
        // The error can be ignored since the note argument is optional
        catch (const std::out_of_range&) {}

        const std::string& typeStr = args[3];
        const std::string& dataStr = args[4];
        std::vector<uint8_t> byteVector;

        switch (ParseDataType(typeStr))
        {
            case DataType::int8:   byteVector = DataToByteVector<int8_t>(dataStr);      break;
            case DataType::int16:  byteVector = DataToByteVector<int16_t>(dataStr);     break;
            case DataType::int32:  byteVector = DataToByteVector<int32_t>(dataStr);     break;
            case DataType::int64:  byteVector = DataToByteVector<int64_t>(dataStr);     break;
            case DataType::uint8:  byteVector = DataToByteVector<uint8_t>(dataStr);     break;
            case DataType::uint16: byteVector = DataToByteVector<uint16_t>(dataStr);    break;
            case DataType::uint32: byteVector = DataToByteVector<uint32_t>(dataStr);    break;
            case DataType::uint64: byteVector = DataToByteVector<uint64_t>(dataStr);    break;
            case DataType::f32:    byteVector = DataToByteVector<float>(dataStr);       break;
            case DataType::f64:    byteVector = DataToByteVector<double>(dataStr);      break;
            case DataType::string: byteVector = DataToByteVector<std::string>(dataStr); break;
        }

        if (keywordStr == "add")
        {
            unsigned long address = Utils::StrToNumber<unsigned long>(args[2], "address");
            MemRegion memRegion = Utils::FindRegionOfAddress(proc.GetMemoryRegions(), address);
            MemAddress memAddress = { address, memRegion };

            memFreezer.AddAddress(memAddress, typeStr, dataStr, byteVector, note);
        }
        else if (keywordStr == "modify")
        {
            size_t index = Utils::StrToNumber<size_t>(args[2], "index");

            memFreezer.ModifyAddress(index, typeStr, dataStr, byteVector, note);
        }
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
            "\t[index][enabled/disabled] [address] [pathname] [type] [data]\n\n"

        "Keywords that require 1 argument:\n"
        "remove <index/all> -- Removes the address in the given index, or removes all addresses.\n"
        "enable <index/all> -- Enables the program to freeze the address in the given index, or all addresses.\n"
        "disable <index/all> -- Disables the program from freezing the address in the given index, or all addresses.\n\n"

        "Keywords that require 3 arguments:\n"
        "add <address> <type> <data> [note] -- Adds the address to the freezing list which will write <data> to <address> continuously.\n"
            "\tWhen addresses are added, they are disabled and have to be enabled manually.\n"
            "\tA note is an optional string that will appear next to the address in the freeze list.\n"
        "modify <index> <type> <data> [note] -- Modifies an existing address by changing the <type> and <data>.\n"
            "\tPassing a new note will overwrite the old note, otherwise the old note will stay.\n");
}

