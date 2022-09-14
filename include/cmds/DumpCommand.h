#pragma once
#include "ICommand.h"

class DumpCommand : public ICommand<DumpCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static std::string Help();
};

