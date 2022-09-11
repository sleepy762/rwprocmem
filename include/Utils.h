#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <sys/types.h>
#include <concepts>
#include <stdexcept>
#include <charconv>
#include "Process.h"

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

    void FindDataInMemory(const Process& proc, const size_t dataSize, const void* dataToFind);


    // std::from_chars takes different arguments depending on if the type is integral or float
    template <std::integral T>
    T StrToNumber(const char* dataString, const size_t strLength)
    {
        T dataValue = 0;

        std::from_chars_result res;
        // Help this function out
        // Use hex if the input is in hex
        if (dataString[0] == '0' && (dataString[1] == 'x' || dataString[1] == 'X'))
        {
            dataString += 2; // Move past the '0x'
            res = std::from_chars(dataString, dataString + strLength - 2, dataValue, 16);
        }
        else // Use generic conversion
        {
            res = std::from_chars(dataString, dataString + strLength, dataValue);
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
        return dataValue;
    }

    template <std::floating_point T>
    T StrToNumber(const char* dataString, const size_t strLength)
    {
        T dataValue = 0;
        
        std::from_chars_result res;
        // It also needs help with floating point numbers
        if (dataString[0] == '0' && (dataString[1] == 'x' || dataString[1] == 'X'))
        {
            dataString += 2; // Move past the '0x'
            res = std::from_chars(dataString, dataString + strLength - 2, dataValue, std::chars_format::hex);
        }
        else
        {
            res = std::from_chars(dataString, dataString + strLength, dataValue);
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
        return dataValue;
    }
}

