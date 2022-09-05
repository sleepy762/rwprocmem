#include "CommandProcessor.h"
#include "Utils.h"
#include "Process.h"
#include <vector>
#include <iostream>
#include <stdexcept>

#include <unordered_map>
#include "cmds/ICommand.h"
#include "cmds/PidCommand.h"

using CommandMainFunc = void (*)(Process&, const std::vector<std::string>&);
using CommandHelpFunc = void (*)();

struct cmd_funcs_t
{
    CommandMainFunc MainFunc;
    CommandHelpFunc HelpFunc;
};

std::unordered_map<const char*, cmd_funcs_t> cmdMap =
{
    { "pid", { &ICommand<PidCommand>::Main, &ICommand<PidCommand>::Help } }
};


void CommandProcessor::ProcessCommand(std::string &input, Process& proc)
{
    // Check if the input is empty
    if (input.size() == 0)
    {
        return;
    }

    // Split the input of the user into tokens
    // tokens[0] is always the command, while the rest of the tokens are arguments
    std::vector<std::string> tokens = Utils::SplitString(input, ' ');

    if (tokens[0] == "pid")
    {
        // Temp condition
        if (tokens.size() >= 2)
        {
            proc.SetProcessPid(std::stoi(tokens[1]));
        }
    }
    else if (tokens[0] == "map")
    {
        std::vector<mem_region_t> memRegs = proc.GetMemoryRegions();
        
        if (memRegs.size() == 0)
        {
            std::cout << "No memory regions were found.\n";
        }

        for (size_t i = 0; i < memRegs.size(); i++)
        {
            std::cout << memRegs[i].addressRange << '\t' <<
                memRegs[i].rangeLength << " bytes\t" <<
                memRegs[i].perms << '\t' <<
                memRegs[i].pathName << '\n';
        }
    }
}

void CommandProcessor::HelpCommand(const std::vector<std::string>& args)
{
    
}

