#pragma once
#include "cmds/ICommand.h"

class MapCommand : public ICommand<MapCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static std::string Help();
};

