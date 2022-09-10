#include "cmds/WriteCommand.h"
#include "Utils.h"
#include <cstdint>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>

template <typename T>
void WriteData(const pid_t pid, const unsigned long baseAddr, const std::vector<std::string>& args)
{
    constexpr long dataSize = sizeof(T);
    T dataValue = Utils::StrToNumber<T>(args[3].c_str(), args[3].size());

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
        throw std::runtime_error("Missing arguments.");
    }

    unsigned long baseAddr;
    try
    {
        baseAddr = std::stoul(args[1], nullptr, 16);
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Invalid address.");
    }

    const std::string& typeStr = args[2];
    if (typeStr[0] == 'i') // Signed integers
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
            throw std::invalid_argument("Invalid signed type.");
        }
    }
    else if (typeStr[0] == 'u') // Unsigned integers
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
            throw std::invalid_argument("Invalid unsigned type.");
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
        throw std::invalid_argument("Invalid type.");
    }
}

const char* WriteCommand::Help()
{
    return "Usage: write <address> <type> <data>\n\n"
        "Writes data to a given memory address.\n\n"
        "The <address> must be in hexadecimal.\n\n"
        "The <type> argument can be one of the following:\n"
        "[u]int8, [u]int16, [u]int32, [u]int64, float, double, string\n"
        "The 'u' prefix tells the program to use the unsigned type.\n\n"
        "Data for [u]int8, [u]int16, [u]int32, [u]int64 can be written as decimal numbers or hexadecimal numbers.\n"
        "Data for float and double can be written as floating point numbers or hexadecimal numbers.\n"
        "Data for string can only be a string.\n";
}

