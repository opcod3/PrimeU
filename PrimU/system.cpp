

#include "stdafx.h"
#include <chrono>
#include <ctime>


#include "executor.h"
#include "LCD.h"
#include "InterruptHandler.h"
#include "ThreadHandler.h"

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
    sThreadHandler->InitCriticalSection(__GET(CriticalSection*, args->r0));
    return args->r0;
}

uint32_t OSEnterCriticalSection(Arguments* args)
{
    sThreadHandler->CurrentThreadEnterCriticalSection(__GET(CriticalSection*, args->r0));
    return args->r0;
}

uint32_t OSLeaveCriticalSection(Arguments* args)
{
    sThreadHandler->CurrentThreadExitCriticalSection(__GET(CriticalSection*, args->r0));
    return args->r0;
}

uint32_t OSSleep(Arguments* args)
{
    sThreadHandler->CurrentThreadSleep(args->r0);
    return args->r0;
}

struct EVENT
{
    uint32_t unk0 = 0x201;
    uint32_t unk1;
    uint16_t unk2;
    uint8_t unk3;
    uint8_t unk4;
    uint8_t unk5;
    uint8_t unk6;
    uint8_t unk7;
    uint8_t unk8;
    uint8_t unk9;
    uint8_t unk10;
    uint8_t unk11;
};

uint32_t OSCreateEvent(Arguments* args)
{
    VirtPtr allocAddr;
    sMemoryManager->DyanmicAlloc(&allocAddr, 0x14);
    EVENT* ptr = __GET(EVENT*, allocAddr);
    new (ptr) EVENT();

    ptr->unk1 = args->r1;
    ptr->unk2 = args->r0;

    return allocAddr;
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
    return sLCDHandler->GetActiveLCDPtr();
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
    return sThreadHandler->NewThread(args->r0, args->r4);
}

uint32_t OSSetThreadPriority(Arguments* args)
{
    DUMPARGS;
    sThreadHandler->SetThreadPriority(args->r0, args->r1);
    return 0;
}

struct SystemTime
{
    uint16_t Year;
    uint16_t Month;
    uint16_t DayOfWeek;
    uint16_t Day;
    uint16_t Hour;
    uint16_t Minute;
    uint16_t Second;
    uint16_t Milliseconds;
};

uint32_t GetSysTime(Arguments* args)
{
    SystemTime* sysTime = __GET(SystemTime*, args->r0);
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    tm *parts = std::localtime(&now_c);

    sysTime->Year = parts->tm_yday;
    sysTime->Month = parts->tm_mon;
    sysTime->DayOfWeek = parts->tm_wday;
    sysTime->Day = parts->tm_mday;
    sysTime->Hour = parts->tm_hour;
    sysTime->Minute = parts->tm_min;
    sysTime->Second = parts->tm_sec;
    auto totalMSec = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    sysTime->Milliseconds = static_cast<uint16_t>(totalMSec % 1000);

    return args->r0;
}

uint32_t _GetPrivateProfileString(Arguments* args)
{
    printf("    +appname: %s\n    +keyName: %s\n    +default: %s\n    +size: %i\n    +filename: %s",
        __GET(char*, args->r0), __GET(char*, args->r1), __GET(char*, args->r2),
            *__GET(int*, args->sp + 8), __GET(char*, *__GET(VirtPtr*, args->sp + 0xC)));
    return args->r0;
}