#include "ComparisonType.h"
#include <stdexcept>

ComparisonType ParseComparisonType(const std::string& keywordStr)
{
    if (keywordStr == "==")
    {
        return ComparisonType::Equal;
    }
    else if (keywordStr == "!=")
    {
        return ComparisonType::NotEqual;
    }
    else if (keywordStr == ">")
    {
        return ComparisonType::Greater;
    }
    else if (keywordStr == "<")
    {
        return ComparisonType::Less;
    }
    else if (keywordStr == ">=")
    {
        return ComparisonType::GreaterEqual;
    }
    else if (keywordStr == "<=")
    {
        return ComparisonType::LessEqual;
    }
    else
    {
        throw std::runtime_error("Invalid scan type.");
    }
}

