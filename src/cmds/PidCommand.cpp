#include "cmds/PidCommand.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include "Utils.h"

void PidCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    std::string pidArg = "";
    // Check if an argument was given
    try
    {
        pidArg = args.at(1);
    }
    catch (const std::out_of_range&) {}
    
    // If not, print the current pid in use
    if (pidArg == "")
    {
        pid_t currPid = proc.GetCurrentPid();

        if (currPid == 0)
        {
            std::cout << "Currently not attached to any process.\n";
        }
        else
        {
            std::string procCmd = Utils::GetProcessCommand(currPid);
            std::cout << "Process " << currPid << ": " << procCmd << '\n';
        }
    }
    else // Check if it's a valid pid and use it
    {
        pid_t newPid;

        try
        {
            newPid = std::stoi(pidArg);
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Invalid pid.");
        }
        
        std::string procCmd = Utils::GetProcessCommand(newPid);
        std::cout << "Process " << newPid << ": " << procCmd << '\n';

        proc.SetProcessPid(newPid);
        std::cout << "Pid set to " << newPid << ".\n";
    }
}

std::string PidCommand::Help()
{
    return std::string(
        "Usage: pid [pid]\n\n"
        "If [pid] is passed, the program will use specified pid.\n"
        "If no argument was passed then the currently used pid will be printed.\n");
}


