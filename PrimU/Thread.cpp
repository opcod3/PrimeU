#include "Thread.h"
#include "ThreadHandler.h"

int Thread::GenerateUniqueId()
{
    static int currentInt = -1;
    currentInt++;
    return currentInt;
}

void Thread::LoadState()
{
    _state->LoadState();
}

/*
* Do not call function outside of a callback!!
*   as it may save an errant PC value because
*   of a UC engine bug
*/
void Thread::SaveState()
{
    _state->SaveState();
}

void ThreadState::LoadState()
{
    if (!_isNewThread) {
        uc_context_restore(sExecutor->GetUcInstance(), _state);
    }

    uc_reg_write_batch(sExecutor->GetUcInstance(), reinterpret_cast<int*>(_regs), reinterpret_cast<void**>(_args), 16);
}

/*
* Do not call function outside of a callback!!
*   as it may save an errant PC value because
*   of a UC engine bug
*/
void ThreadState::SaveState()
{
    uc_context_save(sExecutor->GetUcInstance(), _state);
    uc_reg_read_batch(sExecutor->GetUcInstance(), reinterpret_cast<int*>(_regs), reinterpret_cast<void**>(_args), 16);

    size_t thumb;
    uc_query(sExecutor->GetUcInstance(), UC_QUERY_MODE, &thumb);
    _pc += (thumb & UC_MODE_THUMB) ? 1 : 0;

    if (_isNewThread)
        _isNewThread = false;
}


uint32_t Thread::GetTimeQuantum()
{
    return 500 - _priority;
}

void Thread::EnterCriticalSection(CriticalSection* criticalSection)
{
#ifdef _DEBUG
    // If this triggers something is seriously broken
    if (_requested != nullptr)
        __debugbreak();
#endif

    // Critical section not initialized
    if (criticalSection == nullptr)
        return;

    criticalSection->ContentionCount++;
    _requested = criticalSection;
}

void Thread::LeaveCriticalSection(CriticalSection* criticalSection)
{
    // Critical section not initialized
    if (criticalSection == nullptr || _requested == nullptr)
        return;

#ifdef _DEBUG
    // If this triggers something is seriously broken
    if (_requested != criticalSection)
        __debugbreak();
#endif

    _requested->ContentionCount--;
    _requested = nullptr;
}



bool Thread::CanRun()
{
    if (_isSleeping) {
        if (_sleepEnd > std::chrono::high_resolution_clock::now())
            return false;

        _isSleeping = false;
    }

    if (_requested == nullptr)
        return true;

    // If handle is free Lock handle and take ownership
    if (!_requested->isLocked) {
        _requested->isLocked = 1;
        _requested->ContentionCount--;
        _requested->ownerHandle = GetId();
        return true;
    }

    if (_requested->ownerHandle == GetId())
        return true;

    return false;
}

void Thread::Sleep(uint32_t time)
{
    _isSleeping = true;
    _sleepEnd = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(time);
}
