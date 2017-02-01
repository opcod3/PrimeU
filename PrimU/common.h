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

enum MemoryRegion : VirtPtr
{
    MEM_STACK = 0x10000000,
    LCD_BUFFER = 0x40600000
};

#define PAGE_SIZE 0x1000

#define __check(f, v, e) if (f != v) return e

#endif