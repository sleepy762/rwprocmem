#include <exception>
#include <string>
#include <iostream>
#include "CommandProcessor.h"
#include "Process.h"
#include <unistd.h>

int main()
{
    std::cout << "rwprocmem v0.0.1\n";
    std::cout << "Type 'help' to see a list of commands.\n";
    if (getuid() != 0)
    {
        std::cout << "WARNING: not running as root, reading and writing to the memory of certain processes may not work.\n";
    }

    std::string input = "";
    
    Process proc;
    while (input != "exit")
    {
        std::cout << "> ";
        std::getline(std::cin, input);
        try
        {
            CommandProcessor::ProcessCommand(input, proc);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
    }

    return 0;
}

