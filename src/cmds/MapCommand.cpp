#include "cmds/MapCommand.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

void MapCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    (void)args;
    if (proc.GetCurrentPid() == 0)
    {
        throw std::runtime_error("A pid has not been set. (see command 'pid')");
    }

    std::vector<mem_region_t> memRegions = proc.GetMemoryRegions();
    if (memRegions.size() == 0)
    {
        std::cout << "No memory regions were found.\n";
    }
    else
    {
        unsigned long totalMem = 0; // Total amount of bytes used

        for (size_t i = 0; i < memRegions.size(); i++)
        {
            totalMem += memRegions[i].rangeLength;
            // The maximum length of the "addressRange" string is 33
            // an address in hex can be up to 16 characters, this string has 2 of them + the '-' inbetween
            std::cout << std::left << std::setw(33) << memRegions[i].addressRange << '\t' <<
                memRegions[i].rangeLength << " bytes\t" <<
                memRegions[i].perms << '\t' <<
                memRegions[i].pathName << '\n';
        }
        std::cout << "Total: " << totalMem << " bytes.\n";
    }
}

const char* MapCommand::Help()
{
    return "Usage: map\n"
        "If attached to a process, print the memory regions map (/proc/pid/maps)\n"
        "The fields are the following in order:\n"
        "<memory address range> <range length> <permissions> <pathname>\n";
}

