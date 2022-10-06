#include "cmds/WriteCommand.h"
#include "Utils.h"
#include <exception>
#include <stdexcept>
#include <sys/types.h>
#include "DataType.h"
#include "MemoryFuncs.h"

template <typename T>
void WriteData(pid_t pid, unsigned long baseAddr, const std::vector<std::string>& args)
{
    // The data is in index 3, according to the syntax
    constexpr long dataTypeSize = sizeof(T);
    T dataValue = Utils::StrToNumber<T>(args[3]);

    ssize_t nread = MemoryFuncs::WriteToProcessMemory(pid, baseAddr, dataTypeSize, &dataValue);
    if (nread != dataTypeSize)
    {
        fmt::print("WARNING: Partial write of {}/{} bytes at address {:#018x}.\n", nread, dataTypeSize, baseAddr);
    }
}

// Accepts string
template <>
void WriteData<std::string>(pid_t pid, unsigned long baseAddr, const std::vector<std::string>& args)
{
    // Data starts at index 3
    std::string fullData = Utils::JoinVectorOfStrings(args, 3, ' ');
    const long dataSize = fullData.size();

    ssize_t nread = MemoryFuncs::WriteToProcessMemory(pid, baseAddr, dataSize, (void*)fullData.c_str());
    if (nread != dataSize)
    {
        fmt::print("WARNING: Partial write of {}/{} bytes at address {:#018x}.\n", nread, dataSize, baseAddr);
    }
}

void WriteCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 4)
    {
        throw std::runtime_error("Missing arguments.");
    }

    // The address is in index 1
    unsigned long baseAddr = Utils::StrToNumber<unsigned long>(args[1], "address");

    const pid_t pid = proc.GetCurrentPid();
    const std::string& typeStr = args[2];
    switch (ParseDataType(typeStr))
    {
        case DataType::int8:   WriteData<int8_t>(pid, baseAddr, args);      break;
        case DataType::int16:  WriteData<int16_t>(pid, baseAddr, args);     break;
        case DataType::int32:  WriteData<int32_t>(pid, baseAddr, args);     break;
        case DataType::int64:  WriteData<int64_t>(pid, baseAddr, args);     break;
        case DataType::uint8:  WriteData<uint8_t>(pid, baseAddr, args);     break;
        case DataType::uint16: WriteData<uint16_t>(pid, baseAddr, args);    break;
        case DataType::uint32: WriteData<uint32_t>(pid, baseAddr, args);    break;
        case DataType::uint64: WriteData<uint64_t>(pid, baseAddr, args);    break;
        case DataType::f32:    WriteData<float>(pid, baseAddr, args);       break;
        case DataType::f64:    WriteData<double>(pid, baseAddr, args);      break;
        case DataType::string: WriteData<std::string>(pid, baseAddr, args); break;
        // No default: so that the compiler can generate a warning for us in case we forget something.
    }
}

std::string WriteCommand::Help()
{
    return std::string(
        "Usage: write <address> <type> <data>\n\n"

        "Writes data to a given memory address.\n\n"

        "The <address> must be in hexadecimal.\n\n"

        "The <type> argument can be one of the following:\n"
        "[u]int8, [u]int16, [u]int32, [u]int64, float, double, string\n"
        "The 'u' prefix tells the program to use the unsigned type.\n\n"

        "Data for [u]int8, [u]int16, [u]int32, [u]int64 can be written as decimal numbers or hexadecimal numbers.\n"
        "Data for float and double can be written as floating point numbers or hexadecimal numbers.\n"
        "Data for string can only be a string.\n");
}
