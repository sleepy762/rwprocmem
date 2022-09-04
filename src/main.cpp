#include <exception>
#include <string>
#include <iostream>
#include "CommandProcessor.h"
#include "Process.h"

int main()
{
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

