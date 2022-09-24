#pragma once
#include <string>

enum class ComparisonType
{
    Equal,
    NotEqual,
    Greater,
    Less,
    GreaterEqual,
    LessEqual,
};

ComparisonType ParseComparisonType(const std::string& keywordStr);

