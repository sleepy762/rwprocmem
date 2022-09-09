#include "cmds/DumpCommand.h"
#include "Utils.h"
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cctype>

void DumpCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    if (args.size() < 3)
    {
        throw std::runtime_error(args[0] + ": Missing arguments.");
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
        throw std::invalid_argument(args[0] + ": Invalid address.");
    }

    try
    {
        length = std::stoul(args[2]);
    }
    catch (const std::exception& e)
    {
        throw std::invalid_argument(args[0] + ": Invalid length.");
    }

    const std::vector<uint8_t> dataVec = Utils::ReadProcessMemory(proc.GetCurrentPid(), baseAddr, length);
    constexpr int BYTES_PER_LINE = 16;

    for (size_t i = 0; i < length; i += BYTES_PER_LINE)
    {
        std::string printableData = ""; // Holds printable ASCII characters 
        // Output the memory address offset (memory address is 16 characters long)
        std::cout << "0x" << std::hex << baseAddr + i << ": ";

        for (int j = 0; j < BYTES_PER_LINE; j++)
        {
            // Avoid going out of bounds
            if (i + j >= length)
            {
                // Add padding if the last line is shorter
                for (int k = 0; k < BYTES_PER_LINE - j; k++)
                {
                    // Pad with 3 spaces because there is a space inbetween every hex byte
                    // and every hex byte takes 2 characters
                    std::cout << "   ";
                }
                break;
            }
            int byte = dataVec[i + j]; // Read a byte from the vector
            std::cout << std::setfill('0') << std::setw(2) << byte << ' ';

            // Save ASCII character, if it's printable
            printableData += std::isprint(byte) ? byte : '.';
        }
        std::cout << std::dec << '|' << printableData << "|\n";
    }
}

const char* DumpCommand::Help()
{
    return "Usage: dump <address> <length>\n"
        "Ouputs a hex dump with the given length of the data in the given address.\n"
        "The memory address has to be in hexadecimal.\n";
}

