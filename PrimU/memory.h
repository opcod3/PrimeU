#ifndef MEMORY_H
#define MEMORY_H

#include "stdafx.h"

class Memory
{
public:
    Memory();
    ~Memory();

public:
    bool alloc(size_t size, uint32_t offset = 0, uint8_t* buf = nullptr);
    bool cpy(const uint8_t* dat, size_t size, uint32_t addr = NULL);
    bool align_4k();

    uint32_t get_size() { return m_mem_size; }
    uint32_t get_offset() { return m_offset; }
    uint8_t* get_data() { return m_mem; }

    template<typename T>
    T* get(uint32_t addr) { return reinterpret_cast<T*>(read_(addr)); }

private:

private:

    // Size is used for bounds checking only
    uint8_t* read_(uint32_t addr)
    {
        if (addr < m_offset ||
            addr > m_offset + m_mem_size)
                return nullptr;

        return m_mem + (addr - m_offset);
    }


    uint8_t* m_mem = nullptr;

    uint32_t m_offset = 0;
    size_t m_mem_size = 0;
};



#endif