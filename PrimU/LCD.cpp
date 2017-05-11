#include "LCD.h"

LCDHandler* LCDHandler::_instance = nullptr;

LCDHandler::LCDHandler()
{
    InitActiveLCD();
}

LCDHandler::~LCDHandler()
{
    _activeLCD->~LCD();
    DeleteActiveLCD();
    delete _instance;
}


void LCDHandler::InitActiveLCD()
{
    ErrorCode err = ERROR_OK;
    VirtPtr lcd, virt_ptrBuf;

    //allocate memory for LCD object inside VM
    if ((err = sMemoryManager->DyanmicAlloc(&lcd, sizeof(LCD))) != ERROR_OK)
        __debugbreak();

    // Create LCD object in allocated memory
    _activeLCD = reinterpret_cast<LCD*>(sMemoryManager->GetRealAddr(lcd));
    _activeLCD = new (_activeLCD) LCD();

    // Allocate a 4 bytes for a pointer to the LCD object
    if ((err = sMemoryManager->DyanmicAlloc(&_activeLCDPtr, 0x4)) != ERROR_OK)
        __debugbreak();

    *__GET(uint32_t*, _activeLCDPtr) = reinterpret_cast<uint32_t>(_activeLCD->LCDMagicPtr);
}

void LCDHandler::DeleteActiveLCD()
{
    sMemoryManager->DynamicFree(sMemoryManager->GetVirtualAddr(reinterpret_cast<RealPtr>(_activeLCD)));
    sMemoryManager->DynamicFree(_activeLCDPtr);
}


VirtPtr LCDHandler::GetActiveLCDPtr() const
{
    return _activeLCDPtr;
}

LCD::LCD()
{
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

        for (int i = 0; i < 320 * 240 * 3; i++) {
            buffer[i] = 0xFF;
        }
    }
}
