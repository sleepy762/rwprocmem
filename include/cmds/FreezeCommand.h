#pragma once
#include "ICommand.h"

class FreezeCommand : public ICommand<FreezeCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static std::string Help();
};

