#include "MemoryBlock.h"

MemoryChunk* MemoryBlock::VirtualAlloc(size_t size)
{
    if (!CanAllocate(size))
        return 0;

    uint32_t offset = GetSize() - _free;
    MemoryChunk* newChunk = new MemoryChunk(GetVAddr() + offset, GetRAddr() + offset, size);
    _free -= size;
}

void MemoryBlock::VirtualFree(uint32_t vAddr)
{
    for (auto it = _chunks.begin(); it != _chunks.end(); ++it) {
        if ((*it)->GetVAddr() == vAddr) {
            delete *it;
            _chunks.erase(it);
            break;
        }
    }
}

bool MemoryBlock::ContainsVAddr(VirtPtr vPtr) const
{
    for (auto chunk : _chunks) {
        if (chunk->ContainsVAddr(vPtr))
            return true;
    }

    return false;
}
