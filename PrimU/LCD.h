#pragma once
#include <cstdint>

struct LCD_MAGIC
{
    uint16_t SomeVal;
    uint16_t x_res;
    uint16_t y_res;
    uint16_t pixel_bits;
    uint16_t unk2_640;
    uint16_t unk0_2;
    uint16_t unk1_0;
    uint32_t window1_bufferstart;
    LCD_MAGIC(uint32_t bufferPtr)
    {
        SomeVal = 0x5850;
        x_res = 320;
        y_res = 240;
        pixel_bits = 16;
        unk2_640 = 640;
        unk0_2 = 2;
        unk1_0 = 0;
        window1_bufferstart = bufferPtr;
    }
};

struct LCD_MAGIC_SUPER
{
    LCD_MAGIC* thisPtr;
    LCD_MAGIC magic;
    LCD_MAGIC_SUPER(uint32_t bufferPtr) : magic(bufferPtr)
    {
        thisPtr = &magic;
    }
};

struct LCDSubObject
{
    
};

struct LCD
{
    LCD_MAGIC* LCDMagicPtr;
    uint32_t secondBufferstart;
    uint32_t bufferSize;
    uint32_t unk[0x1E];
    LCDSubObject* LCDSubObjectPtr;
    uint16_t xRes;
    uint16_t yRes;
    uint32_t unk0;
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    void* enterCriticalSecFunc;
    void* leaveCriticalSecFunc;
    uint8_t unk4[92];
    LCD_MAGIC LCD_MAGIC;
};
