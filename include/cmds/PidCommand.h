#pragma once
#include "cmds/ICommand.h"
#include "Process.h"
#include <vector>
#include <string>

class PidCommand : public ICommand<PidCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static const char* Help();
};

