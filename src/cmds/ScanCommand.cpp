#include "cmds/ScanCommand.h"
#include <cstdint>
#include <fmt/core.h>
#include <stdexcept>
#include <string>
#include "Utils.h"
#include "DataType.h"
#include "ComparisonType.h"
#include "cmds/WriteCommand.h"
#include "cmds/FreezeCommand.h"

template <typename T>
size_t CallScanner(Process& proc, size_t dataSize, const void* data, ComparisonType cmpType)
{
    MemoryScanner& memScanner = proc.GetMemoryScanner();

    // Calls the correct scan depending on if a new scan was started or not
    if (memScanner.GetScanStartedFlag())
    {
        return memScanner.NextScan<T>(dataSize, data, cmpType);
    }
    else
    {
        return memScanner.NewScan<T>(proc.GetMemoryRegions(), dataSize, data, cmpType);
    } 
}

template <typename T>
size_t ScanForData(Process& proc, const std::string& dataStr, ComparisonType cmpType)
{
    constexpr size_t dataSize = sizeof(T);
    T dataValue = Utils::StrToNumber<T>(dataStr);

    return CallScanner<T>(proc, dataSize, (void*)&dataValue, cmpType);
}

template <>
size_t ScanForData<std::string>(Process& proc, const std::string& dataStr, ComparisonType cmpType)
{
    return CallScanner<std::string>(proc, dataStr.size(), (void*)dataStr.c_str(), cmpType);
}

static void ListSavedAddresses(const std::vector<MemAddress>& memAddrs)
{
    if (memAddrs.empty())
    {
        throw std::runtime_error("No memory addresses to list.");
    }
    else
    {
        Utils::PrintMemoryAddresses(memAddrs);
    }
}

static void WriteToSavedAddresses(Process& proc, const std::vector<std::string>& args)
{
    // This is the vector of arguments which will be passed to the write command
    // the first argument is always the command name
    // the second argument is the address (it will be updated inside the loop)
    // the third argument is the type of the data
    // the fourth argument is the data itself
    // Scan command syntax: scan write <type> <data>
    if (args.size() < 4)
    {
        throw std::runtime_error("Missing arguments.");
    }
    std::vector<std::string> writeCmdArgs = { "write", "", args[2], args[3] };

    auto memAddrs = proc.GetMemoryScanner().GetCurrScanVector();
    int writeSuccess = 0; // Tracks how many addresses were written to

    for (auto it = memAddrs.cbegin(); it != memAddrs.cend(); it++)
    {
        // Skip addresses to which data cannot be written
        if (!it->memRegion.perms.writeFlag)
        {
            continue;
        }

        writeCmdArgs[1] = std::to_string(it->address);       

        try
        {
            WriteCommand::Main(proc, writeCmdArgs);
            writeSuccess++;
        }
        catch (const std::exception& e)
        {
            fmt::print(stderr, "Error writing to address {:#018x}: {}\n", it->address, e.what());
        }
    }
    fmt::print("Written to {}/{} memory addresses.\n", writeSuccess, memAddrs.size());
}

static void AddScanListToFreezeList(Process& proc, const std::vector<std::string>& args)
{
    // The syntax of the freeze add command is:
    // freeze add <address> <type> <data> [note]
    // The synax of the scan freeze command is:
    // scan freeze <type> <data> [note]
    if (args.size() < 4)
    {
        throw std::runtime_error("Missing arguments.");
    }

    std::string note = "";
    try
    {
        note = args.at(4);
    }
    // No error handling since the argument is optional
    catch (const std::out_of_range&) {}

    // The third element is the address which will be modified inside the loop
    std::vector<std::string> freezeCmdArgs = { "freeze", "add", "", args[2], args[3], note }; 

    auto memAddrs = proc.GetMemoryScanner().GetCurrScanVector();
    int success = 0;

    for (auto it = memAddrs.cbegin(); it != memAddrs.cend(); it++)
    {
        // Skip addresses without the write flag enabled
        if (!it->memRegion.perms.writeFlag)
        {
            continue;
        }

        freezeCmdArgs[2] = std::to_string(it->address);

        try
        {
            FreezeCommand::Main(proc, freezeCmdArgs);
            success++;
        }
        catch (const std::exception& e)
        {
            fmt::print(stderr, "Error adding address {:#018x}: {}\n", it->address, e.what());
        }
    }
    fmt::print("Added {}/{} addresses to the freeze list.\n", success, memAddrs.size());
}

void ScanCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 2)
    {
        throw std::runtime_error("Missing keyword argument.");
    }

    const std::string& keywordStr = args[1];
    if (keywordStr == "clear")
    {
        proc.GetMemoryScanner().Clear();   
    }
    else if (keywordStr == "undo")
    {
        proc.GetMemoryScanner().Undo();
    }
    else if (keywordStr == "list")
    {
        ListSavedAddresses(proc.GetMemoryScanner().GetCurrScanVector());
    }
    else if (keywordStr == "write")
    {
        WriteToSavedAddresses(proc, args);
    }
    else if (keywordStr == "freeze")
    {
        AddScanListToFreezeList(proc, args);
    }
    else
    {
        // ParseComparisonType will throw if the keyword is incorrect or doesn't exist
        ComparisonType cmpType = ParseComparisonType(keywordStr);

        // Check if enough arguments were given
        // scan <keyword> <type> <value>
        if (args.size() < 4)
        {
            throw std::runtime_error("Missing arguments for scanning.");
        }

        size_t resAmount = 0;
        const std::string& typeStr = args[2];
        const std::string& dataStr = args[3];
        switch (ParseDataType(typeStr))
        {
            case DataType::int8:   resAmount = ScanForData<int8_t>(proc, dataStr, cmpType);      break;
            case DataType::int16:  resAmount = ScanForData<int16_t>(proc, dataStr, cmpType);     break;
            case DataType::int32:  resAmount = ScanForData<int32_t>(proc, dataStr, cmpType);     break;
            case DataType::int64:  resAmount = ScanForData<int64_t>(proc, dataStr, cmpType);     break;
            case DataType::uint8:  resAmount = ScanForData<uint8_t>(proc, dataStr, cmpType);     break;
            case DataType::uint16: resAmount = ScanForData<uint16_t>(proc, dataStr, cmpType);    break;
            case DataType::uint32: resAmount = ScanForData<uint32_t>(proc, dataStr, cmpType);    break;
            case DataType::uint64: resAmount = ScanForData<uint64_t>(proc, dataStr, cmpType);    break;
            case DataType::f32:    resAmount = ScanForData<float>(proc, dataStr, cmpType);       break;
            case DataType::f64:    resAmount = ScanForData<double>(proc, dataStr, cmpType);      break;
            case DataType::string: resAmount = ScanForData<std::string>(proc, dataStr, cmpType); break;
        }
        fmt::print("{} addresses found.\n", resAmount);
    }
}

std::string ScanCommand::Help()
{
    return std::string(
        "Usage: scan <keyword> [type] [value]\n\n"

        "Scans the memory of a process and keeps track of the addresses where the value was found.\n"
        "Subsequent scans check the values in the saved memory addresses.\n\n"

        "Keywords with no args required:\n"
        "clear -- Clears the saved addresses.\n"
        "undo -- Undo the last scan. (depth of 1 scan)\n"
        "list -- List the saved memory addresses.\n\n"

        "Keywords that require type and value:\n"
        "== -- Scans for addresses with a value equal to <value> in the given <type>.\n"
        "!= -- Scans for addresses which do not have <value>.\n"
        "> -- Scans for addresses where the value is greater than the given <value>.\n"
        "< -- Scans for addresses where the value is less than the given <value>.\n"
        ">= -- Scans for addresses where the value is greater or equal to <value>\n"
        "<= -- Scans for addresses where the value is less or equal to <value>.\n"
        "write -- Writes the <value> with the given <type> to all the saved memory addresses.\n"
        "freeze -- Adds all the writable addressses in the scan list to the freeze list.\n"
            "\tAn optional note can be added as well as another argument after <value>.\n");
}

