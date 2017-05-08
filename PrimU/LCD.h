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
    //LCD_MAGIC(uint32_t bufferPtr)
    //{
    //    SomeVal = 0x5850;
    //    x_res = 320;
    //    y_res = 240;
    //    pixel_bits = 16;
    //    unk2_640 = 640;
    //    unk0_2 = 2;
    //    unk1_0 = 8;
    //    window1_bufferstart = bufferPtr;
    //}
};
#pragma pack(pop)
//struct LCD_MAGIC_SUPER
//{
//    LCD_MAGIC* thisPtr; //VirtPTR
//    LCD_MAGIC magic;
//    LCD_MAGIC_SUPER(VirtPtr *addr, uint32_t bufferPtr) : magic(bufferPtr)
//    {
//        
//        ErrorCode err;
//        if ((err = sMemoryManager->DyanmicAlloc(addr, sizeof(LCD_MAGIC_SUPER))) != ERROR_OK)
//            __debugbreak();
//        //if ((err = sMemoryManager->DyanmicAlloc(reinterpret_cast<VirtPtr*>(&magic), sizeof(LCD_MAGIC))) != ERROR_OK)
//        //    __debugbreak();
//
//
//        //*__GET(LCD_MAGIC*, reinterpret_cast<VirtPtr>(magic)) = *(new LCD_MAGIC(bufferPtr));
//        thisPtr = reinterpret_cast<LCD_MAGIC*>(addr);
//    }
//};

struct LCDSubObject
{
    
};

struct LCD;

struct BUFINFO
{
    static int za;
    static LCD* lcd_a[5];
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
    uint8_t buffer[320 * 240 * 3];

    LCD()// : LcdMagic(sMemoryManager->GetVirtualAddr(reinterpret_cast<RealPtr>(&bufPTR)))
    {
        xRes = 320;
        yRes = 240;
        LcdMagic.SomeVal = 0x5850;
        LcdMagic.x_res = 320;
        LcdMagic.y_res = 240;
        LcdMagic.pixel_bits = 16;
        LcdMagic.unk2_640 = 640;
        LcdMagic.unk0_2 = 2;
        LcdMagic.unk1_0 = 8;
        LcdMagic.window1_bufferstart = sMemoryManager->GetVirtualAddr(reinterpret_cast<RealPtr>(&buffer));

        //(*(reinterpret_cast<uint32_t*>(&LcdMagic) + 0x10)) = LcdMagic.window1_bufferstart;

        LCDMagicPtr = reinterpret_cast<LCD_MAGIC*>(sMemoryManager->GetVirtualAddr(reinterpret_cast<RealPtr>(&LcdMagic)));
        itself = reinterpret_cast<LCD*>(sMemoryManager->GetVirtualAddr(reinterpret_cast<RealPtr>(this)));
        //buffer[0] = 123;

        if (LCDMagicPtr == 0)
            __debugbreak();

        for (int i = 0; i < 320 * 240; i++) {
            buffer[i] = i;
        }

        BUFINFO::lcd_a[BUFINFO::za] = this;
        BUFINFO::za = BUFINFO::za < 5 ? BUFINFO::za + 1 : 0;
    }
};

#pragma pack(pop)

