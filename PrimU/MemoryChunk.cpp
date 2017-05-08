#include "MemoryChunk.h"



RealPtr MemoryChunk::GetRAddr(VirtPtr vPtr)
{
    if (_virtualAddress > vPtr ||
        vPtr > _virtualAddress + _size)
        return nullptr;

    return _realAddress + (vPtr - _virtualAddress);
}

VirtPtr MemoryChunk::GetVAddr(RealPtr rPtr)
{
    if (_realAddress > rPtr ||
        rPtr > _realAddress + _size)
        return 0;

    return _virtualAddress + (rPtr - _realAddress);
}

bool MemoryChunk::ContainsVAddr(VirtPtr vPtr) const
{
    if (vPtr < GetVAddr() || vPtr >= GetVAddr() + GetSize())
        return false;

    return true;
}

bool MemoryChunk::ContainsRAddr(RealPtr rPtr) const
{
    if (rPtr < GetRAddr() || rPtr >= GetRAddr() + GetSize())
        return false;

    return true;
}
