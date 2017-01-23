#include "memory.h"

Memory::Memory() { }

Memory::~Memory()
{
    free(m_mem);
}


bool Memory::alloc(size_t size, uint32_t offset, uint8_t *buf)
{
    if (!size) return false;

    size_t newSize;
    
    // Memory not initialized yet
    if (m_mem == nullptr)
    {
        m_offset = offset;
        newSize = size;
    }
    // No offset between end of current memory and block to alloc
    else if (!offset || offset == m_offset + m_mem_size)
    {
        newSize = m_mem_size + size;
    }
    else 
    {
        if (offset < m_offset + size)
            return false;

        newSize = size + (offset - m_offset);
    }

    uint8_t* newMem = reinterpret_cast<uint8_t*>(malloc(newSize));
    if (!newMem) return false;

    if (m_mem != nullptr)
        memcpy_s(newMem, newSize, m_mem, m_mem_size);

    if (buf != nullptr)
        memcpy_s(newMem + m_mem_size, newSize - m_mem_size, buf, size);
    else
        memset(newMem + m_mem_size, 0, newSize - m_mem_size);

    if (m_mem != nullptr) free(m_mem);

    m_mem_size = newSize;
    m_mem = newMem;
    return true;
}


bool Memory::cpy(const uint8_t* dat, size_t size, uint32_t addr)
{
    if (addr == 0)
        addr = m_offset;


    if (size == NULL ||
        addr > m_offset + m_mem_size ||
        addr < m_offset ||
        addr + size > m_offset + m_mem_size) return false;

    memcpy_s(m_mem + (addr - m_offset), m_mem_size - (addr - m_offset), dat, size);

    return true;
}

bool Memory::align_4k()
{
    auto leftover = m_mem_size % 0x1000;

    if (leftover == 0)
        return true;

    alloc(0x1000 - leftover);
    return true;
}



