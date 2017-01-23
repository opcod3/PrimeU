#ifndef ELFLOADER_H
#define ELFLOADER_H

#include "stdafx.h"

#define check(f, v) if (f != v) return;


class Memory;

enum State
{
    EXEC_LOADED       = 0x0,
    EXEC_EXECUTING    = 0x1,

    EXEC_LOAD_FAILED  = -0x1
};


class Executable
{
public:
    Executable(char* path);
    static uint32_t load_elf_to_memory(char* path, Memory* mem);

    State get_state() { return m_state; }
    uint32_t get_entry() { return m_entry; }

    Memory* get_mem() { return m_mem; }

private:

private:
    State m_state;
    Memory* m_mem;
    uint32_t m_entry;
};

#endif