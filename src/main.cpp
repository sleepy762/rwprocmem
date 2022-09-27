#include <exception>
#include <string>
#include "CommandProcessor.h"
#include "Process.h"
#include <unistd.h>
#include <iostream>
#include <fmt/core.h>

int main()
{
    fmt::print("rwprocmem v1.1.2\n");
    fmt::print("Type 'help' to see a list of commands.\n");
    if (getuid() != 0)
    {
        fmt::print("WARNING: not running as root, reading and writing to the memory of certain processes may not work.\n");
    }

    std::string input = "";
    
    Process proc;
    while (input != "exit")
    {
        fmt::print("> ");
        if (!std::getline(std::cin, input))
        {
            break;
        }

        CommandProcessor::ProcessCommand(input, proc);
    }

    return 0;
}

