

#include "stdafx.h"
#include "memory.h"
#include <stdlib.h>

#include "executor.h"
#include "LCD.h"
#include "InterruptHandler.h"

#define DUMPARGS printf("    r0: %08X|%i\n    r1: %08X|%i\n    r2: %08X|%i\n    r3: %08X|%i\n    r4: %08X|%i\n    sp: %08X\n", \
    args->r0, args->r0, args->r1, args->r1, args->r2,\
    args->r2, args->r3, args->r3, args->r4, args->r4, args->sp)

VirtPtr struc = 0;

uint32_t getCurrentDir(Arguments* args)
{
    static char* path = "A:\\WINDOW\\SYSTEM\\";

    VirtPtr addr;
    sMemoryManager->DyanmicAlloc(&addr, 30);
    strcpy_s(__GET(char*, addr), 30, path);


    return addr;
}


uint32_t prgrmIsRunning(Arguments* args)
{
    printf("    program: %s\n",__GET(char*, args->r0));

    if (struc == 0)
        sMemoryManager->DyanmicAlloc(&struc, 0x250);

    return struc;
}

uint32_t _FindResourceW(Arguments* args)
{
    return 0;
}
// 

uint32_t _OpenFile(Arguments* args)
{
    printf("    +name: %s, flags: %s", __GET(char*, args->r0), __GET(char*, args->r1));
    return 0;
}

uint32_t _LoadLibraryA(Arguments* args)
{
    return 1;
}

uint32_t _FreeLibrary(Arguments* args)
{
    return 1;
}

uint32_t OSInitCriticalSection(Arguments* args)
{
    return 0;
}

uint32_t OSCreateEvent(Arguments* args)
{
    DUMPARGS;
    return 4415;
}

uint32_t OSSetEvent(Arguments* args)
{
    DUMPARGS;
    return 0;
}

uint32_t LCDOn(Arguments* args)
{
    DUMPARGS;
    return 0;
}

uint32_t GetActiveLCD(Arguments* args)
{
    //DUMPARGS;
    ErrorCode err = ERROR_OK;
    VirtPtr lcd, virt_ptrBuf;
    //new LCD(&virt_buffer, LCD_BUFFER);
    if ((err = sMemoryManager->DyanmicAlloc(&lcd, sizeof(LCD))) != ERROR_OK)
        __debugbreak();
    LCD* realLCD = reinterpret_cast<LCD*>(sMemoryManager->GetRealAddr(lcd));
    realLCD = new (realLCD) LCD();

    if ((err = sMemoryManager->DyanmicAlloc(&virt_ptrBuf, 0x4)) != ERROR_OK)
        __debugbreak();
    *__GET(uint32_t*, virt_ptrBuf) = reinterpret_cast<uint32_t>(realLCD->LCDMagicPtr);
    
    //printf("    ret: %08X\n", virt_ptrBuf);

    return virt_ptrBuf;
}

uint32_t lcalloc(Arguments* args)
{
    printf("    +nElements: %i | size: %i\n", args->r0, args->r1);

    //uint32_t virt_addr = sExecutor->alloc_dynamic_mem(r0*r1);
    //auto addr = sExecutor->get_from_memory<void>(virt_addr);
    //memset(addr, 0, r0*r1);

    VirtPtr addr;
    if (sMemoryManager->DyanmicAlloc(&addr, args->r0) == ERROR_OK)
        return addr;

    return 0;
}

uint32_t lmalloc(Arguments* args)
{
    printf("    +size: %i\n", args->r0);

    VirtPtr addr;
    if (sMemoryManager->DyanmicAlloc(&addr, args->r0) == ERROR_OK)
        return addr;

    return 0;
}

uint32_t lrealloc(Arguments* args)
{
    VirtPtr ptr = args->r0;
    uint32_t new_size = args->r1;

    if (ptr == 0) {
        sMemoryManager->DyanmicAlloc(&ptr, new_size);
        return ptr;
    }

    printf("    +addr: %08X, size: %X", ptr, new_size);
    sMemoryManager->DynamicRealloc(&ptr, static_cast<size_t>(new_size));
    printf("    +new_addr: %08X", ptr);
    return ptr;
}

uint32_t _lfree(Arguments* args)
{
    ErrorCode err;
    if ((err = sMemoryManager->DynamicFree(args->r0)) != ERROR_OK)
        printf("    +error\n");
    return args->r0;
}

uint32_t _amkdir(Arguments* args)
{
    printf("    +name: %s, flags: %s", __GET(char*, args->r0), __GET(char*, args->r1));
    return 0;
}

uint32_t _achdir(Arguments* args)
{
    printf("    +name: %s, flags: %s", __GET(char*, args->r0), __GET(char*, args->r1));
    return 0;
}

uint32_t __wfopen(Arguments* args)
{
    wprintf(L"    +name: %s, flags: %s", __GET(wchar_t*, args->r0), __GET(wchar_t*, args->r1));
    return 0;
}

uint32_t OSCreateThread(Arguments* args)
{
    DUMPARGS;
    return sExecutor->NewThread(args->r0, args->r4);
}