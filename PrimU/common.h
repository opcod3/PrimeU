#ifndef COMMON_H
#define COMMON_H

#include "stdafx.h"

enum ErrorCode {
    ERROR_OK                           =0x00000000,
    ERROR_UC_NOT_INITIALIZED           =0x10000000,
    ERROR_UC_MAP                       =0x10000001,
    ERROR_UC_UNMAP                     =0x10000002,
    ERROR_MEM_ALREADY_ALLOCATED        =0x20000000,
    ERROR_MEM_ALLOC_FAIL               =0x20000001,
    ERROR_MEM_STATIC_NOT_FREEABLE      =0x20000002,
    ERROR_MEM_ADDR_NOT_ALLOCATED       =0x20000004,
    ERROR_LOADER_READER_FAIL           =0x40000000,
    ERROR_LOADER_INCORRECT_ATTRIBUTE   =0x40000001,
    ERROR_GENERIC                      =0xFFFFFFFF
};

typedef uint32_t VirtPtr;
typedef uint8_t* RealPtr;

enum RegionSize : size_t
{
    MEM_STACK_SIZE = 0x05000000,
    MEM_DYNAMIC_SIZE = 0x10000000,
    LCD_REGISTER_SIZE = 0x4E000000 - 0x4C800000
};

enum MemoryRegion : VirtPtr
{
    MEM_STACK = 0x10000000,
    MEM_DYNAMIC = MEM_STACK + MEM_STACK_SIZE,
    LCD_REGISTER = 0x4C800000
};

// THESE ARE WRONG
enum PRIORITY_LEVELS : uint8_t
{
    THREAD_PRIORITY_IDLE = 1,
    THREAD_PRIORITY_LOWEST = 6,
    THREAD_PRIORITY_BELOW_NORMAL = 7,
    THREAD_PRIORITY_NORMAL = 8,
    THREAD_PRIORITY_ABOVE_NORMAL = 9,
    THREAD_PRIORITY_HIGHEST = 10,
    THREAD_PRIORITY_TIME_CRITICAL = 15
};

#define PAGE_SIZE 0x1000
#define THREAD_INS 10000

#define __check(f, v, e) if (f != v) return e
#define __CAST(t, v) reinterpret_cast<t>(v)

#endif