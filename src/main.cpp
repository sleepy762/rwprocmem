#include <exception>
#include <fmt/core.h>
#include <string>
#include "CommandProcessor.h"
#include "Process.h"
#include <unistd.h>
#include <iostream>
#include <fmt/format.h>

int main()
{
    fmt::print("rwprocmem v1.0.1\n");
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

        try
        {
            CommandProcessor::ProcessCommand(input, proc);
        }
        catch (const std::exception& e)
        {
            fmt::print(stderr, "{}\n", e.what());
        }
    }

    return 0;
}

