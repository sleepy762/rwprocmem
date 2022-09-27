#include "cmds/ScanCommand.h"
#include <cstdint>
#include <fmt/core.h>
#include <stdexcept>
#include "Utils.h"
#include "DataType.h"
#include "ComparisonType.h"
#include "cmds/WriteCommand.h"

template <typename T>
void CallScanner(Process& proc, size_t dataSize, const void* data, ComparisonType cmpType)
{
    MemoryScanner& memScanner = proc.GetMemoryScanner();

    // Calls the correct scan depending on if a new scan was started or not
    if (memScanner.GetScanStartedFlag())
    {
        memScanner.NextScan<T>(dataSize, data, cmpType);
    }
    else
    {
        memScanner.NewScan<T>(proc.GetMemoryRegions(), dataSize, data, cmpType);
    } 
}

template <typename T>
void ScanForData(Process& proc, const std::vector<std::string>& args, ComparisonType cmpType)
{
    constexpr size_t dataSize = sizeof(T);
    T dataValue = Utils::StrToNumber<T>(args[3]);

    CallScanner<T>(proc, dataSize, (void*)&dataValue, cmpType);
}

template <>
void ScanForData<std::string>(Process& proc, const std::vector<std::string>& args, ComparisonType cmpType)
{
    std::string data = Utils::JoinVectorOfStrings(args, 3, ' ');
    size_t dataSize = data.size();

    CallScanner<std::string>(proc, dataSize, (void*)data.c_str(), cmpType);
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

        // Update the address in the arguments into the current address in hex
        writeCmdArgs[1] = fmt::format("{:x}", it->address);       

        try
        {
            ICommand<WriteCommand>::Main(proc, writeCmdArgs);
            writeSuccess++;
        }
        catch (const std::runtime_error& e)
        {
            // Catching runtime errors only beacuse we want other exceptions to propogate out
            // namely std::invalid_argument
            fmt::print(stderr, "{:#018x}: {}\n", it->address, e.what());
        }
    }
    fmt::print("Written to {}/{} memory addresses.\n", writeSuccess, memAddrs.size());
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

        const std::string typeStr = args[2];
        switch (ParseDataType(typeStr))
        {
            case DataType::int8:   ScanForData<int8_t>(proc, args, cmpType);      break;
            case DataType::int16:  ScanForData<int16_t>(proc, args, cmpType);     break;
            case DataType::int32:  ScanForData<int32_t>(proc, args, cmpType);     break;
            case DataType::int64:  ScanForData<int64_t>(proc, args, cmpType);     break;
            case DataType::uint8:  ScanForData<uint8_t>(proc, args, cmpType);     break;
            case DataType::uint16: ScanForData<uint16_t>(proc, args, cmpType);    break;
            case DataType::uint32: ScanForData<uint32_t>(proc, args, cmpType);    break;
            case DataType::uint64: ScanForData<uint64_t>(proc, args, cmpType);    break;
            case DataType::f32:    ScanForData<float>(proc, args, cmpType);       break;
            case DataType::f64:    ScanForData<double>(proc, args, cmpType);      break;
            case DataType::string: ScanForData<std::string>(proc, args, cmpType); break;
        }
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
        "write -- Writes the <value> with the given <type> to all the saved memory addresses.\n");
}

