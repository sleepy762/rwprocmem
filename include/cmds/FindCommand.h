#pragma once
#include "ICommand.h"

class FindCommand : public ICommand<FindCommand>
{
public:
    static void Main(Process& proc, const std::vector<std::string>& args);
    static std::string Help();
};

