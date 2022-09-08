#pragma once
#include "cmds/ICommand.h"

class WriteCommand : public ICommand<WriteCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static const char* Help();
};

