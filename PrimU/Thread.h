#ifndef THREAD_H
#define THREAD_H

#include "executor.h"
#include <chrono>

class ThreadState
{
public:
    ThreadState(VirtPtr startPtr, VirtPtr stackPtr, uint32_t arg) :_r0(arg), _sp(stackPtr), _pc(startPtr)
    {
        uc_context_alloc(sExecutor->GetUcInstance(), &_state);
    }

    ~ThreadState() { uc_free(&_state); }

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

struct CriticalSection;

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

        VirtPtr stackPtrStart;
        sMemoryManager->DyanmicAlloc(&_stackAddr, _stackSize);
        stackPtrStart = sMemoryManager->GetAllocSize(_stackAddr) - 1 + _stackAddr;
        printf("Thread [%i] stack starts at %08X and ends at %08X\n", _id, stackPtrStart, _stackAddr);

        _state = new ThreadState(start, stackPtrStart, arg);

        SetPriority(priority);
        _requested = nullptr;
    }

    ~Thread()
    {
        sMemoryManager->DynamicFree(_stackAddr);
        delete _state;
    }

    void SetNextThread(Thread* nextThread) { _nextThread = nextThread; }
    Thread* GetNextThread() const { return _nextThread; }

    void SaveState();
    void LoadState();

    void EnterCriticalSection(CriticalSection* criticalSection);
    void LeaveCriticalSection(CriticalSection* criticalSection);

    void Sleep(uint32_t time);

    void SetPriority(uint8_t priority) { _priority = priority; }

    uint32_t GetTimeQuantum();
    uint32_t GetCurrentPC() const { return _state->GetCurrentAddr(); }
    uint8_t GetPriority() const { return _priority; }

    bool CanRun();
    int GetId() const { return _id; }

private:
    static int GenerateUniqueId();

    ThreadState* _state;

    uint8_t _priority;
    int _id;
    size_t _stackSize = 0x2000;
    VirtPtr _stackAddr;

    Thread* _nextThread;
    CriticalSection* _requested = nullptr;

    std::chrono::high_resolution_clock::time_point _sleepEnd;
    bool _isSleeping = false;
};

#endif
