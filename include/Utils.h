#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include <concepts>
#include <stdexcept>
#include <charconv>
#include "MemoryStructs.h"
#include <fmt/core.h>

namespace Utils
{
    std::vector<std::string> SplitString(const std::string& str, char delim);

    std::string GetProcessCommand(pid_t pid);

    std::string JoinVectorOfStrings(const std::vector<std::string>& vec, int startIndex, 
            char joinChar);

    void PrintMemoryAddresses(const std::vector<MemAddress>& memAddrs);

    MemRegion FindRegionOfAddress(const std::vector<MemRegion>& memRegions, unsigned long address);

    // These function templates are very thin wrappers around std::from_chars
    // std::from_chars is actually unable to detect whether a number is in hex
    // so we have to explicitly call the function with a base 16/hex argument
    template <std::integral T>
    inline std::from_chars_result from_chars_hex(const char* start, const char* end, T& value);

    template <std::floating_point T>
    inline std::from_chars_result from_chars_hex(const char* start, const char* end, T& value);

    template <typename T>
    T StrToNumber(const std::string& dataString, std::string varName = "data"); 
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

// Set varName to make the errors easier to understand
template <typename T>
T Utils::StrToNumber(const std::string& dataString, std::string varName)
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
        const std::string err = fmt::format("Invalid {}.", varName);
        throw std::invalid_argument(err);
    }
    else if (res.ec == std::errc::result_out_of_range)
    {
        const std::string err = fmt::format(
            "Result of {} string conversion to a number is out of range for the given type.", varName);
        throw std::runtime_error(err);
    }
    else if (res.ptr != dataStringEnd)
    {
        const std::string err = fmt::format(
            "Failed to fully convert the given {} string to a number.", varName);
        throw std::runtime_error(err);
    }
    return dataValue;
}

