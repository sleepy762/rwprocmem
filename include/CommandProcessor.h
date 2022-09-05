#pragma once
#include <string>
#include <vector>
#include "Process.h"

namespace CommandProcessor
{
    void ProcessCommand(std::string& input, Process& proc);
    void HelpCommand(const std::vector<std::string>& args);
}

