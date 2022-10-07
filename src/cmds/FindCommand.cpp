#include "cmds/FindCommand.h"
#include "ComparisonType.h"
#include "Process.h"
#include "Utils.h"
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <fmt/core.h>
#include "DataType.h"
#include "MemoryFuncs.h"

template <typename T>
std::vector<MemAddress> FindData(const Process& proc, const std::string& dataStr)
{
    constexpr unsigned long dataTypeSize = sizeof(T); 
    T dataValue = Utils::StrToNumber<T>(dataStr);
    
    return MemoryFuncs::FindDataInMemory<T>(proc.GetCurrentPid(), proc.GetMemoryRegions(), 
            dataTypeSize, &dataValue, ComparisonType::Equal);
}

template <>
std::vector<MemAddress> FindData<std::string>(const Process& proc, const std::string& dataStr)
{
    return MemoryFuncs::FindDataInMemory<std::string>(proc.GetCurrentPid(), proc.GetMemoryRegions(),
            dataStr.size(), dataStr.c_str(), ComparisonType::Equal);
}

void FindCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        throw std::runtime_error("Missing arguments.");
    }

    std::vector<MemAddress> foundAddrs;
    const std::string& typeStr = args[1]; 
    const std::string& dataStr = args[2];

    switch (ParseDataType(typeStr))
    {
        case DataType::int8:    foundAddrs = FindData<int8_t>(proc, dataStr);      break;
        case DataType::int16:   foundAddrs = FindData<int16_t>(proc, dataStr);     break;
        case DataType::int32:   foundAddrs = FindData<int32_t>(proc, dataStr);     break;
        case DataType::int64:   foundAddrs = FindData<int64_t>(proc, dataStr);     break;
        case DataType::uint8:   foundAddrs = FindData<uint8_t>(proc, dataStr);     break;
        case DataType::uint16:  foundAddrs = FindData<uint16_t>(proc, dataStr);    break;
        case DataType::uint32:  foundAddrs = FindData<uint32_t>(proc, dataStr);    break;
        case DataType::uint64:  foundAddrs = FindData<uint64_t>(proc, dataStr);    break;
        case DataType::f32:     foundAddrs = FindData<float>(proc, dataStr);       break;
        case DataType::f64:     foundAddrs = FindData<double>(proc, dataStr);      break;
        case DataType::string:  foundAddrs = FindData<std::string>(proc, dataStr); break;
        // No default: so that the compiler can generate a warning for us in case we forget something.
    }

    Utils::PrintMemoryAddresses(foundAddrs);
}
std::string FindCommand::Help()
{
    return std::string(
        "Usage: find <type> <data>\n\n"

        "Lists the memory addresses where the given data was found.\n\n"

        "The <type> argument can be one of the following:\n"
        "[u]int8, [u]int16, [u]int32, [u]int64, float, double, string\n"
        "The 'u' prefix tells the program to use the unsigned type.\n\n"

        "Data for [u]int8, [u]int16, [u]int32, [u]int64 can be written as decimal numbers or hexadecimal numbers.\n"
        "Data for float and double can be written as floating point numbers or hexadecimal numbers.\n"
        "Data for string can only be a string.\n");
}

