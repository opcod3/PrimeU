#include "MemoryManager.h"

#include "MemoryBlock.h"
#include "executor.h"

MemoryManager* MemoryManager::_instance = nullptr;

ErrorCode MemoryManager::StaticAlloc(VirtPtr addr, size_t size, MemoryBlock** memoryBlock)
{
    if (isVAddrAllocated(addr))
        return ERROR_MEM_ALREADY_ALLOCATED;

    uint32_t pageCount = size % PAGE_SIZE == 0 ? size / PAGE_SIZE : size / PAGE_SIZE + 1;
    RealPtr realMemory = reinterpret_cast<RealPtr>(calloc(pageCount, PAGE_SIZE));
    size_t pageAlignedSize = pageCount * PAGE_SIZE;

    if (realMemory == nullptr)
        return ERROR_MEM_ALLOC_FAIL;

    auto err = uc_mem_map_ptr(sExecutor->GetUcInstance(), addr, pageAlignedSize, UC_PROT_ALL, realMemory);
    if (err != UC_ERR_OK) {
        free(realMemory);
        return ERROR_UC_MAP;
    }

    MemoryBlock* newBlock = new MemoryBlock(addr, realMemory, pageCount);
    newBlock->VirtualAlloc(pageAlignedSize);

    _blocks.insert(newBlock);

    if (memoryBlock != nullptr)
        *memoryBlock = newBlock;

    return ERROR_OK;
}

ErrorCode MemoryManager::StaticFree(VirtPtr addr)
{
    for (auto it = _blocks.begin(); it != _blocks.end(); ++it) {
        auto block = *it;
        if (block->GetVAddr() == addr) {
            block->VirtualFree(addr);
            if (!block->CanFree())
                return ERROR_MEM_STATIC_NOT_FREEABLE;

            if (uc_mem_unmap(sExecutor->GetUcInstance(), addr, block->GetVAddr()) != UC_ERR_OK) {
                return ERROR_UC_UNMAP;
            }

            free(block->GetRAddr());
            _blocks.erase(it);
            delete block;
            return ERROR_OK;
        }
    }

    return ERROR_MEM_ADDR_NOT_ALLOCATED;
}

bool MemoryManager::isVAddrAllocated(VirtPtr virtPtr)
{
    for (auto block : _blocks) {
        if (block->ContainsVAddr(virtPtr))
            return true;
    }

    return false;
}

RealPtr MemoryManager::GetRealAddr(VirtPtr virtPtr)
{
    for (auto block : _blocks)
    {
        if (block->ContainsVAddr(virtPtr)) {
            return block->GetRAddr(virtPtr);
        }
    }

    return nullptr;
}

