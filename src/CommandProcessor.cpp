#include "CommandProcessor.h"
#include "Utils.h"
#include "Process.h"
#include <exception>
#include <vector>
#include <fmt/core.h>

#include <unordered_map>
#include "cmds/ICommand.h"
#include "cmds/PidCommand.h"
#include "cmds/MapCommand.h"
#include "cmds/DumpCommand.h"
#include "cmds/WriteCommand.h"
#include "cmds/FindCommand.h"
#include "cmds/ScanCommand.h"

using CommandMainFunc = void (*)(Process&, const std::vector<std::string>&);
using CommandHelpFunc = std::string (*)();

struct CmdFuncs
{
    CommandMainFunc MainFunc;
    CommandHelpFunc HelpFunc;
};

// All the commands are stored in this map
static const std::unordered_map<std::string , CmdFuncs> cmdMap =
{
    { "pid",  { &ICommand<PidCommand>::Main,  &ICommand<PidCommand>::Help } },
    { "map",  { &ICommand<MapCommand>::Main,  &ICommand<MapCommand>::Help } },
    { "dump", { &ICommand<DumpCommand>::Main, &ICommand<DumpCommand>::Help } },
    { "write", { &ICommand<WriteCommand>::Main, &ICommand<WriteCommand>::Help } },
    { "find", { &ICommand<FindCommand>::Main, &ICommand<FindCommand>::Help } },
    { "scan", { &ICommand<ScanCommand>::Main, &ICommand<ScanCommand>::Help } }
};


void CommandProcessor::ProcessCommand(std::string &input, Process& proc)
{
    // Split the input of the user into tokens
    // tokens[0] is always the command, while the rest of the tokens are arguments
    std::vector<std::string> tokens = Utils::SplitString(input, ' ');
    
    // Leave if the input is empty
    if (tokens.empty())
    {
        return;
    }

    std::string command = tokens[0];

    if (command == "help")
    {
        CommandProcessor::HelpCommand(tokens);
    }
    else if (command != "exit")
    {
        // Gets an iterator to the command the user wants
        auto commandFuncIt = cmdMap.find(command);
        if (commandFuncIt != cmdMap.end())
        {
            try
            {
                commandFuncIt->second.MainFunc(proc, tokens);
            }
            catch (const std::exception& e)
            {
                fmt::print(stderr, "{}: {}\n", command, e.what());
            }
        }
        else
        {
            fmt::print(stderr, "Command {} not found.\n", command);
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
        fmt::print("Available commands:\n");
        for (auto i = cmdMap.cbegin(); i != cmdMap.cend(); i++)
        {
            fmt::print("- {}\n", i->first);
        }
    }
    else // If an argument was given, find the command in the command map and execute the help function
    {
        auto cmdFunctions = cmdMap.find(command);
        if (cmdFunctions == cmdMap.end())
        {
            fmt::print(stderr, "{}: Command '{}' not found or no help available.\n", args[0], command);
        }
        else
        {
            fmt::print("\n{}\n", cmdFunctions->second.HelpFunc());
        }
    }
}

