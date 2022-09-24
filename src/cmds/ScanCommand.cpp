#include "cmds/ScanCommand.h"
#include <cstdint>
#include <stdexcept>
#include "Utils.h"
#include "DataType.h"
#include "ComparisonType.h"

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
        auto memAddrs = proc.GetMemoryScanner().GetCurrScanVector();
        if (memAddrs.empty())
        {
            throw std::runtime_error("No memory addresses to list.");
        }
        else
        {
            Utils::PrintMemoryAddresses(memAddrs);
        }
    }
    else
    {
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
        "Usage: scan <keyword> [type] [arg]\n\n"

        "Scans the memory of a process and keeps track of the addresses where the value was found.\n"
        "Subsequent scans check the values in the saved memory addresses.\n\n"
        "Keywords with no args required:\n"
        "clear -- Clears the saved addresses.\n"
        "undo -- Undo the last scan. (depth of 1 scan)\n"
        "list -- List the saved memory addresses.\n\n"

        "Keywords that require type and arg:\n"
        "== -- Scans for an exact <value> of the given <type>.\n"
        "!= -- Scans for addresses which do not have <value>.\n"
        "> -- Scans for addresses where the value is greater than the given <value>.\n"
        "< -- Scans for addresses where the value is less than the given <value>.\n"
        ">= -- Scans for addresses where the value is greater or equal to <value>\n"
        "<= -- Scans for addresses where the value is less or equal to <value>.\n");
}

