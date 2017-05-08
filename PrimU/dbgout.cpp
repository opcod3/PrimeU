
#include "stdafx.h"
#include "memory.h"
#include "executor.h"
#include "vprintf.h"
#include "InterruptHandler.h"

uint32_t dbgMsg(Arguments* args)
{


    auto stack_ptr = __GET(uint32_t*, args->sp);

    if (stack_ptr == nullptr)
        return -1;

    *(stack_ptr - 0x10) = args->r0;
    *(stack_ptr - 0xC) = args->r1;
    *(stack_ptr - 0x8) = args->r2;
    *(stack_ptr - 0x4) = args->r3;
    //va_list args = sp;
    char* fmt = __GET(char*, args->r0);

    if (fmt != nullptr)
        ee_vsprintf(fmt, reinterpret_cast<va_list>(stack_ptr - 0xC));

    //ee_vsprintf(progmem->get_data(), fmt, )

    //vprintf()

    return 0;
}
