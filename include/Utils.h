#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>
#include <concepts>
#include <stdexcept>
#include <charconv>
#include "MemoryStructs.h"

namespace Utils
{
    std::vector<std::string> SplitString(const std::string& str, const char delim);
    std::string GetProcessCommand(const pid_t pid);

    std::vector<uint8_t> ReadProcessMemory(const pid_t pid, const unsigned long baseAddr,
            const long length);
    void WriteToProcessMemory(const pid_t pid, const unsigned long baseAddr, 
            const long dataSize, void* data);

    std::string JoinVectorOfStrings(const std::vector<std::string>& vec, const int startIndex, 
            const char joinChar);

    std::vector<MemAddress> FindDataInMemory(const pid_t pid, 
            const std::vector<MemRegion>& memRegions, const size_t dataSize, const void* dataToFind);


    // std::from_chars takes different arguments depending on if the type is integral or float
    template <std::integral T>
    T StrToNumber(const std::string& dataString)
    {
        T dataValue = 0;
        std::from_chars_result res;

        const char* dataStringStart = dataString.c_str();
        const char* dataStringEnd = dataStringStart + dataString.size();

        // Use hex if the input is in hex
        if (dataStringStart[0] == '0' && (dataStringStart[1] == 'x' || dataStringStart[1] == 'X'))
        {
            dataStringStart += 2; // Move past the '0x'
            res = std::from_chars(dataStringStart, dataStringEnd, dataValue, 16);
        }
        else // Use generic conversion
        {
            res = std::from_chars(dataStringStart, dataStringEnd, dataValue);
        }

        // Error checking
        if (res.ec == std::errc::invalid_argument)
        {
            throw std::invalid_argument("Invalid data.");
        }
        else if (res.ec == std::errc::result_out_of_range)
        {
            throw std::runtime_error("Result of data conversion is out of range for the given type.");
        }
        else if (res.ptr != dataStringEnd)
        {
            throw std::runtime_error("Failed to fully convert the given data to a number.");
        }
        return dataValue;
    }

    template <std::floating_point T>
    T StrToNumber(const std::string& dataString)
    {
        T dataValue = 0;
        std::from_chars_result res;

        const char* dataStringStart = dataString.c_str();
        const char* dataStringEnd = dataStringStart + dataString.size();

        // It also needs help with floating point numbers
        if (dataStringStart[0] == '0' && (dataStringStart[1] == 'x' || dataStringStart[1] == 'X'))
        {
            dataStringStart += 2; // Move past the '0x'
            res = std::from_chars(dataStringStart, dataStringEnd, dataValue, std::chars_format::hex);
        }
        else
        {
            res = std::from_chars(dataStringStart, dataStringEnd, dataValue);
        }

        // Error checking
        if (res.ec == std::errc::invalid_argument)
        {
            throw std::invalid_argument("Invalid data.");
        }
        else if (res.ec == std::errc::result_out_of_range)
        {
            throw std::runtime_error("Result of data conversion is out of range for the given type.");
        }
        else if (res.ptr != dataStringEnd)
        {
            throw std::runtime_error("Failed to fully convert the given data to a number.");
        }
        return dataValue;
    }
}

