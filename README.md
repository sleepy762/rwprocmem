# rwprocmem
A program that can read and modify the memory of other processes.

# Building
This program is not cross-platform and only works on GNU/Linux systems.

A Linux kernel of version 3.2 or higher must be used, and the option `CONFIG_CROSS_MEMORY_ATTACH` should be enabled (required to use to the syscalls `process_vm_readv/process_vm_writev`).

Any compiler which supports C++20 can be used, `gcc` is used by default in the `Makefile`.

To build the program, run `make` in the root directory of the repository.
