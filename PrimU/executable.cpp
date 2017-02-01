#include "executable.h"

#include <elfio/elfio.hpp>

#include "MemoryManager.h"


ErrorCode Executable::Load()
{
    ELFIO::elfio reader;

    if (!reader.load(_path)) return ERROR_LOADER_READER_FAIL;

    __check(reader.get_class(), ELFCLASS32, ERROR_LOADER_INCORRECT_ATTRIBUTE);
    __check(reader.get_machine(), EM_ARM, ERROR_LOADER_INCORRECT_ATTRIBUTE);

    ELFIO::Elf_Half segSize = reader.segments.size();

    for (int i = 0; i < segSize; i++)
    {
        const ELFIO::segment* seg = reader.segments[i];

        _address = seg->get_virtual_address();
        _size = seg->get_memory_size();
        auto fileSize = seg->get_file_size();
        auto data = seg->get_data();

        ErrorCode err;
        MemoryBlock* memBlock;
        __check((err = sMemoryManager->StaticAlloc(_address, _size, &memBlock)), ERROR_OK, err);

        RealPtr addr = memBlock->GetRAddr();
        if (memcpy_s(addr, _size, data, fileSize)) {
            sMemoryManager->StaticFree(_address);
            return ERROR_GENERIC;
        }
    }

    _entry = reader.get_entry();
    _state = EXEC_LOADED;

    return ERROR_OK;
}
