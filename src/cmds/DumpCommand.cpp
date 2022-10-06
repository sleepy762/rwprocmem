#include "cmds/DumpCommand.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <cctype>
#include <fmt/core.h>
#include "MemoryFuncs.h"

void DumpCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        throw std::runtime_error("Missing arguments.");
    }

    unsigned long baseAddr;
    unsigned long length;

    try
    {
        // The address is expected to be passed in hexadecimal
        baseAddr = std::stoul(args[1], nullptr, 16);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument("Invalid address.");
    }

    try
    {
        length = std::stoul(args[2]);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument("Invalid length.");
    }

    const std::vector<uint8_t> dataVec = MemoryFuncs::ReadProcessMemory(proc.GetCurrentPid(), baseAddr, length);
    const size_t dataLen = dataVec.size();
    constexpr int BYTES_PER_LINE = 16;

    for (size_t i = 0; i < dataLen; i += BYTES_PER_LINE)
    {
        std::string printableData = ""; // Holds printable ASCII characters 
        // Output the memory address offset (memory address is 16 characters long)
        fmt::print("{:#018x}: ", baseAddr + i);

        for (int j = 0; j < BYTES_PER_LINE; j++)
        {
            // Avoid going out of bounds
            if (i + j >= dataLen)
            {
                // Add padding if the last line is shorter
                for (int k = 0; k < BYTES_PER_LINE - j; k++)
                {
                    // Pad with 3 spaces because there is a space inbetween every hex byte
                    // and every hex byte takes 2 characters
                    fmt::print("   ");
                }
                break;
            }
            int byte = dataVec[i + j]; // Read a byte from the vector
            fmt::print("{:02x} ", byte);

            // Save ASCII character, if it's printable
            printableData += std::isprint(byte) ? byte : '.';
        }
        fmt::print("|{}|\n", printableData);
    }
    // Notify the user if the dump is partial
    if (dataLen != length)
    {
        fmt::print("WARNING: Partial read of {}/{} bytes at address {:#018x}.\n", dataLen, length, baseAddr);
    }
}

std::string DumpCommand::Help()
{
    return std::string(
        "Usage: dump <address> <length>\n\n"

        "Ouputs a hex dump with the given length of the data in the given address.\n"
        "The memory address has to be in hexadecimal.\n");
}

