#include "CommandProcessor.h"
#include "Utils.h"
#include "Process.h"
#include <vector>
#include <iostream>
#include <stdexcept>

#include <unordered_map>
#include "cmds/ICommand.h"
#include "cmds/PidCommand.h"
#include "cmds/MapCommand.h"
#include "cmds/DumpCommand.h"
#include "cmds/WriteCommand.h"

using CommandMainFunc = void (*)(Process&, const std::vector<std::string>&);
using CommandHelpFunc = const char* (*)();

struct cmd_funcs_t
{
    CommandMainFunc MainFunc;
    CommandHelpFunc HelpFunc;
};

static const std::unordered_map<std::string , cmd_funcs_t> cmdMap =
{
    { "pid",  { &ICommand<PidCommand>::Main,  &ICommand<PidCommand>::Help } },
    { "map",  { &ICommand<MapCommand>::Main,  &ICommand<MapCommand>::Help } },
    { "dump", { &ICommand<DumpCommand>::Main, &ICommand<DumpCommand>::Help } },
    { "write", {&ICommand<WriteCommand>::Main, &ICommand<WriteCommand>::Help } }
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
    std::string command = tokens[0];

    if (command == "help")
    {
        CommandProcessor::HelpCommand(tokens);
    }
    else if (command != "exit")
    {
        auto commandFuncIt = cmdMap.find(command);
        if (commandFuncIt != cmdMap.end())
        {
            commandFuncIt->second.MainFunc(proc, tokens);
        }
        else
        {
            std::cerr << "Command " << command << " not found.\n";
        }
    }
}

void CommandProcessor::HelpCommand(const std::vector<std::string>& args)
{
    // The requested command to get help for
    std::string command = "";
    // Get the the argument if it was given
    try
    {
        command = args.at(1);
    }
    catch(const std::out_of_range&) {} // Do nothing, this just means the argument wasn't passed

    if (command == "")
    {
        // Print all available commands if no argument was given
        std::cout << "Available commands:\n";
        for (auto i = cmdMap.cbegin(); i != cmdMap.cend(); i++)
        {
            std::cout << "- " <<  i->first << '\n';
        }
    }
    else // If an argument was given, find the command in the command map and execute the help function
    {
        auto cmdFunctions = cmdMap.find(command);
        if (cmdFunctions == cmdMap.end())
        {
            std::cerr << args[0] << ": command '" << command << "' not found or no help available.\n";
        }
        else
        {
            std::cout << '\n' << cmdFunctions->second.HelpFunc() << '\n';
        }
    }
}

