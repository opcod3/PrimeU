#include "executable.h"

#include <elfio/elfio.hpp>

#include "memory.h"

Executable::Executable(char* path) : m_state(EXEC_LOAD_FAILED), m_mem(new Memory())
{
    ELFIO::elfio reader;

    if ( !reader.load(path) ) return;

    check(reader.get_class(), ELFCLASS32)
    check(reader.get_machine(), EM_ARM)

    ELFIO::Elf_Half segSize = reader.segments.size();

    for (int i = 0; i < segSize; i++)
    {
        const ELFIO::segment* seg = reader.segments[i];

        auto addr = seg->get_virtual_address();
        auto memSize = seg->get_memory_size();
        auto fileSize = seg->get_file_size();
        auto data = seg->get_data();

        if (!m_mem->alloc(memSize, addr)) return;
        if (!m_mem->cpy(reinterpret_cast<const uint8_t*>(data), fileSize, addr)) return;
    }

    m_mem->align_4k();
    m_entry = reader.get_entry();
    m_state = EXEC_LOADED;
}