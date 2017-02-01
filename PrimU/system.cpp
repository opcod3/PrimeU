

#include "stdafx.h"
#include "memory.h"
#include <stdlib.h>

#include "executor.h"

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