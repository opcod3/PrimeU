#ifndef INTERRUPTHANDLE_H
#define INTERRUPTHANDLE_H

#include "stdafx.h"
#include "interrupts.h"

typedef uint32_t (*syscall_p)(uc_engine* uc, uint32_t /*r0*/, uint32_t /*r1*/, uint32_t /*r2*/, uint32_t /*r3*/, uint32_t /*sp*/);

enum HandleStatus
{
    HANDLE_IMPLEMENTED,
    HANDLE_NAMEONLY,
    HANDLE_UNKOWN
};

struct InterruptHandle
{
    InterruptHandle(InterruptID id, HandleStatus status, syscall_p callback, char* name) : id(id), status(status), callback(callback), name(name) { }
    InterruptID id;
    HandleStatus status;
    syscall_p callback;
    char* name;
};

#endif

