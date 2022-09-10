#include "cmds/FindCommand.h"
#include "Process.h"
#include "Utils.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

template <typename T>
void FindData(const Process& proc, const std::vector<std::string>& args)
{
    constexpr unsigned long dataTypeSize = sizeof(T); 
    T dataValue = Utils::StrToNumber<T>(args[2].c_str(), args[2].size());

    const pid_t procPid = proc.GetCurrentPid();
    
    unsigned long matches = 0;
    const std::vector<mem_region_t> memRegions = proc.GetMemoryRegions();
    for (auto it = memRegions.cbegin(); it != memRegions.cend(); it++)
    {
        // Skip unreadable memory regions
        if (!it->perms.readFlag)
        {
            continue;
        }
        
        std::vector<uint8_t> regMemory;
        try
        {
            regMemory = Utils::ReadProcessMemory(procPid, it->startAddr, it->rangeLength);
        }
        catch (const std::exception& e) 
        {
            // Apparently some memory regions are marked as readable
            // but in reality are unreadable... ([vvar] for example)
            continue;
        }

        // The vector is a contiguous array in memory so we can do this
        const unsigned char* dataPtr = &regMemory[0];

        for (unsigned long i = 0; i < regMemory.size(); i++)
        {
            // We always want to have at least dataTypeSize bytes
            if (regMemory.size() - i < dataTypeSize)
            {
                break;
            }

            const unsigned char* offsetDataPtr = dataPtr + i;
            if (*(T*)offsetDataPtr == dataValue) // Check if the value is the same
            {
                matches++;
                std::cout << '[' << matches << "] 0x" << std::hex << it->startAddr + i << 
                    " (in " << it->pathName << ")\n";
            }
        }
    }
    std::cout << "Found " << matches << " matches.\n";
}

template <>
void FindData<char>(const Process& proc, const std::vector<std::string>& args)
{
    // Data starts at index 2
    const std::string fullString = Utils::JoinVectorOfStrings(args, 2, ' ');    
    const size_t fullStringLen = fullString.size();

    const pid_t procPid = proc.GetCurrentPid();

    unsigned long matches = 0;
    const std::vector<mem_region_t> memRegions = proc.GetMemoryRegions();
    for (auto it = memRegions.cbegin(); it != memRegions.cend(); it++)
    {
        // Skip unreadable memory regions
        if (!it->perms.readFlag)
        {
            continue;
        }
        
        std::vector<uint8_t> regMemory;
        try
        {
            regMemory = Utils::ReadProcessMemory(procPid, it->startAddr, it->rangeLength);
        }
        catch (const std::exception& e) 
        {
            // Apparently some memory regions are marked as readable
            // but in reality are unreadable... ([vvar] for example)
            continue;
        }

    }
}

void FindCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        throw std::runtime_error("Missing arguments.");
    }

    const std::string& typeStr = args[1]; 
    if (typeStr[0] == 'i')
    {
        if (typeStr == "int8")
        {
            FindData<int8_t>(proc, args);
        }
        else if (typeStr == "int16")
        {
            FindData<int16_t>(proc, args);
        }
        else if (typeStr == "int32")
        {
            FindData<int32_t>(proc, args);
        }
        else if (typeStr == "int64")
        {
            FindData<int64_t>(proc, args);
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
            FindData<uint8_t>(proc, args);
        }
        else if (typeStr == "uint16")
        {
            FindData<uint16_t>(proc, args);
        }
        else if (typeStr == "uint32")
        {
            FindData<uint32_t>(proc, args);
        }
        else if (typeStr == "uint64")
        {
            FindData<uint64_t>(proc, args);
        }
        else
        {
            throw std::runtime_error("Invalid unsigned type.");
        }
    }
    else if (typeStr == "float")
    {
        FindData<float>(proc, args);
    }
    else if (typeStr == "double")
    {
        FindData<double>(proc, args);
    }
    else if (typeStr == "string")
    {

    }
    else
    {
        throw std::runtime_error("Invalid type.");
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

