#ifndef EXECUTOR_H
#define EXECUTOR_H

#include  "stdafx.h"
#include  <unicorn/unicorn.h>
#include "executable.h"
#include "memory.h"
#include "MemoryManager.h"

enum InterruptID : uint32_t;
struct InterruptHandle;
class Thread;

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

class Executor
{
public:
    static Executor* get_instance() { return (!m_instance) ? m_instance = new Executor : m_instance; }
    ~Executor() { delete m_instance; }

    bool initialize(Executable* exec);
    void execute();
    uc_err get_last_error() { return m_err; }
    uc_engine* GetUcInstance() { return m_uc; }

    friend void interrupt_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
    friend void code_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
    int NewThread(VirtPtr start, uint32_t arg = 0, uint8_t priority = THREAD_PRIORITY_NORMAL, size_t stackSize = 0x2000);
    int GetCurrentThreadId() const;

private:
    Executor() : m_uc(nullptr)
    {
        init_interrupts_();
    }
    void init_interrupts_();

    static Executor* m_instance;
    uc_engine* m_uc;
    uc_hook m_interrupt_hook;
    uc_err m_err;

    Executable* m_exec;
    Memory* m_stack;
    uint32_t m_sp;

    Memory* m_dynamic;
    Memory* m_LCD;

    Thread* _currentThread = nullptr;

public:
    Executor(Executor const&) = delete;
    void operator=(Executor const&) = delete;

private:
    std::map<InterruptID, InterruptHandle*> m_interrupts;
    std::map<uint32_t, size_t>  m_dynamic_free;
};

#define sExecutor Executor::get_instance()

class ThreadState
{
public:
    ThreadState(VirtPtr startPtr, VirtPtr stackPtr, uint32_t arg) :_r0(arg), _sp(stackPtr), _pc(startPtr)
    {
        uc_context_alloc(sExecutor->GetUcInstance(), &_state);
    }

    void LoadState();
    void SaveState();
    uint32_t GetCurrentAddr() const { return _pc; }
private:
    uint32_t _r0 = 0;
    uint32_t _r1 = 0;
    uint32_t _r2 = 0;
    uint32_t _r3 = 0;
    uint32_t _r4 = 0;
    uint32_t _r5 = 0;
    uint32_t _r6 = 0;
    uint32_t _r7 = 0;
    uint32_t _r8 = 0;
    uint32_t _r9 = 0;
    uint32_t _r10 = 0;
    uint32_t _r11 = 0;
    uint32_t _r12 = 0;
    uint32_t _sp = 0;
    uint32_t _pc = 0;
    uint32_t _lr = 0;

    uc_context* _state;
    bool _isNewThread = true;

    uc_arm_reg _regs[16] =
    {
        UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3,
        UC_ARM_REG_R4, UC_ARM_REG_R5, UC_ARM_REG_R6, UC_ARM_REG_R7,
        UC_ARM_REG_R8, UC_ARM_REG_R9, UC_ARM_REG_R10, UC_ARM_REG_R11,
        UC_ARM_REG_R12, UC_ARM_REG_SP, UC_ARM_REG_LR, UC_ARM_REG_PC
    };

    void* _args[16] = { &_r0, &_r1, &_r2, &_r3, &_r4, &_r5, &_r6, &_r7, &_r8, &_r9, &_r10, &_r11, &_r12, &_sp, &_lr, &_pc };

};

class Thread
{
public:
    Thread(Thread** currentThread, VirtPtr start, uint32_t arg, uint8_t priority, size_t stackSize) :_id(GenerateUniqueId())
    {
        if (*currentThread == nullptr) {
            SetNextThread(this);
            *currentThread = this;
        }
        else {
            SetNextThread((*currentThread)->GetNextThread());
            (*currentThread)->SetNextThread(this);
        }

        if (stackSize != 0)
            _stackSize = stackSize;

        VirtPtr stackPtrEnd, stackPtrStart;
        sMemoryManager->DyanmicAlloc(&stackPtrEnd, _stackSize);
        stackPtrStart = sMemoryManager->GetAllocSize(stackPtrEnd) - 1 + stackPtrEnd;
        printf("Thread [%i] stack starts at %08X and ends at %08X\n", _id, stackPtrStart, stackPtrEnd);
       
        _state = new ThreadState(start, stackPtrStart, arg);      

        SetPriority(priority);
    }

    void SetNextThread(Thread* nextThread) { _nextThread = nextThread; }
    Thread* GetNextThread() const { return _nextThread; }

    void SetPriority(uint8_t priority) { _priority = priority; }
    uint8_t GetPriority() const { return _priority; }

    void SaveState();
    void LoadState();

    int GetId() const { return _id; }
    uint32_t GetCurrentPC() const { return _state->GetCurrentAddr(); }

private:
    static int GenerateUniqueId();

    ThreadState* _state;

    uint8_t _priority;
    int _id;
    size_t _stackSize = 0x2000;

    Thread* _nextThread;
};

#endif

