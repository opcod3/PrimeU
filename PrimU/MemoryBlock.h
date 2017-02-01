#pragma once
#include  "common.h"

#include <unordered_set>

#include "MemoryChunk.h"


class MemoryBlock : public MemoryChunk
{
public:
    MemoryBlock(VirtPtr vAddr, RealPtr rAddr, uint32_t pageCount) : MemoryChunk(vAddr, rAddr, pageCount * PAGE_SIZE),
    _pageCount(pageCount) ,_free(GetSize()), _freed(0) { }


    size_t GetFree() { return _free; }
    uint32_t GetPageCount() { return _pageCount; }
    bool CanAllocate(size_t size) { return _free >= size; }
    bool CanFree() { return _freed == GetSize() - _free; }

    MemoryChunk* VirtualAlloc(size_t size);
    void VirtualFree(uint32_t vAddr);

    bool ContainsVAddr(VirtPtr vPtr) const override;

private:
    uint32_t _pageCount;
    size_t _free;
    size_t _freed;
    std::unordered_set<MemoryChunk*> _chunks;
};

