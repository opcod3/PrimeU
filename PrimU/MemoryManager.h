#pragma once
#include "common.h"

#include <unordered_set>
#include <unordered_map>

#include "MemoryBlock.h"
#include "MemoryChunk.h"

class MemoryManager
{
public:
    static MemoryManager* GetInstance() { return !_instance ? _instance = new MemoryManager : _instance; }

    ErrorCode StaticAlloc(VirtPtr addr, size_t size, MemoryBlock** memoryBlock = nullptr);
    ErrorCode StaticFree(VirtPtr addr);

    ErrorCode DyanmicAlloc(size_t size);
    ErrorCode DynamicFree(VirtPtr addr);

    VirtPtr GetVirtualAddr(RealPtr realPtr);
    RealPtr GetRealAddr(VirtPtr virtPtr);
    bool isVAddrAllocated(VirtPtr virtPtr);

private:
    MemoryManager() { }
    ~MemoryManager() { delete _instance; }
    MemoryManager(MemoryManager const&) = delete;
    void operator=(MemoryManager const&) = delete;
    static MemoryManager* _instance;

    std::unordered_set<MemoryBlock*> _blocks;
};

#define sMemoryManager MemoryManager::GetInstance()
