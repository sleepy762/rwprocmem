#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include <concepts>
#include <stdexcept>
#include <charconv>
#include "MemoryStructs.h"

namespace Utils
{
    std::vector<std::string> SplitString(const std::string& str, char delim);

    std::string GetProcessCommand(pid_t pid);

    std::string JoinVectorOfStrings(const std::vector<std::string>& vec, int startIndex, 
            char joinChar);

    void PrintMemoryAddresses(const std::vector<MemAddress>& memAddrs);

    // These function templates are very thin wrappers around std::from_chars
    // std::from_chars is actually unable to detect whether a number is in hex
    // so we have to explicitly call the function with a base 16/hex argument
    template <std::integral T>
    inline std::from_chars_result from_chars_hex(const char* start, const char* end, T& value);

    template <std::floating_point T>
    inline std::from_chars_result from_chars_hex(const char* start, const char* end, T& value);

    template <typename T>
    T StrToNumber(const std::string& dataString); 
}


template <std::integral T>
inline std::from_chars_result Utils::from_chars_hex(const char* start, const char* end, T& value)
{
    return std::from_chars(start, end, value, 16);
}

template <std::floating_point T>
inline std::from_chars_result Utils::from_chars_hex(const char* start, const char* end, T& value)
{
    return std::from_chars(start, end, value, std::chars_format::hex);
}

template <typename T>
T Utils::StrToNumber(const std::string& dataString)
{
    T dataValue = 0;
    std::from_chars_result res;

    const char* dataStringStart = dataString.c_str();
    const char* dataStringEnd = dataStringStart + dataString.size();

    // Use hex if the input is in hex
    if (dataStringStart[0] == '0' && (dataStringStart[1] == 'x' || dataStringStart[1] == 'X'))
    {
        dataStringStart += 2; // Move past the '0x'
        res = Utils::from_chars_hex<T>(dataStringStart, dataStringEnd, dataValue);
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

