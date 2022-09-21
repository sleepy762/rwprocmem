#include "DataType.h"
#include <stdexcept>

DataType ParseDataType(const std::string& typeStr)
{
    if (typeStr[0] == 'i')
    {
        if (typeStr == "int8")
        {
            return DataType::int8;
        }
        else if (typeStr == "int16")
        {
            return DataType::int16;
        }
        else if (typeStr == "int32")
        {
            return DataType::int32;
        }
        else if (typeStr == "int64")
        {
            return DataType::int64;
        }
        else
        {
            throw std::runtime_error("Invalid signed type.");
        }
    }
    else if (typeStr[0] == 'u')
    {
        if (typeStr == "uint8")
        {
            return DataType::uint8;
        }
        else if (typeStr == "uint16")
        {
            return DataType::uint16;
        }
        else if (typeStr == "uint32")
        {
            return DataType::uint32;
        }
        else if (typeStr == "uint64")
        {
            return DataType::uint64;
        }
        else
        {
            throw std::runtime_error("Invalid unsigned type.");
        }
    }
    else if (typeStr == "float")
    {
        return DataType::f32;
    }
    else if (typeStr == "double")
    {
        return DataType::f64;
    }
    else if (typeStr == "string")
    {
        return DataType::string;
    }
    else
    {
        throw std::runtime_error("Invalid type.");
    }
}

