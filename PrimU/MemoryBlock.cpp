#include "MemoryBlock.h"

MemoryChunk* MemoryBlock::VirtualAlloc(size_t size)
{
    if (!CanAllocate(size))
        return nullptr;

    uint32_t offset = GetSize() - _free;
    MemoryChunk* newChunk = new MemoryChunk(GetVAddr() + offset, GetRAddr() + offset, size);
    _chunks.insert(newChunk);

    _free -= size;

    return newChunk;
}

void MemoryBlock::VirtualFree(uint32_t vAddr)
{
    for (auto it = _chunks.begin(); it != _chunks.end(); ++it) {
        if ((*it)->GetVAddr() == vAddr) {
            _freed += (*it)->GetSize();
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
