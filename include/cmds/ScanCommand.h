#pragma once
#include "ICommand.h"

class ScanCommand : public ICommand<ScanCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static std::string Help();
};

