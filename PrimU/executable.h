#ifndef ELFLOADER_H
#define ELFLOADER_H

#include "common.h"
#include <elfio/elfio.hpp>


class Memory;

enum State
{
    EXEC_LOADED       = 0x0,
    EXEC_EXECUTING    = 0x1,
    EXEC_NOT_INITIALIZED = 0x2,


    EXEC_LOAD_FAILED  = -0x1
};


class Executable
{
public:
    Executable(char* path) : _path(path), _state(EXEC_NOT_INITIALIZED) { }
    ErrorCode Load();

    State get_state() { return _state; }
    uint32_t get_entry() { return _entry; }

    //Memory* get_mem() { return m_mem; }

private:

private:
    char* _path;

    VirtPtr _address;
    uint32_t _entry;
    size_t _size;

    State _state;
};

#endif