#pragma once
#include "Process.h"
#include <vector>
#include <string>

template <typename T>
class ICommand 
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args)
    {
        T::Main(proc, args);
    }
    static void Help()
    {
        T::Help();
    }
};

