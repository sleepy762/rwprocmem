#include "cmds/WriteCommand.h"
#include "Utils.h"
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <charconv>
#include <concepts>

// Accepts int8, int16, int32, int64
template <std::integral T>
void WriteData(const pid_t pid, const unsigned long baseAddr, const std::vector<std::string>& args)
{
    const long dataSize = sizeof(T);
    
    T dataValue = 0;
    const char* dataString = args[3].c_str();

    std::from_chars_result res;
    // Help this function out
    // Use hex if the input is in hex
    if (dataString[0] == '0' && (dataString[1] == 'x' || dataString[1] == 'X'))
    {
        dataString += 2; // Move past the '0x'
        res = std::from_chars(dataString, dataString + args[3].size() - 2, dataValue, 16);
    }
    else // Use decimal base
    {
        res = std::from_chars(dataString, dataString + args[3].size(), dataValue);
    }

    // Error checking
    if (res.ec == std::errc::invalid_argument)
    {
        throw std::invalid_argument(args[0] + ": Invalid data.");
    }
    else if (res.ec == std::errc::result_out_of_range)
    {
        throw std::runtime_error(args[0] + ": Result of data conversion is out of range for the given type.");
    }

    ssize_t nread = Utils::WriteToProcessMemory(pid, baseAddr, dataSize, &dataValue);
    if (nread != dataSize)
    {
        std::cout << args[0] << ": WARNING: Partial write. Written " << nread << '/' << dataSize << " bytes.\n";
    }
}

// Template function specialized for floating point types because std::from_chars doesn't
// interpret hex numbers in the generic template function
// Accepts float, double
template <std::floating_point T>
void WriteData(const pid_t pid, const unsigned long baseAddr, const std::vector<std::string>& args)
{
    const long dataSize = sizeof(T);
    
    T dataValue = 0;
    const char* dataString = args[3].c_str();
    
    std::from_chars_result res;
    // It also needs help with floating point numbers
    if (dataString[0] == '0' && (dataString[1] == 'x' || dataString[1] == 'X'))
    {
        dataString += 2; // Move past the '0x'
        res = std::from_chars(dataString, dataString + args[3].size() - 2, dataValue, std::chars_format::hex);
    }
    else
    {
        res = std::from_chars(dataString, dataString + args[3].size(), dataValue);
    }

    // Error checking
    if (res.ec == std::errc::invalid_argument)
    {
        throw std::invalid_argument(args[0] + ": Invalid data.");
    }
    else if (res.ec == std::errc::result_out_of_range)
    {
        throw std::runtime_error(args[0] + ": Result of data conversion is out of range for the given type.");
    }

    ssize_t nread = Utils::WriteToProcessMemory(pid, baseAddr, dataSize, &dataValue);
    if (nread != dataSize)
    {
        std::cout << args[0] << ": WARNING: Partial write. Written " << nread << '/' << dataSize << " bytes.\n";
    }
}

// Accepts string
template <>
void WriteData<char>(const pid_t pid, const unsigned long baseAddr,
        const std::vector<std::string>& args)
{
    // Merge all the data strings into 1 string
    std::string fullData = "";
    for (auto it = args.cbegin() + 3; it != args.cend(); it++)
    {
        fullData += *it;
        // Add a space character in between the strings when joining them
        if (it + 1 != args.cend())
        {
            fullData += ' ';
        }
    }

    const long dataSize = fullData.size();
    ssize_t nread = Utils::WriteToProcessMemory(pid, baseAddr, dataSize, (void*)fullData.c_str());
    if (nread != dataSize)
    {
        std::cout << args[0] << ": WARNING: Partial write. Written " << nread << '/' << dataSize << " bytes.\n";
    }
}

void WriteCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 4)
    {
        throw std::runtime_error(args[0] + ": Missing arguments.");
    }

    unsigned long baseAddr;
    try
    {
        baseAddr = std::stoul(args[1], nullptr, 16);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(args[0] + ": Invalid address.");
    }

    const std::string typeStr = args[2];
    if (typeStr[0] == 'i')
    {
        if (typeStr == "int8")
        {
            WriteData<int8_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else if (typeStr == "int16")
        {
            WriteData<int16_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else if (typeStr == "int32")
        {
            WriteData<int32_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else if (typeStr == "int64")
        {
            WriteData<int64_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else
        {
            throw std::invalid_argument(args[0] + ": Invalid signed type.");
        }
    }
    else if (typeStr[0] == 'u')
    {
        if (typeStr == "uint8")
        {
            WriteData<uint8_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else if (typeStr == "uint16")
        {
            WriteData<uint16_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else if (typeStr == "uint32")
        {
            WriteData<uint32_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else if (typeStr == "uint64")
        {
            WriteData<uint64_t>(proc.GetCurrentPid(), baseAddr, args);
        }
        else
        {
            throw std::invalid_argument(args[0] + ": Invalid unsigned type.");
        }
    }
    else if (typeStr == "float")
    {
        WriteData<float>(proc.GetCurrentPid(), baseAddr, args);
    }
    else if (typeStr == "double")
    {
        WriteData<double>(proc.GetCurrentPid(), baseAddr, args);
    }
    else if (typeStr == "string")
    {
        WriteData<char>(proc.GetCurrentPid(), baseAddr, args);
    }
    else
    {
        throw std::invalid_argument(args[0] + ": Invalid type.");
    }
}

const char* WriteCommand::Help()
{
    return "Usage: write <address> <type> <data>\n"
        "Writes data to a given memory address.\n"
        "The <address> must be in hexadecimal.\n"
        "The <type> argument can be one of the following:\n"
        "[u]int8, [u]int16, [u]int32, [u]int64, float, double, string\n"
        "The 'u' prefix tells the program to use the unsigned type.\n"
        "Data for [u]int8, [u]int16, [u]int32, [u]int64 can be written as decimal numbers or hexadecimal numbers.\n"
        "Data for float and double can be written as floating point numbers or hexadecimal numbers.\n"
        "Data for string can only be a string.\n";
}

