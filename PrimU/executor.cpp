#include "executor.h"


#include "memory.h"
#include "executable.h"
#include "interrupts.h"
#include "InterruptHandler.h"
#include <valarray>

#include "handlers.h"
#include "SystemAPI.h"

Executor* Executor::m_instance = nullptr;

#define STACK_MIN 0x20000000

#define callAndcheckError(f) m_err = f; if (m_err != UC_ERR_OK) return false
#define DEFINE_INTERRUPT(id, s, n, c) m_interrupts.insert(std::pair<InterruptID, InterruptHandle*>(id, new InterruptHandle(id, s, c, n)))

static void hook_code(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{
    printf(">>> Tracing instruction at 0x%llX, instruction size = 0x%x\n", address, size);

    uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, pc, lr;
    void* args[16] = { &r0, &r1, &r2, &r3, &r4, &r5, &r6, &r7, &r8, &r9, &r10, &r11, &r12, &sp, &lr, &pc };
    int regs[16] = { UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_R4, UC_ARM_REG_R5, UC_ARM_REG_R6,
        UC_ARM_REG_R7, UC_ARM_REG_R8, UC_ARM_REG_R9, UC_ARM_REG_R10, UC_ARM_REG_R11, UC_ARM_REG_R12, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC };
    uc_reg_read_batch(uc, regs, args, 16);

    pc += 4; // In ARM state, the value of the PC is the address of the current instruction plus 8 bytes

    printf("Registers: \n");
    printf("    r0: %08X|%i\n    r1: %08X|%i\n    r2: %08X|%i\n    r3: %08X|%i\n    r4: %08X|%i\n"\
           "    r5: %08X|%i\n    r6: %08X|%i\n    r7: %08X|%i\n    r8: %08X|%i\n    r9: %08X|%i\n"\
           "   r10: %08X|%i\n   r11: %08X|%i\n   r12: %08X|%i\n"\
           "    sp: %08X\n    pc: %08X\n    lr: %08X\n",
           r0, r0, r1, r1, r2, r2, r3, r3, r4, r4, r5, r5, r6, r6, r7, r7, r8, r8,
           r9, r9, r10, r10, r11, r11, r12, r12, sp, pc, lr);
}

bool Executor::initialize(Executable* exec)
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

    callAndcheckError(uc_hook_add(m_uc, &m_interrupt_hook, UC_HOOK_INTR, interrupt_hook, this, 0, 1));

    MemoryBlock* stackBlock;
    __check(sMemoryManager->StaticAlloc(MEM_STACK, MEM_STACK_SIZE, &stackBlock), ERROR_OK, false);


    m_sp = stackBlock->GetVAddr() + stackBlock->GetSize();
    callAndcheckError(uc_reg_write(m_uc, UC_ARM_REG_SP, &m_sp));

    __check(sMemoryManager->StaticAlloc(LCD_REGISTER, LCD_REGISTER_SIZE), ERROR_OK, false);

    //uc_hook trace2;
    //uc_hook_add(m_uc, &trace2, UC_HOOK_CODE, hook_code, NULL, 0x30108000, 0x3010FFFF);

    return true;
}

void Executor::init_interrupts_()
{

}


void Executor::execute()
{
    uint32_t ins = 0x0;
    uint32_t SP0, PC0;
    PC0 = m_exec->get_entry();
    uc_reg_read(m_uc, UC_ARM_REG_SP, &SP0);
    printf("Starting execution at 0x%X\n\n", PC0);
    while (m_err == UC_ERR_OK)
    {
        m_err = uc_emu_start(m_uc, PC0, 0, 0, ins);

        uc_reg_read(m_uc, UC_ARM_REG_PC, &PC0);
        uc_reg_read(m_uc, UC_ARM_REG_SP, &SP0);
        //PC0;

        printf("\nExecuted 0x%X instructions\n PC: %8X | SP: %8X\n", ins, PC0, SP0);

        //_sleep(100);

        //if ((SP0 == SP1) && (PC0 == PC1))
        //{
        //    printf("Not moving, broke loop\n");
            //break;
        //}
    }
    
    if (m_err != UC_ERR_OK) { 

        uint32_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, sp, pc, lr;
        void* args[16] = { &r0, &r1, &r2, &r3, &r4, &r5, &r6, &r7, &r8, &r9, &r10, &r11, &r12, &sp, &lr, &pc };
        int regs[16] = { UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_R4, UC_ARM_REG_R5, UC_ARM_REG_R6,
            UC_ARM_REG_R7, UC_ARM_REG_R8, UC_ARM_REG_R9, UC_ARM_REG_R10, UC_ARM_REG_R11, UC_ARM_REG_R12, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC };
        uc_reg_read_batch(sExecutor->GetUcInstance(), regs, args, 16);

        pc += 4; // In ARM state, the value of the PC is the address of the current instruction plus 8 bytes

        printf("Execution aborted on error: %s!\nRegisters: \n", uc_strerror(m_err));
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

    /*if (err != UC_ERR_OK)
        printf("COULD NOT READ R0");
    else*/
    //    printf("+[0x%05X] Interrupt called\n    PC=%08X\n    LR=%08X\n", SVC, pc, LR);

    //_sleep(1500);

}