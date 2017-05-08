#ifndef INTERRUPTHANDLER_H
#define INTERRUPTHANDLER_H

#include "stdafx.h"
#include "interrupts.h"
#include "executor.h"


enum HandleStatus
{
    HANDLE_IMPLEMENTED,
    HANDLE_NAMEONLY,
    HANDLE_UNKOWN
};

struct Arguments
{
    Arguments()
    {
        uc_reg_read_batch(sExecutor->GetUcInstance(), (int*)_regs, (void**)_args, 5);
    }
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t sp;
private:
    uc_arm_reg _regs[5] =
    {
        UC_ARM_REG_R0,
        UC_ARM_REG_R1,
        UC_ARM_REG_R2,
        UC_ARM_REG_R3,
        UC_ARM_REG_SP
    };

    uint32_t* _args[5] =
    {
        &r0,
        &r1,
        &r2,
        &r3,
        &sp
    };
};

typedef uint32_t(*Handler)(Arguments* args);

struct InterruptHandler
{
    InterruptHandler(InterruptID id, HandleStatus status, Handler callback, char* name) : Id(id), Status(status), Callback(callback), Name(name) { }
    InterruptID Id;
    HandleStatus Status;
    Handler Callback;
    char* Name;
};

#endif

