#include "cmds/FindCommand.h"
#include "Process.h"
#include "Utils.h"
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <fmt/core.h>
#include "DataType.h"

template <typename T>
std::vector<MemAddress> FindData(const Process& proc, const std::vector<std::string>& args)
{
    constexpr unsigned long dataTypeSize = sizeof(T); 
    T dataValue = Utils::StrToNumber<T>(args[2]);
    
    return Utils::FindDataInMemory(proc.GetCurrentPid(), proc.GetMemoryRegions(), 
            dataTypeSize, &dataValue);
}

template <>
std::vector<MemAddress> FindData<std::string>(const Process& proc, const std::vector<std::string>& args)
{
    // Data starts at index 2
    const std::string fullString = Utils::JoinVectorOfStrings(args, 2, ' ');    
    const size_t fullStringLen = fullString.size();

    return Utils::FindDataInMemory(proc.GetCurrentPid(), proc.GetMemoryRegions(),
            fullStringLen, fullString.c_str());
}

void FindCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        throw std::runtime_error("Missing arguments.");
    }

    std::vector<MemAddress> foundAddrs;
    const std::string& typeStr = args[1]; 

    switch (ParseDataType(typeStr))
    {
        case DataType::int8:    foundAddrs = FindData<int8_t>(proc, args);      break;
        case DataType::int16:   foundAddrs = FindData<int16_t>(proc, args);     break;
        case DataType::int32:   foundAddrs = FindData<int32_t>(proc, args);     break;
        case DataType::int64:   foundAddrs = FindData<int64_t>(proc, args);     break;
        case DataType::uint8:   foundAddrs = FindData<uint8_t>(proc, args);     break;
        case DataType::uint16:  foundAddrs = FindData<uint16_t>(proc, args);    break;
        case DataType::uint32:  foundAddrs = FindData<uint32_t>(proc, args);    break;
        case DataType::uint64:  foundAddrs = FindData<uint64_t>(proc, args);    break;
        case DataType::f32:     foundAddrs = FindData<float>(proc, args);       break;
        case DataType::f64:     foundAddrs = FindData<double>(proc, args);      break;
        case DataType::string:  foundAddrs = FindData<std::string>(proc, args); break;
        // No default: so that the compiler can generate a warning for us in case we forget something.
    }

    // Gets the amount of digits in the number of found addresses
    const size_t indexWidth = std::to_string(foundAddrs.size()).size();

    for (auto it = foundAddrs.cbegin(); it != foundAddrs.cend(); it++)
    {
        int index = it - foundAddrs.cbegin();
        fmt::print("[{:{}}] {:#018x} [{}] (in {})\n",
                index, indexWidth, it->address, it->memRegion.permsStr, it->memRegion.pathName);
    }
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

