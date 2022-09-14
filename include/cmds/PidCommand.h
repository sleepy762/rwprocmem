#pragma once
#include "cmds/ICommand.h"

class PidCommand : public ICommand<PidCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static std::string Help();
};

