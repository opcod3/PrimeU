

#include "stdafx.h"
#include "memory.h"
#include <stdlib.h>

#include "executor.h"
#include "LCD.h"

#define DUMPARGS printf("    r0: %08X|%i\n    r1: %08X|%i\n    r2: %08X|%i\n    r3: %08X|%i\n    sp: %08X\n", r0, r0, r1, r1, r2, r2, r3, r3, sp)

VirtPtr struc = 0;

uint32_t getCurrentDir(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    static char* path = "A:\\WINDOW\\SYSTEM\\";

    VirtPtr addr;
    sMemoryManager->DyanmicAlloc(&addr, 30);
    strcpy_s(__GET(char*, addr), 30, path);


    return addr;
}


uint32_t prgrmIsRunning(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    printf("    program: %s\n",__GET(char*, r0));

    if (struc == 0)
        sMemoryManager->DyanmicAlloc(&struc, 0x250);

    return struc;
}

uint32_t _FindResourceW(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    return 0;
}
// 

uint32_t _OpenFile(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    printf("    +name: %s, flags: %s", __GET(char*, r0), __GET(char*, r1));
    return 0;
}

uint32_t _LoadLibraryA(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    return 1;
}

uint32_t _FreeLibrary(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    return 1;
}

uint32_t OSInitCriticalSection(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    return 0;
}

uint32_t OSCreateEvent(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    DUMPARGS;
    return 4415;
}

uint32_t OSSetEvent(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    DUMPARGS;
    return 0;
}

uint32_t LCDOn(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    DUMPARGS;
    return 0;
}

uint32_t GetActiveLCD(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    DUMPARGS;
    VirtPtr buffer, ptrBuf;
    sMemoryManager->DyanmicAlloc(&buffer, sizeof(LCD_MAGIC_SUPER));
    sMemoryManager->DyanmicAlloc(&ptrBuf, 0x8);
    *__GET(uint32_t*, ptrBuf) = buffer;
    *__GET(LCD_MAGIC_SUPER*, buffer) = LCD_MAGIC_SUPER(0x45000000);
    printf("    ret: %08X\n", ptrBuf);
    return ptrBuf;
}

uint32_t lcalloc(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    printf("    +nElements: %i | size: %i\n", r0, r1);

    //uint32_t virt_addr = sExecutor->alloc_dynamic_mem(r0*r1);
    //auto addr = sExecutor->get_from_memory<void>(virt_addr);
    //memset(addr, 0, r0*r1);

    VirtPtr addr;
    if (sMemoryManager->DyanmicAlloc(&addr, r0) == ERROR_OK)
        return addr;

    return 0;
}

uint32_t lmalloc(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    printf("    +size: %i\n", r0);

    VirtPtr addr;
    if (sMemoryManager->DyanmicAlloc(&addr, r0) == ERROR_OK)
        return addr;

    return 0;
}

uint32_t _lfree(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp)
{
    if (sMemoryManager->DynamicFree(r0) != ERROR_OK)
        printf("    +error\n");
    return r0;
}