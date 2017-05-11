#include "executor.h"


#include "memory.h"
#include "executable.h"
#include "interrupts.h"
#include "InterruptHandler.h"
#include "SystemAPI.h"
#include "Thread.h"
#include "ThreadHandler.h"

#include <valarray>
#include <chrono>


Executor* Executor::m_instance = nullptr;

#define STACK_MIN 0x20000000

#define callAndcheckError(f) m_err = f; if (m_err != UC_ERR_OK) return false
#define DEFINE_INTERRUPT(id, s, n, c) m_interrupts.insert(std::pair<InterruptID, InterruptHandle*>(id, new InterruptHandle(id, s, c, n)))

void interrupt_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
void code_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
    static auto lastUpdate = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);
    bool canrun = sThreadHandler->CanCurrentThreadRun();
    if (elapsed.count() < sThreadHandler->GetCurrentThreadQuantum() && canrun) {
        return;
    }

    sThreadHandler->SaveCurrentThreadState();
    uc_emu_stop(sExecutor->GetUcInstance());
    printf(">>> Stopping at 0x%llX, instruction size = 0x%x\n", address, size);

    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, pc, lr;
    void* args[16] = { &r0, &r1, &r2, &r3, &r4, &r5, &r6, &r7, &r8, &r9, &r10, &r11, &r12, &sp, &lr, &pc };
    int regs[16] = { UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_R4, UC_ARM_REG_R5, UC_ARM_REG_R6,
        UC_ARM_REG_R7, UC_ARM_REG_R8, UC_ARM_REG_R9, UC_ARM_REG_R10, UC_ARM_REG_R11, UC_ARM_REG_R12, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC };
    uc_reg_read_batch(uc, regs, args, 16);

    printf("Registers: \n");
    printf("    r0: %08X|%i\n    r1: %08X|%i\n    r2: %08X|%i\n    r3: %08X|%i\n    r4: %08X|%i\n"\
           "    r5: %08X|%i\n    r6: %08X|%i\n    r7: %08X|%i\n    r8: %08X|%i\n    r9: %08X|%i\n"\
           "   r10: %08X|%i\n   r11: %08X|%i\n   r12: %08X|%i\n"\
           "    sp: %08X\n    pc: %08X\n    lr: %08X\n",
           r0, r0, r1, r1, r2, r2, r3, r3, r4, r4, r5, r5, r6, r6, r7, r7, r8, r8,
           r9, r9, r10, r10, r11, r11, r12, r12, sp, pc, lr);


    lastUpdate = std::chrono::high_resolution_clock::now();
}

bool Executor::Initialize(Executable* exec)
{
    if (!exec)
        return false;

    m_exec = exec;

    if (!m_uc)
    {
        m_err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &m_uc);
        if (m_err != UC_ERR_OK) return false;
    }

    __check(exec->Load(), ERROR_OK, false);

    __check(sMemoryManager->StaticAlloc(LCD_REGISTER, LCD_REGISTER_SIZE), ERROR_OK, false);

    __check(InitInterrupts(), true, false);

    return true;
}

bool Executor::Cleanup()
{
    callAndcheckError(uc_hook_del(m_uc, m_interrupt_hook));
    callAndcheckError(uc_hook_del(m_uc, _codeHook));
    __check(sMemoryManager->StaticFree(LCD_REGISTER), ERROR_OK, false);
    callAndcheckError(uc_close(m_uc));

}


bool Executor::InitInterrupts()
{
    callAndcheckError(uc_hook_add(m_uc, &m_interrupt_hook, UC_HOOK_INTR, interrupt_hook, this, 0, 1));
    callAndcheckError(uc_hook_add(m_uc, &_codeHook, UC_HOOK_CODE, code_hook, NULL, 1, 0));
    return true;
}


void Executor::Execute()
{
    sThreadHandler->NewThread(m_exec->get_entry(), 0, THREAD_PRIORITY_NORMAL, MEM_STACK_SIZE);
    sThreadHandler->LoadCurrentThreadState();


    m_err = UC_ERR_OK;
    printf("Starting execution at 0x%X\n\n", sThreadHandler->GetCurrentThreadPC());
    while (true)
    {
        m_err = uc_emu_start(m_uc, sThreadHandler->GetCurrentThreadPC(), 0, 0, 0);

        if (m_err != UC_ERR_OK)
            break;
        sThreadHandler->SwitchThread();
    }

    if (m_err != UC_ERR_OK) { 

        uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, pc, lr;
        void* args[16] = { &r0, &r1, &r2, &r3, &r4, &r5, &r6, &r7, &r8, &r9, &r10, &r11, &r12, &sp, &lr, &pc };
        int regs[16] = { UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_R4, UC_ARM_REG_R5, UC_ARM_REG_R6,
            UC_ARM_REG_R7, UC_ARM_REG_R8, UC_ARM_REG_R9, UC_ARM_REG_R10, UC_ARM_REG_R11, UC_ARM_REG_R12, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC };
        uc_reg_read_batch(sExecutor->GetUcInstance(), regs, args, 16);

        printf("Execution aborted on error: %s!\nThread: %i\nRegisters: \n", uc_strerror(m_err), sThreadHandler->GetCurrentThreadId());
        printf("    r0: %08X|%i\n    r1: %08X|%i\n    r2: %08X|%i\n    r3: %08X|%i\n    r4: %08X|%i\n"\
               "    r5: %08X|%i\n    r6: %08X|%i\n    r7: %08X|%i\n    r8: %08X|%i\n    r9: %08X|%i\n"\
               "   r10: %08X|%i\n   r11: %08X|%i\n   r12: %08X|%i\n"\
               "    sp: %08X\n    pc: %08X\n    lr: %08X\n",
                   r0, r0, r1, r1, r2, r2, r3, r3, r4, r4, r5, r5, r6, r6, r7, r7, r8, r8,
            r9, r9, r10, r10, r11, r11, r12, r12, sp, pc, lr);
    }


}

void interrupt_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{

    uint32_t r0, r1, r2, r3, sp, pc, lr;
    void* args[6] = { &r0, &r1, &r2, &r3, &sp, &pc };
    int regs[6] = { UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP, UC_ARM_REG_PC };


    uc_reg_read_batch(uc, regs, args, sizeof(args) / sizeof(void*));

    uint32_t SVC;
    uc_mem_read(uc, pc - 4, &SVC, 4);
    uc_mem_read(uc, sp, &lr, 4);


    SVC &= 0xFFFFF;

    uint32_t return_value = sSystemAPI->Call(static_cast<InterruptID>(SVC), Arguments());
    printf("    Caller: %08X\n    PC: %08X\n", lr - 4, pc);
    sp += 8;

    uc_reg_write(uc, UC_ARM_REG_R0, &return_value);
    uc_reg_write(uc, UC_ARM_REG_SP, &sp);
    uc_reg_write(uc, UC_ARM_REG_PC, &lr);
}

