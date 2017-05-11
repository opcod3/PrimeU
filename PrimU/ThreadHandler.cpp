

#include "ThreadHandler.h"
#include "Thread.h"

ThreadHandler* ThreadHandler::_instance = nullptr;

int ThreadHandler::NewThread(VirtPtr start, uint32_t arg, uint8_t priority, size_t stackSize)
{
    Thread* newThread = new Thread(&_currentThread, start, arg, priority, stackSize);
    return newThread->GetId();
}

void ThreadHandler::LoadCurrentThreadState()
{
    if (_currentThread)
        _currentThread->LoadState();
}

/*
* Do not call function outside of a callback!!
*   as it may save an errant PC value because
*   of a UC engine bug
*/
void ThreadHandler::SaveCurrentThreadState()
{
    if (_currentThread)
        _currentThread->SaveState();
}

uint32_t ThreadHandler::GetCurrentThreadPC()
{
    if (_currentThread)
        return _currentThread->GetCurrentPC();
    return 0;
}

void ThreadHandler::SwitchThread()
{

    if (_currentThread != _currentThread->GetNextThread()) {
        _currentThread = _currentThread->GetNextThread();
        _currentThread->LoadState();
    }
}


int ThreadHandler::SetThreadPriority(int threadId, uint8_t priority)
{
    for (Thread* thread = nullptr; thread != _currentThread; thread = thread->GetNextThread()) {
        if (thread == nullptr)
            thread = _currentThread;

        if (thread->GetId() == threadId) {
            thread->SetPriority(priority);
            return 1;
        }
    }
    return NULL;
}


int ThreadHandler::GetCurrentThreadId() const
{
    return _currentThread->GetId();
}

uint32_t ThreadHandler::GetCurrentThreadQuantum() const
{
    return _currentThread->GetTimeQuantum();
}

bool ThreadHandler::CanCurrentThreadRun()
{
    return _currentThread->CanRun();
}

void ThreadHandler::InitCriticalSection(CriticalSection* criticalSection)
{
    static int32_t critId = 0;
    criticalSection = new (criticalSection) CriticalSection();
    criticalSection->SectionHandle = critId;

    critId++;
}


void ThreadHandler::CurrentThreadEnterCriticalSection(CriticalSection* criticalSection)
{
    _currentThread->EnterCriticalSection(criticalSection);
}

void ThreadHandler::CurrentThreadExitCriticalSection(CriticalSection* criticalSection)
{
    _currentThread->LeaveCriticalSection(criticalSection);
}

void ThreadHandler::CurrentThreadSleep(uint32_t time)
{
    _currentThread->Sleep(time);
}

