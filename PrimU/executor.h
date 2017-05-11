#ifndef EXECUTOR_H
#define EXECUTOR_H

#include  "stdafx.h"
#include  <unicorn/unicorn.h>
#include "executable.h"
#include "memory.h"
#include "MemoryManager.h"

enum InterruptID : uint32_t;

class Thread;
struct InterruptHandle;

class Executor
{
public:
    static Executor* get_instance() { return (!m_instance) ? m_instance = new Executor : m_instance; }
    ~Executor() { delete m_instance; }

    bool Initialize(Executable* exec);
    void Execute();
    bool Cleanup();

    uc_err GetLastError() { return m_err; }
    uc_engine* GetUcInstance() { return m_uc; }

    friend void interrupt_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);

private:
    Executor() : m_uc(nullptr)
    {
    }
    bool InitInterrupts();

    static Executor* m_instance;
    uc_engine* m_uc;
    uc_hook m_interrupt_hook, _codeHook;
    uc_err m_err;

    Executable* m_exec;
    Memory* m_stack;
    uint32_t m_sp;

    Memory* m_dynamic;
    Memory* m_LCD;



public:
    Executor(Executor const&) = delete;
    void operator=(Executor const&) = delete;

private:
    std::map<InterruptID, InterruptHandle*> m_interrupts;
};

#define sExecutor Executor::get_instance()

#endif

