#pragma once
#include <cstdint>
#include "MemoryManager.h"

#pragma pack(push)
#pragma pack(1)

struct LCD_MAGIC
{
    uint16_t SomeVal;
    uint16_t x_res;
    uint16_t y_res;
    uint16_t pixel_bits;
    uint16_t unk2_640;
    uint16_t unk0_2;
    uint32_t unk1_0;
    uint32_t window1_bufferstart;
};
#pragma pack(pop)

struct LCDSubObject
{  
};

#pragma pack(push)
#pragma pack(1)

struct LCD // BLIGLCD
{
    LCD_MAGIC* LCDMagicPtr;
    uint32_t secondBufferstart;
    uint32_t bufferSize;
    LCD* itself;
    uint32_t unk[0x70];
    LCDSubObject* LCDSubObjectPtr;
    uint16_t xRes;
    uint16_t yRes;
    uint32_t unk0;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t unk4;
    void* enterCriticalSecFunc;
    void* leaveCriticalSecFunc;
    uint8_t unk5[92];
    LCD_MAGIC LcdMagic;
    uint32_t buffer[320 * 240 * 3];

    LCD();
    ~LCD() { }
};

#pragma pack(pop)

class LCDHandler
{
public:
    static LCDHandler* GetInstance() { return !_instance ? _instance = new LCDHandler : _instance; }

    VirtPtr GetActiveLCDPtr() const;
private:
    LCDHandler();
    ~LCDHandler();
    LCDHandler(LCDHandler const&) = delete;
    void operator=(LCDHandler const&) = delete;
    static LCDHandler* _instance;

    void InitActiveLCD();
    void DeleteActiveLCD();

    VirtPtr _activeLCDPtr = NULL;
    LCD* _activeLCD = nullptr;
};

#define sLCDHandler LCDHandler::GetInstance()