
#include "stdafx.h"
#include "memory.h"
#include "executor.h"
#include "vprintf.h"

uint32_t dbgMsg(uc_engine* uc, uint32_t r0, uint32_t r1, uint32_t r2, uint32_t r3, uint32_t sp, Memory* stack, Memory* progmem)
{


    auto stack_ptr = sExecutor->get_from_memory<uint32_t>(sp);

    if (stack_ptr == nullptr)
        return -1;

    *(stack_ptr - 0x10) = r0;
    *(stack_ptr - 0xC) = r1;
    *(stack_ptr - 0x8) = r2;
    *(stack_ptr - 0x4) = r3;
    //va_list args = sp;
    char* fmt = sExecutor->get_from_memory<char>(r0);

    if (fmt != nullptr)
        ee_vsprintf(fmt, reinterpret_cast<va_list>(stack_ptr - 0xC));

    //ee_vsprintf(progmem->get_data(), fmt, )

    //vprintf()

    return 0;
}
