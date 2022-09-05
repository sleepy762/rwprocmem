#pragma once
#include "cmds/ICommand.h"
#include "Process.h"
#include <vector>

class PidCommand : public ICommand<PidCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static void Help();
};

