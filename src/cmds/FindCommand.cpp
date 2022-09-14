#include "cmds/FindCommand.h"
#include "Process.h"
#include "Utils.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>

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
    if (typeStr[0] == 'i')
    {
        if (typeStr == "int8")
        {
            foundAddrs = FindData<int8_t>(proc, args);
        }
        else if (typeStr == "int16")
        {
            foundAddrs = FindData<int16_t>(proc, args);
        }
        else if (typeStr == "int32")
        {
            foundAddrs = FindData<int32_t>(proc, args);
        }
        else if (typeStr == "int64")
        {
            foundAddrs = FindData<int64_t>(proc, args);
        }
        else
        {
            throw std::runtime_error("Invalid signed type.");
        }
    }
    else if (typeStr[0] == 'u')
    {
        if (typeStr == "uint8")
        {
            foundAddrs = FindData<uint8_t>(proc, args);
        }
        else if (typeStr == "uint16")
        {
            foundAddrs = FindData<uint16_t>(proc, args);
        }
        else if (typeStr == "uint32")
        {
            foundAddrs = FindData<uint32_t>(proc, args);
        }
        else if (typeStr == "uint64")
        {
            foundAddrs = FindData<uint64_t>(proc, args);
        }
        else
        {
            throw std::runtime_error("Invalid unsigned type.");
        }
    }
    else if (typeStr == "float")
    {
        foundAddrs = FindData<float>(proc, args);
    }
    else if (typeStr == "double")
    {
        foundAddrs = FindData<double>(proc, args);
    }
    else if (typeStr == "string")
    {
        foundAddrs = FindData<std::string>(proc, args);
    }
    else
    {
        throw std::runtime_error("Invalid type.");
    }

    for (auto it = foundAddrs.cbegin(); it != foundAddrs.cend(); it++)
    {
        int index = it - foundAddrs.cbegin();
        std::cout << '[' << index << "] 0x" << std::hex << it->address << 
            " [" << it->memRegion.permsStr << std::dec << "] (in " << it->memRegion.pathName << ")\n";
    }
}
const char* FindCommand::Help()
{
    return "Usage: find <type> <data>\n\n"
        "Lists the memory addresses where the given data was found.\n\n"
        "The <type> argument can be one of the following:\n"
        "[u]int8, [u]int16, [u]int32, [u]int64, float, double, string\n"
        "The 'u' prefix tells the program to use the unsigned type.\n\n"
        "Data for [u]int8, [u]int16, [u]int32, [u]int64 can be written as decimal numbers or hexadecimal numbers.\n"
        "Data for float and double can be written as floating point numbers or hexadecimal numbers.\n"
        "Data for string can only be a string.\n";
}

