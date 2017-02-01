#pragma once
#include "common.h"

class MemoryChunk
{
public:
    MemoryChunk(VirtPtr vAddr, RealPtr rAddr, size_t size) : _virtualAddress(vAddr), _realAddress(rAddr), _size(size) { }
    //~MemoryChunk();

    VirtPtr GetVAddr() const { return _virtualAddress; }
    RealPtr GetRAddr() const { return _realAddress; }
    size_t GetSize() const { return _size; }
    virtual bool ContainsVAddr(VirtPtr vPtr) const;

    VirtPtr GetVAddr(RealPtr rPtr);
    RealPtr GetRAddr(VirtPtr vPtr);

private:
    VirtPtr _virtualAddress;
    RealPtr _realAddress;
    size_t _size;
};

