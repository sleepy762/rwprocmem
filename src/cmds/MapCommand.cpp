#include "cmds/MapCommand.h"
#include <stdexcept>
#include <fmt/core.h>

void MapCommand::Main(Process& proc, const std::vector<std::string>& args)
{
    (void)args; // This command doesn't use any args, this silences a warning

    const std::vector<MemRegion> memRegions = proc.GetMemoryRegions();
    if (memRegions.empty())
    {
        fmt::print("No memory regions were found.\n");
    }
    else
    {
        unsigned long totalMem = 0; // Total amount of bytes used
        
        // Gets the amount of digits in the number of memory regions
        const size_t indexWidth = std::to_string(memRegions.size()).size();

        for (size_t i = 0; i < memRegions.size(); i++)
        {
            totalMem += memRegions[i].rangeLength;

            fmt::print("[{:{}}] {:#018x}-{:#018x}\t{} bytes\t[{}]\t{}\n", 
                    i, indexWidth,
                    memRegions[i].startAddr, memRegions[i].endAddr,
                    memRegions[i].rangeLength,
                    memRegions[i].permsStr,
                    memRegions[i].pathName);
        }
        fmt::print("\nTotal: {} bytes in {} memory regions.\n", totalMem, memRegions.size());
    }
}

std::string MapCommand::Help()
{
    return std::string(
        "Usage: map\n\n"

        "If attached to a process, print the memory regions map (/proc/pid/maps)\n"
        "The fields are the following in order:\n"
        "<memory address range> <range length> <permissions> <pathname>\n");
}

