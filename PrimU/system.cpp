

#include "stdafx.h"
#include "memory.h"
#include <stdlib.h>

#include "executor.h"

uint32_t struc = 0;

uint32_t getCurrentDir(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    static char* path = "A:\\WINDOW\\SYSTEM\\";

    uint32_t addr = sExecutor->alloc_dynamic_mem(30);
    strcpy(sExecutor->get_from_memory<char>(addr), path);


    return addr;
}


uint32_t prgrmIsRunning(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    printf("    program: %s\n", sExecutor->get_from_memory<char>(r0));

    if (struc == 0)
        struc = sExecutor->alloc_dynamic_mem(0x250);

    return struc;
}

uint32_t _FindResourceW(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    return 0;
}
// 

uint32_t _OpenFile(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    printf("    +name: %s, flags: %s", sExecutor->get_from_memory<char>(r0), sExecutor->get_from_memory<char>(r1));
    return 0;
}

uint32_t _LoadLibraryA(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    return 1;
}

uint32_t _FreeLibrary(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    return 1;
}

uint32_t OSInitCriticalSection(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    return 0;
}

uint32_t lcalloc(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    printf("    +nElements: %i | size: %i\n", r0, r1);

    uint32_t virt_addr = sExecutor->alloc_dynamic_mem(r0*r1);
    auto addr = sExecutor->get_from_memory<void>(virt_addr);
    memset(addr, 0, r0*r1);

    return virt_addr;
}

uint32_t lmalloc(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{
    printf("    +size: %i\n", r0);

    return  sExecutor->alloc_dynamic_mem(r0);
}