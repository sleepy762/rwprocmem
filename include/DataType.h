#pragma once
#include <string>

enum class DataType
{
    int8,
    int16,
    int32,
    int64,
    uint8,
    uint16,
    uint32,
    uint64,
    f32,
    f64,
    string,
};

DataType ParseDataType(const std::string& typeStr);

