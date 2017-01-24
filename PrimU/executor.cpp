#include "executor.h"


#include "memory.h"
#include "executable.h"
#include "interrupts.h"
#include "interrupt_handle.h"
#include <valarray>

#include "handlers.h"

Executor* Executor::m_instance = nullptr;

#define STACK_MIN 0x20000000

#define callAndcheckError(f) m_err = f; if (m_err != UC_ERR_OK) return false
#define DEFINE_INTERRUPT(id, s, n, c) m_interrupts.insert(std::pair<InterruptID, InterruptHandle*>(id, new InterruptHandle(id, s, c, n)))

bool Executor::initialize(Executable* exec)
{
    if (!exec)
        return false;

    m_exec = exec;

    if (!m_uc)
    {
        m_err = uc_open(UC_ARCH_ARM, UC_MODE_ARM, &m_uc);
        if (m_err != UC_ERR_OK) return false;
    }

    Memory* mem = exec->get_mem();

    if (!mem)
        return false;

    callAndcheckError(uc_mem_map_ptr(m_uc, mem->get_offset(), mem->get_size(), UC_PROT_ALL, mem->get_data()));
    //callAndcheckError(uc_mem_write(m_uc, mem->get_offset(), mem->get_data(), mem->get_size()))
    callAndcheckError(uc_hook_add(m_uc, &m_interrupt_hook, UC_HOOK_INTR, interrupt_hook, this, 0, 1));

    m_stack = new Memory;
    m_stack->alloc(0x05000000, 0x20000000); //Allocate 256mb of stack

    callAndcheckError(uc_mem_map_ptr(m_uc, m_stack->get_offset(), m_stack->get_size(), UC_PROT_READ | UC_PROT_WRITE, m_stack->get_data()));


    m_sp = m_stack->get_offset() + m_stack->get_size();
    callAndcheckError(uc_reg_write(m_uc, UC_ARM_REG_SP, &m_sp));


    m_dynamic = new Memory;
    m_dynamic->alloc(0x05000000, 0x00001000);
    callAndcheckError(uc_mem_map_ptr(m_uc, m_dynamic->get_offset(), m_dynamic->get_size(), UC_PROT_READ | UC_PROT_WRITE, m_dynamic->get_data()));

    m_dynamic_free.insert(std::pair<uint32_t, size_t>(m_dynamic->get_offset(), m_dynamic->get_size()));


    m_LCD = new Memory;
    m_LCD->alloc(320*240*4, 0x45000000);
    callAndcheckError(uc_mem_map_ptr(m_uc, m_LCD->get_offset(), m_LCD->get_size(), UC_PROT_READ | UC_PROT_WRITE, m_LCD->get_data()));

    return true;
}

void Executor::init_interrupts_()
{
    DEFINE_INTERRUPT(SDKLIB_OSCreateThread,               HANDLE_NAMEONLY, "OSCreateThread",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSTerminateThread,            HANDLE_NAMEONLY, "OSTerminateThread",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSSetThreadPriority,          HANDLE_NAMEONLY, "OSSetThreadPriority",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSGetThreadPriority,          HANDLE_NAMEONLY, "OSGetThreadPriority",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSSuspendThread,              HANDLE_NAMEONLY, "OSSuspendThread",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSResumeThread,               HANDLE_NAMEONLY, "OSResumeThread",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSWakeUpThread,               HANDLE_NAMEONLY, "OSWakeUpThread",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSExitThread,                 HANDLE_NAMEONLY, "OSExitThread",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSSleep,                      HANDLE_NAMEONLY, "OSSleep",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSCreateSemaphore,            HANDLE_NAMEONLY, "OSCreateSemaphore",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSWaitForSemaphore,           HANDLE_NAMEONLY, "OSWaitForSemaphore",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSReleaseSemaphore,           HANDLE_NAMEONLY, "OSReleaseSemaphore",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSCloseSemaphore,             HANDLE_NAMEONLY, "OSCloseSemaphore",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSCreateEvent,                HANDLE_IMPLEMENTED, "OSCreateEvent",             OSCreateEvent);
    DEFINE_INTERRUPT(SDKLIB_OSWaitForEvent,               HANDLE_NAMEONLY, "OSWaitForEvent",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSSetEvent,                   HANDLE_IMPLEMENTED, "OSSetEvent",                OSSetEvent);
    DEFINE_INTERRUPT(SDKLIB_OSResetEvent,                 HANDLE_NAMEONLY, "OSResetEvent",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSCloseEvent,                 HANDLE_NAMEONLY, "OSCloseEvent",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSInitCriticalSection,        HANDLE_IMPLEMENTED, "OSInitCriticalSection",     OSInitCriticalSection);
    DEFINE_INTERRUPT(SDKLIB_OSEnterCriticalSection,       HANDLE_NAMEONLY, "OSEnterCriticalSection",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSLeaveCriticalSection,       HANDLE_NAMEONLY, "OSLeaveCriticalSection",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSDeleteCriticalSection,      HANDLE_NAMEONLY, "OSDeleteCriticalSection",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSSetLastError,               HANDLE_NAMEONLY, "OSSetLastError",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSGetLastError,               HANDLE_NAMEONLY, "OSGetLastError",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSCreateMsgQue,               HANDLE_NAMEONLY, "OSCreateMsgQue",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSPostMsgQue,                 HANDLE_NAMEONLY, "OSPostMsgQue",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSSendMsgQue,                 HANDLE_NAMEONLY, "OSSendMsgQue",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSPeekMsgQue,                 HANDLE_NAMEONLY, "OSPeekMsgQue",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSGetMsgQue,                  HANDLE_NAMEONLY, "OSGetMsgQue",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_OSCloseMsgQue,                HANDLE_NAMEONLY, "OSCloseMsgQue",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_InterruptInitialize,          HANDLE_NAMEONLY, "InterruptInitialize",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_InterruptEnable,              HANDLE_NAMEONLY, "InterruptEnable",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_InterruptDisable,             HANDLE_NAMEONLY, "InterruptDisable",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_InterruptDone,                HANDLE_NAMEONLY, "InterruptDone",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_InterruptSetMode,             HANDLE_NAMEONLY, "InterruptSetMode",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_DisableAutoSync,              HANDLE_NAMEONLY, "DisableAutoSync",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_EnableAutoSync,               HANDLE_NAMEONLY, "EnableAutoSync",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_getvect,                      HANDLE_NAMEONLY, "getvect",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_setvect,                      HANDLE_NAMEONLY, "setvect",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetLCDContrast,               HANDLE_NAMEONLY, "GetLCDContrast",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetLCDContrast,               HANDLE_NAMEONLY, "SetLCDContrast",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_LCDOn,                        HANDLE_IMPLEMENTED, "LCDOn",                     LCDOn);
    DEFINE_INTERRUPT(SDKLIB_LCDOff,                       HANDLE_NAMEONLY, "LCDOff",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_CheckLCDOn,                   HANDLE_NAMEONLY, "CheckLCDOn",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_Buzzer,                       HANDLE_NAMEONLY, "Buzzer",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_KeyBeep,                      HANDLE_NAMEONLY, "KeyBeep",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetTimer1IntHandler,          HANDLE_NAMEONLY, "SetTimer1IntHandler",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetAutoPowerOff,              HANDLE_NAMEONLY, "SetAutoPowerOff",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetTimer1IntHandler,          HANDLE_NAMEONLY, "GetTimer1IntHandler",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_RemapMemory,                  HANDLE_NAMEONLY, "RemapMemory",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SysPowerOff,                  HANDLE_NAMEONLY, "SysPowerOff",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSysKeyState,               HANDLE_NAMEONLY, "SetSysKeyState",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSysKeyState,               HANDLE_NAMEONLY, "GetSysKeyState",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetBatteryType,               HANDLE_NAMEONLY, "GetBatteryType",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_BatteryLowCheck,              HANDLE_NAMEONLY, "BatteryLowCheck",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_lmalloc,                      HANDLE_IMPLEMENTED, "lmalloc",                   lmalloc);
    DEFINE_INTERRUPT(SDKLIB_lcalloc,                      HANDLE_IMPLEMENTED, "lcalloc",                   lcalloc);
    DEFINE_INTERRUPT(SDKLIB_lrealloc,                     HANDLE_NAMEONLY, "lrealloc",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__lfree,                       HANDLE_NAMEONLY, "_lfree",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPenEvent,                  HANDLE_NAMEONLY, "GetPenEvent",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_CheckPenEvent,                HANDLE_NAMEONLY, "CheckPenEvent",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearPenEvent,                HANDLE_NAMEONLY, "ClearPenEvent",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PutSystemEvent,               HANDLE_NAMEONLY, "PutSystemEvent",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetEvent,                     HANDLE_NAMEONLY, "GetEvent",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPendEvent,                 HANDLE_NAMEONLY, "GetPendEvent",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetEventType,                 HANDLE_NAMEONLY, "SetEventType",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetEventType,                 HANDLE_NAMEONLY, "GetEventType",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_PutEvent,                     HANDLE_NAMEONLY, "PutEvent",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_PutEventExt,                  HANDLE_NAMEONLY, "PutEventExt",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetLastEvent,                 HANDLE_NAMEONLY, "GetLastEvent",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_TestPendEvent,                HANDLE_NAMEONLY, "TestPendEvent",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearPendEvent,               HANDLE_NAMEONLY, "ClearPendEvent",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearPenState,                HANDLE_NAMEONLY, "ClearPenState",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearEvent,                   HANDLE_NAMEONLY, "ClearEvent",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearAllEvents,               HANDLE_NAMEONLY, "ClearAllEvents",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_TestKeyEvent,                 HANDLE_NAMEONLY, "TestKeyEvent",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSystemVariable,            HANDLE_NAMEONLY, "SetSystemVariable",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCharWidth,                 HANDLE_NAMEONLY, "GetCharWidth",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetFontHeight,                HANDLE_NAMEONLY, "GetFontHeight",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetFontType,                  HANDLE_NAMEONLY, "GetFontType",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetStringLength,              HANDLE_NAMEONLY, "GetStringLength",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetFontType,                  HANDLE_NAMEONLY, "SetFontType",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteAlignString,             HANDLE_NAMEONLY, "WriteAlignString",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteChar,                    HANDLE_NAMEONLY, "WriteChar",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteString,                  HANDLE_NAMEONLY, "WriteString",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteStringInWindow,          HANDLE_NAMEONLY, "WriteStringInWindow",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteStringInWindowEx,        HANDLE_NAMEONLY, "WriteStringInWindowEx",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_Printf,                       HANDLE_NAMEONLY, "Printf",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_PrintfXY,                     HANDLE_NAMEONLY, "PrintfXY",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowGraphic,                  HANDLE_NAMEONLY, "ShowGraphic",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SizeofGraphic,                HANDLE_NAMEONLY, "SizeofGraphic",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_InitGraphic,                  HANDLE_NAMEONLY, "InitGraphic",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateIcon,                   HANDLE_NAMEONLY, "CreateIcon",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetCursorSize,                HANDLE_NAMEONLY, "SetCursorSize",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCursorSize,                HANDLE_NAMEONLY, "GetCursorSize",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetCursorPosition,            HANDLE_NAMEONLY, "SetCursorPosition",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCursorPosition,            HANDLE_NAMEONLY, "GetCursorPosition",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetCursorType,                HANDLE_NAMEONLY, "SetCursorType",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCursorType,                HANDLE_NAMEONLY, "GetCursorType",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_CursorLock,                   HANDLE_NAMEONLY, "CursorLock",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_CursorUnlock,                 HANDLE_NAMEONLY, "CursorUnlock",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetTransparentColor,          HANDLE_NAMEONLY, "SetTransparentColor",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetTransparentColor,          HANDLE_NAMEONLY, "GetTransparentColor",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_rgbSetBkColor,                HANDLE_NAMEONLY, "rgbSetBkColor",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_rgbSetColor,                  HANDLE_NAMEONLY, "rgbSetColor",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_rgbGetBkColor,                HANDLE_NAMEONLY, "rgbGetBkColor",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_rgbGetColor,                  HANDLE_NAMEONLY, "rgbGetColor",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetPenStyle,                  HANDLE_NAMEONLY, "SetPenStyle",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPenStyle,                  HANDLE_NAMEONLY, "GetPenStyle",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPenSize,                   HANDLE_NAMEONLY, "GetPenSize",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetPenSize,                   HANDLE_NAMEONLY, "SetPenSize",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPixel,                     HANDLE_NAMEONLY, "GetPixel",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetPixel,                     HANDLE_NAMEONLY, "SetPixel",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetImage,                     HANDLE_NAMEONLY, "GetImage",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_PutImage,                     HANDLE_NAMEONLY, "PutImage",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDrawArea,                  HANDLE_NAMEONLY, "SetDrawArea",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetDrawArea,                  HANDLE_NAMEONLY, "GetDrawArea",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawLine,                     HANDLE_NAMEONLY, "DrawLine",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawRect,                     HANDLE_NAMEONLY, "DrawRect",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_FillRect,                     HANDLE_NAMEONLY, "FillRect",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawRoundRect,                HANDLE_NAMEONLY, "DrawRoundRect",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawCircle,                   HANDLE_NAMEONLY, "DrawCircle",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_FillCircle,                   HANDLE_NAMEONLY, "FillCircle",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawEllipse,                  HANDLE_NAMEONLY, "DrawEllipse",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_FillEllipse,                  HANDLE_NAMEONLY, "FillEllipse",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_InverseSetArea,               HANDLE_NAMEONLY, "InverseSetArea",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearScreen,                  HANDLE_NAMEONLY, "ClearScreen",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearSetArea,                 HANDLE_NAMEONLY, "ClearSetArea",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_ScrollDown,                   HANDLE_NAMEONLY, "ScrollDown",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_ScrollLeft,                   HANDLE_NAMEONLY, "ScrollLeft",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_ScrollRight,                  HANDLE_NAMEONLY, "ScrollRight",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_ScrollUp,                     HANDLE_NAMEONLY, "ScrollUp",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetRealLCD,                   HANDLE_NAMEONLY, "GetRealLCD",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetToRealLCD,                 HANDLE_NAMEONLY, "SetToRealLCD",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetToVirtualLCD,              HANDLE_NAMEONLY, "SetToVirtualLCD",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateVirtualLCD,             HANDLE_NAMEONLY, "CreateVirtualLCD",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_DeleteVirtualLCD,             HANDLE_NAMEONLY, "DeleteVirtualLCD",             nullptr);
    DEFINE_INTERRUPT(SDKLIB__BitBlt,                      HANDLE_NAMEONLY, "_BitBlt",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB___fillrect,                   HANDLE_NAMEONLY, "__fillrect",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetActiveLCD,                 HANDLE_NAMEONLY, "SetActiveLCD",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetRealLCD,                   HANDLE_NAMEONLY, "SetRealLCD",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetActiveLCD,                 HANDLE_IMPLEMENTED, "GetActiveLCD",              GetActiveLCD);
    DEFINE_INTERRUPT(SDKLIB_CreateCompatibleLCD,          HANDLE_NAMEONLY, "CreateCompatibleLCD",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateCompatibleImage,        HANDLE_NAMEONLY, "CreateCompatibleImage",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_DeleteLCD,                    HANDLE_NAMEONLY, "DeleteLCD",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_SelectLCDObject,              HANDLE_NAMEONLY, "SelectLCDObject",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_DeleteLCDObject,              HANDLE_NAMEONLY, "DeleteLCDObject",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDCObject,                  HANDLE_NAMEONLY, "SetDCObject",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetWindowSize,                HANDLE_NAMEONLY, "GetWindowSize",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetImageSize,                 HANDLE_NAMEONLY, "GetImageSize",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetImageSizeExt,              HANDLE_NAMEONLY, "GetImageSizeExt",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ImageData,                    HANDLE_NAMEONLY, "ImageData",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_SizeofImage,                  HANDLE_NAMEONLY, "SizeofImage",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeImage,                    HANDLE_NAMEONLY, "FreeImage",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_Delay,                        HANDLE_NAMEONLY, "Delay",                        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PenDelay,                     HANDLE_NAMEONLY, "PenDelay",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPenSilenceArea,            HANDLE_NAMEONLY, "GetPenSilenceArea",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetPenSilenceArea,            HANDLE_NAMEONLY, "SetPenSilenceArea",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_WarningBeep,                  HANDLE_NAMEONLY, "WarningBeep",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_LockSystem,                   HANDLE_NAMEONLY, "LockSystem",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_UnlockSystem,                 HANDLE_NAMEONLY, "UnlockSystem",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_CopyToClipBoard,              HANDLE_NAMEONLY, "CopyToClipBoard",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CopyFromClipBoard,            HANDLE_NAMEONLY, "CopyFromClipBoard",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearClipBoard,               HANDLE_NAMEONLY, "ClearClipBoard",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetClipBoardTextLength,       HANDLE_NAMEONLY, "GetClipBoardTextLength",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSysTime,                   HANDLE_NAMEONLY, "GetSysTime",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSysTime,                   HANDLE_NAMEONLY, "SetSysTime",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_PopupWaitingMsg,              HANDLE_NAMEONLY, "PopupWaitingMsg",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CloseWaitingMsg,              HANDLE_NAMEONLY, "CloseWaitingMsg",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetLanguageType,              HANDLE_NAMEONLY, "GetLanguageType",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetLanguageType,              HANDLE_NAMEONLY, "SetLanguageType",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetChineseFont,               HANDLE_NAMEONLY, "SetChineseFont",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetChineseFont,               HANDLE_NAMEONLY, "GetChineseFont",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetShiftState,                HANDLE_NAMEONLY, "SetShiftState",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetCapsState,                 HANDLE_NAMEONLY, "SetCapsState",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetShiftState,                HANDLE_NAMEONLY, "GetShiftState",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCapsState,                 HANDLE_NAMEONLY, "GetCapsState",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_Tradional2Simple,             HANDLE_NAMEONLY, "Tradional2Simple",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_Simple2Tradional,             HANDLE_NAMEONLY, "Simple2Tradional",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetEPTSLastChar,              HANDLE_NAMEONLY, "SetEPTSLastChar",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PlayAllVoice,                 HANDLE_NAMEONLY, "PlayAllVoice",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_StopAllVoice,                 HANDLE_NAMEONLY, "StopAllVoice",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_PauseAllVoice,                HANDLE_NAMEONLY, "PauseAllVoice",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_ContinueAllVoice,             HANDLE_NAMEONLY, "ContinueAllVoice",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetAllVoiceState,             HANDLE_NAMEONLY, "GetAllVoiceState",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_RecordVoiceEx,                HANDLE_NAMEONLY, "RecordVoiceEx",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PlaybackVoiceEx,              HANDLE_NAMEONLY, "PlaybackVoiceEx",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetAudioHandler,              HANDLE_NAMEONLY, "SetAudioHandler",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConvCharToUnicode,            HANDLE_NAMEONLY, "ConvCharToUnicode",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConvStrToUnicode,             HANDLE_NAMEONLY, "ConvStrToUnicode",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_CompSecretkey,                HANDLE_NAMEONLY, "CompSecretkey",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClearSecretkey,               HANDLE_NAMEONLY, "ClearSecretkey",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetUserFontHandle,            HANDLE_NAMEONLY, "SetUserFontHandle",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMasterIDInfo,              HANDLE_NAMEONLY, "GetMasterIDInfo",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ReadFollowMe,                 HANDLE_NAMEONLY, "ReadFollowMe",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPrivateState,              HANDLE_NAMEONLY, "GetPrivateState",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetPrivateState,              HANDLE_NAMEONLY, "SetPrivateState",              nullptr);
    DEFINE_INTERRUPT(SDKLIB__afnsplit,                    HANDLE_NAMEONLY, "_afnsplit",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__afnmerge,                    HANDLE_NAMEONLY, "_afnmerge",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__afcreate,                    HANDLE_NAMEONLY, "_afcreate",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__afcreateSz,                  HANDLE_NAMEONLY, "_afcreateSz",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__afopen,                      HANDLE_NAMEONLY, "_afopen",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__fclose,                      HANDLE_NAMEONLY, "_fclose",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__filesize,                    HANDLE_NAMEONLY, "_filesize",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB___fflush,                     HANDLE_NAMEONLY, "__fflush",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__fflushall,                   HANDLE_NAMEONLY, "_fflushall",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__rewind,                      HANDLE_NAMEONLY, "_rewind",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB___fseek,                      HANDLE_NAMEONLY, "__fseek",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__ftell,                       HANDLE_NAMEONLY, "_ftell",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB__feof,                        HANDLE_NAMEONLY, "_feof",                        nullptr);
    DEFINE_INTERRUPT(SDKLIB__fgetc,                       HANDLE_NAMEONLY, "_fgetc",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB__fgets,                       HANDLE_NAMEONLY, "_fgets",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB__fread,                       HANDLE_NAMEONLY, "_fread",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB__fputc,                       HANDLE_NAMEONLY, "_fputc",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB__fputs,                       HANDLE_NAMEONLY, "_fputs",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB__fwrite,                      HANDLE_NAMEONLY, "_fwrite",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__afindfirst,                  HANDLE_NAMEONLY, "_afindfirst",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__afindnext,                   HANDLE_NAMEONLY, "_afindnext",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__findclose,                   HANDLE_NAMEONLY, "_findclose",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__afgetattr,                   HANDLE_NAMEONLY, "_afgetattr",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__afsetattr,                   HANDLE_NAMEONLY, "_afsetattr",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__aremove,                     HANDLE_NAMEONLY, "_aremove",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__arename,                     HANDLE_NAMEONLY, "_arename",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__afcopy,                      HANDLE_NAMEONLY, "_afcopy",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__amkdir,                      HANDLE_NAMEONLY, "_amkdir",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__armdir,                      HANDLE_NAMEONLY, "_armdir",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__achdir,                      HANDLE_NAMEONLY, "_achdir",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__agetcurdir,                  HANDLE_NAMEONLY, "_agetcurdir",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__isformateddisk,              HANDLE_NAMEONLY, "_isformateddisk",              nullptr);
    DEFINE_INTERRUPT(SDKLIB__getfattype,                  HANDLE_NAMEONLY, "_getfattype",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__setdisk,                     HANDLE_NAMEONLY, "_setdisk",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__getdisk,                     HANDLE_NAMEONLY, "_getdisk",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__getdiskchar,                 HANDLE_NAMEONLY, "_getdiskchar",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB__setdiskchar,                 HANDLE_NAMEONLY, "_setdiskchar",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB__getdisknum,                  HANDLE_NAMEONLY, "_getdisknum",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_FSGetDiskRoomState,           HANDLE_NAMEONLY, "FSGetDiskRoomState",           nullptr);
    DEFINE_INTERRUPT(SDKLIB__OpenFile,                    HANDLE_IMPLEMENTED, "_OpenFile",                 _OpenFile);
    DEFINE_INTERRUPT(SDKLIB__OpenFileEx,                  HANDLE_NAMEONLY, "_OpenFileEx",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__OpenFileW,                   HANDLE_NAMEONLY, "_OpenFileW",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__CloseFile,                   HANDLE_NAMEONLY, "_CloseFile",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__ReadFile,                    HANDLE_NAMEONLY, "_ReadFile",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__FseekFile,                   HANDLE_NAMEONLY, "_FseekFile",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__FileSize,                    HANDLE_NAMEONLY, "_FileSize",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__OpenSubFile,                 HANDLE_NAMEONLY, "_OpenSubFile",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB__TellFile,                    HANDLE_NAMEONLY, "_TellFile",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBSave,                       HANDLE_NAMEONLY, "DBSave",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBSaveAll,                    HANDLE_NAMEONLY, "DBSaveAll",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBOpenUserFile,               HANDLE_NAMEONLY, "DBOpenUserFile",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBCreate,                     HANDLE_NAMEONLY, "DBCreate",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBCreateSZ,                   HANDLE_NAMEONLY, "DBCreateSZ",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBOpen,                       HANDLE_NAMEONLY, "DBOpen",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBClose,                      HANDLE_NAMEONLY, "DBClose",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBRemove,                     HANDLE_NAMEONLY, "DBRemove",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBEmpty,                      HANDLE_NAMEONLY, "DBEmpty",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBOptimize,                   HANDLE_NAMEONLY, "DBOptimize",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBRepair,                     HANDLE_NAMEONLY, "DBRepair",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetNewRecordPID,            HANDLE_NAMEONLY, "DBGetNewRecordPID",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetNewRecordID,             HANDLE_NAMEONLY, "DBGetNewRecordID",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBCreateNewRecord,            HANDLE_NAMEONLY, "DBCreateNewRecord",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBInsertRecord,               HANDLE_NAMEONLY, "DBInsertRecord",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBModifyRecord,               HANDLE_NAMEONLY, "DBModifyRecord",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBRemoveRecord,               HANDLE_NAMEONLY, "DBRemoveRecord",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBReadRecord,                 HANDLE_NAMEONLY, "DBReadRecord",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetRecordTime,              HANDLE_NAMEONLY, "DBGetRecordTime",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetRecordDate,              HANDLE_NAMEONLY, "DBGetRecordDate",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetRecordPid,               HANDLE_NAMEONLY, "DBGetRecordPid",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetCurRecordCount,          HANDLE_NAMEONLY, "DBGetCurRecordCount",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBOverLoadSortFunc,           HANDLE_NAMEONLY, "DBOverLoadSortFunc",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetRecPidState,             HANDLE_NAMEONLY, "DBGetRecPidState",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_DBGetDBState,                 HANDLE_NAMEONLY, "DBGetDBState",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetSystemDirectory,          HANDLE_NAMEONLY, "_GetSystemDirectory",          nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetTempPath,                 HANDLE_NAMEONLY, "_GetTempPath",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetPrivateProfileInt,        HANDLE_NAMEONLY, "_GetPrivateProfileInt",        nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetPrivateProfileString,     HANDLE_NAMEONLY, "_GetPrivateProfileString",     nullptr);
    DEFINE_INTERRUPT(SDKLIB__WritePrivateProfileString,   HANDLE_NAMEONLY, "_WritePrivateProfileString",   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetTadCityNo,                 HANDLE_NAMEONLY, "GetTadCityNo",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_RunApplicationA,              HANDLE_NAMEONLY, "RunApplicationA",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationNameA,          HANDLE_NAMEONLY, "GetApplicationNameA",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadProgramA,                 HANDLE_NAMEONLY, "LoadProgramA",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeProgram,                  HANDLE_NAMEONLY, "FreeProgram",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_ExecuteProgram,               HANDLE_NAMEONLY, "ExecuteProgram",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCurrentPathA,              HANDLE_IMPLEMENTED, "GetCurrentPathA",           getCurrentDir);
    DEFINE_INTERRUPT(SDKLIB_ProgramIsRunningA,            HANDLE_IMPLEMENTED, "ProgramIsRunningA",         prgrmIsRunning);
    DEFINE_INTERRUPT(SDKLIB_FindApplications,             HANDLE_NAMEONLY, "FindApplications",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeFindApplications,         HANDLE_NAMEONLY, "FreeFindApplications",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationInfo,           HANDLE_NAMEONLY, "GetApplicationInfo",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateAppView,                HANDLE_NAMEONLY, "CreateAppView",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_TextPicker,                   HANDLE_NAMEONLY, "TextPicker",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMonDays,                   HANDLE_NAMEONLY, "GetMonDays",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetWeekDay,                   HANDLE_NAMEONLY, "GetWeekDay",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConvSolarToLunar,             HANDLE_NAMEONLY, "ConvSolarToLunar",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConvLunarToSolar,             HANDLE_NAMEONLY, "ConvLunarToSolar",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetLeapMonth,                 HANDLE_NAMEONLY, "GetLeapMonth",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateEdit,                   HANDLE_NAMEONLY, "CreateEdit",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_EDInitEdit,                   HANDLE_NAMEONLY, "EDInitEdit",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_EDDraw,                       HANDLE_NAMEONLY, "EDDraw",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_EDDrawText,                   HANDLE_NAMEONLY, "EDDrawText",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_EDHandleEvent,                HANDLE_NAMEONLY, "EDHandleEvent",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertTextField,              HANDLE_NAMEONLY, "InsertTextField",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ReplaceEditBuffer,            HANDLE_NAMEONLY, "ReplaceEditBuffer",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetEditCmdFunc,               HANDLE_NAMEONLY, "SetEditCmdFunc",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetEditCmdFunc,               HANDLE_NAMEONLY, "GetEditCmdFunc",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetEditFunctionMenuState,     HANDLE_NAMEONLY, "SetEditFunctionMenuState",     nullptr);
    DEFINE_INTERRUPT(SDKLIB_DatePicker,                   HANDLE_NAMEONLY, "DatePicker",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateBoolDateField,          HANDLE_NAMEONLY, "CreateBoolDateField",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateBoolMenuField,          HANDLE_NAMEONLY, "CreateBoolMenuField",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateBoolField,              HANDLE_NAMEONLY, "CreateBoolField",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateBoolTextField,          HANDLE_NAMEONLY, "CreateBoolTextField",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateTimeField,              HANDLE_NAMEONLY, "CreateTimeField",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateDateField,              HANDLE_NAMEONLY, "CreateDateField",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateDoubleTimeField,        HANDLE_NAMEONLY, "CreateDoubleTimeField",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateNumericField,           HANDLE_NAMEONLY, "CreateNumericField",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDATEFIELD_draw,              HANDLE_NAMEONLY, "PDATEFIELD_draw",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDATEFIELD_handleEvent,       HANDLE_NAMEONLY, "PDATEFIELD_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDATEFIELD_setState,          HANDLE_NAMEONLY, "PDATEFIELD_setState",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_PNUMERICFIELD_handleEvent,    HANDLE_NAMEONLY, "PNUMERICFIELD_handleEvent",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_PBOOLFIELD_draw,              HANDLE_NAMEONLY, "PBOOLFIELD_draw",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_MessageBox,                   HANDLE_NAMEONLY, "MessageBox",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateMessageBox,             HANDLE_NAMEONLY, "CreateMessageBox",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_NumericPicker,                HANDLE_NAMEONLY, "NumericPicker",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetNumPkValidHandle,          HANDLE_NAMEONLY, "SetNumPkValidHandle",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateDetailView,             HANDLE_NAMEONLY, "CreateDetailView",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDETAILVIEW_draw,             HANDLE_NAMEONLY, "PDETAILVIEW_draw",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_NumericToStr,                 HANDLE_NAMEONLY, "NumericToStr",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_StrToNumeric,                 HANDLE_NAMEONLY, "StrToNumeric",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_AllocBlock,                   HANDLE_NAMEONLY, "AllocBlock",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeBlock,                    HANDLE_NAMEONLY, "FreeBlock",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_RelatedKeyButton,             HANDLE_NAMEONLY, "RelatedKeyButton",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_RelatedKeyButtonEx,           HANDLE_NAMEONLY, "RelatedKeyButtonEx",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_UnRelatedKeyButton,           HANDLE_NAMEONLY, "UnRelatedKeyButton",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateButton,                 HANDLE_NAMEONLY, "CreateButton",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeButton,                 HANDLE_NAMEONLY, "ChangeButton",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertButton,                 HANDLE_NAMEONLY, "InsertButton",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateStatic,                 HANDLE_NAMEONLY, "CreateStatic",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_PBUTTON_draw,                 HANDLE_NAMEONLY, "PBUTTON_draw",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertImageClip,              HANDLE_NAMEONLY, "InsertImageClip",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreatePageArrow,              HANDLE_NAMEONLY, "CreatePageArrow",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertPgUpDn,                 HANDLE_NAMEONLY, "InsertPgUpDn",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateHorPageArrow,           HANDLE_NAMEONLY, "CreateHorPageArrow",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateControlMenu,            HANDLE_NAMEONLY, "CreateControlMenu",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateMenuField,              HANDLE_NAMEONLY, "CreateMenuField",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateNumberSet,              HANDLE_NAMEONLY, "CreateNumberSet",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateStackedList,            HANDLE_NAMEONLY, "CreateStackedList",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateSlider,                 HANDLE_NAMEONLY, "CreateSlider",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateProgress,               HANDLE_NAMEONLY, "CreateProgress",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateRadioButton,            HANDLE_NAMEONLY, "CreateRadioButton",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PPROGRESSetPos,               HANDLE_NAMEONLY, "PPROGRESSetPos",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PPROGRESSetRange,             HANDLE_NAMEONLY, "PPROGRESSetRange",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSLIDER_SetPos,               HANDLE_NAMEONLY, "PSLIDER_SetPos",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSLIDER_SetRange,             HANDLE_NAMEONLY, "PSLIDER_SetRange",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSLIDER_handleEvent,          HANDLE_NAMEONLY, "PSLIDER_handleEvent",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_IsRadioButtonCheck,           HANDLE_NAMEONLY, "IsRadioButtonCheck",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_CheckDeskRadioButton,         HANDLE_NAMEONLY, "CheckDeskRadioButton",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_PNUMBERSET_handleEvent,       HANDLE_NAMEONLY, "PNUMBERSET_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_PMENUFIELD_draw,              HANDLE_NAMEONLY, "PMENUFIELD_draw",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PMENUFIELD_handleEvent,       HANDLE_NAMEONLY, "PMENUFIELD_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_PCONTROLMENU_draw,            HANDLE_NAMEONLY, "PCONTROLMENU_draw",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PCONTROLMENU_handleEvent,     HANDLE_NAMEONLY, "PCONTROLMENU_handleEvent",     nullptr);
    DEFINE_INTERRUPT(SDKLIB_PVIEW_draw,                   HANDLE_NAMEONLY, "PVIEW_draw",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_PVIEW_EraseBackGround,        HANDLE_NAMEONLY, "PVIEW_EraseBackGround",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PVIEW_handleEvent,            HANDLE_NAMEONLY, "PVIEW_handleEvent",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PVIEW_setState,               HANDLE_NAMEONLY, "PVIEW_setState",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_draw,                  HANDLE_NAMEONLY, "PGROUP_draw",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_handleEvent,           HANDLE_NAMEONLY, "PGROUP_handleEvent",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_insert,                HANDLE_NAMEONLY, "PGROUP_insert",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_redraw,                HANDLE_NAMEONLY, "PGROUP_redraw",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_setCurrent,            HANDLE_NAMEONLY, "PGROUP_setCurrent",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_setState,              HANDLE_NAMEONLY, "PGROUP_setState",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_execute,               HANDLE_NAMEONLY, "PGROUP_execute",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PGROUP_preView,               HANDLE_NAMEONLY, "PGROUP_preView",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateDeskBox,                HANDLE_NAMEONLY, "CreateDeskBox",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDESKBOX_draw,                HANDLE_NAMEONLY, "PDESKBOX_draw",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDESKBOX_drawTitle,           HANDLE_NAMEONLY, "PDESKBOX_drawTitle",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDESKBOX_handleEvent,         HANDLE_NAMEONLY, "PDESKBOX_handleEvent",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_PDESKBOX_redraw,              HANDLE_NAMEONLY, "PDESKBOX_redraw",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_Destroy,                      HANDLE_NAMEONLY, "Destroy",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_DeskBox_construct,            HANDLE_NAMEONLY, "DeskBox_construct",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawDeskBoxBound,             HANDLE_NAMEONLY, "DrawDeskBoxBound",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeCommandMenu,            HANDLE_NAMEONLY, "ChangeCommandMenu",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDeskBoxReturn,             HANDLE_NAMEONLY, "SetDeskBoxReturn",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_ExecView,                     HANDLE_NAMEONLY, "ExecView",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetExitWordVal,               HANDLE_NAMEONLY, "SetExitWordVal",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetExitWordVal,               HANDLE_NAMEONLY, "GetExitWordVal",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetCurModeWord,               HANDLE_NAMEONLY, "SetCurModeWord",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCurModeWord,               HANDLE_NAMEONLY, "GetCurModeWord",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertTitleBarButton,         HANDLE_NAMEONLY, "InsertTitleBarButton",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_Delete,                       HANDLE_NAMEONLY, "Delete",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB_DisableCommand,               HANDLE_NAMEONLY, "DisableCommand",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_EnableCommand,                HANDLE_NAMEONLY, "EnableCommand",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_InverseSetAreaArc,            HANDLE_NAMEONLY, "InverseSetAreaArc",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_InverseSetAreaColor,          HANDLE_NAMEONLY, "InverseSetAreaColor",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_InverseSetAreaArcColor,       HANDLE_NAMEONLY, "InverseSetAreaArcColor",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_SendMessage,                  HANDLE_NAMEONLY, "SendMessage",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SendMessageExt,               HANDLE_NAMEONLY, "SendMessageExt",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetPreView,                   HANDLE_NAMEONLY, "GetPreView",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetFocuseItem,                HANDLE_NAMEONLY, "SetFocuseItem",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetTabOrder,                  HANDLE_NAMEONLY, "SetTabOrder",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetDeskEntry,                 HANDLE_NAMEONLY, "GetDeskEntry",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetDeskItem,                  HANDLE_NAMEONLY, "GetDeskItem",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_QueryByCommand,               HANDLE_NAMEONLY, "QueryByCommand",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMaxScrX,                   HANDLE_NAMEONLY, "GetMaxScrX",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMaxScrY,                   HANDLE_NAMEONLY, "GetMaxScrY",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetDeskClientRect,            HANDLE_NAMEONLY, "GetDeskClientRect",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_InvalidateRect,               HANDLE_NAMEONLY, "InvalidateRect",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertSplitViewFrame,         HANDLE_NAMEONLY, "InsertSplitViewFrame",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateSplitView,              HANDLE_NAMEONLY, "CreateSplitView",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertSplitView,              HANDLE_NAMEONLY, "InsertSplitView",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSPLITVIEW_draw,              HANDLE_NAMEONLY, "PSPLITVIEW_draw",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSPLITVIEW_handleEvent,       HANDLE_NAMEONLY, "PSPLITVIEW_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSPLITVIEW_setState,          HANDLE_NAMEONLY, "PSPLITVIEW_setState",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertMultiPage,              HANDLE_NAMEONLY, "InsertMultiPage",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PPAGE_draw,                   HANDLE_NAMEONLY, "PPAGE_draw",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_PPAGE_handleEvent,            HANDLE_NAMEONLY, "PPAGE_handleEvent",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PMPAGE_insertPage,            HANDLE_NAMEONLY, "PMPAGE_insertPage",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PMPAGE_handleEvent,           HANDLE_NAMEONLY, "PMPAGE_handleEvent",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConverNumToStr,               HANDLE_NAMEONLY, "ConverNumToStr",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConverTimeToStr,              HANDLE_NAMEONLY, "ConverTimeToStr",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_DateToString,                 HANDLE_NAMEONLY, "DateToString",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_TimeToString,                 HANDLE_NAMEONLY, "TimeToString",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_ZfxGetWeekDay,                HANDLE_NAMEONLY, "ZfxGetWeekDay",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteNumber,                  HANDLE_NAMEONLY, "WriteNumber",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_ConverDateToStr,              HANDLE_NAMEONLY, "ConverDateToStr",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PopUpList,                    HANDLE_NAMEONLY, "PopUpList",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_PressAtRange,                 HANDLE_NAMEONLY, "PressAtRange",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_PressAtButton,                HANDLE_NAMEONLY, "PressAtButton",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_PressAtButtonInv,             HANDLE_NAMEONLY, "PressAtButtonInv",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PressAtButtonIconAddr,        HANDLE_NAMEONLY, "PressAtButtonIconAddr",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PressAtButtonIcon,            HANDLE_NAMEONLY, "PressAtButtonIcon",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_IsSystemItemID,               HANDLE_NAMEONLY, "IsSystemItemID",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_WaitingMessageBox,            HANDLE_NAMEONLY, "WaitingMessageBox",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateCalendar,               HANDLE_NAMEONLY, "CreateCalendar",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PCALENDAR_draw,               HANDLE_NAMEONLY, "PCALENDAR_draw",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PCALENDAR_handleEvent,        HANDLE_NAMEONLY, "PCALENDAR_handleEvent",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateScrollBar,              HANDLE_NAMEONLY, "CreateScrollBar",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertScrollBar,              HANDLE_NAMEONLY, "InsertScrollBar",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertScrollBarPosition,      HANDLE_NAMEONLY, "InsertScrollBarPosition",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSCROLLBAR_draw,              HANDLE_NAMEONLY, "PSCROLLBAR_draw",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSCROLLBAR_redraw,            HANDLE_NAMEONLY, "PSCROLLBAR_redraw",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PSCROLLBAR_handleEvent,       HANDLE_NAMEONLY, "PSCROLLBAR_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeScrollBarValue,         HANDLE_NAMEONLY, "ChangeScrollBarValue",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeScrollBarStep,          HANDLE_NAMEONLY, "ChangeScrollBarStep",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetScrollBarValue,            HANDLE_NAMEONLY, "SetScrollBarValue",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetScrollBarStep,             HANDLE_NAMEONLY, "SetScrollBarStep",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateLister,                 HANDLE_NAMEONLY, "CreateLister",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLISTER_draw,                 HANDLE_NAMEONLY, "PLISTER_draw",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLISTER_handleEvent,          HANDLE_NAMEONLY, "PLISTER_handleEvent",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLISTER_writeItem,            HANDLE_NAMEONLY, "PLISTER_writeItem",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLISTER_penDownAct,           HANDLE_NAMEONLY, "PLISTER_penDownAct",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLISTER_changeFocuseAct,      HANDLE_NAMEONLY, "PLISTER_changeFocuseAct",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeListRowHeight,          HANDLE_NAMEONLY, "ChangeListRowHeight",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeListItemSum,            HANDLE_NAMEONLY, "ChangeListItemSum",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateLongLister,             HANDLE_NAMEONLY, "CreateLongLister",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLONGLISTER_draw,             HANDLE_NAMEONLY, "PLONGLISTER_draw",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLONGLISTER_handleEvent,      HANDLE_NAMEONLY, "PLONGLISTER_handleEvent",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLONGLISTER_changeFocuseAct,  HANDLE_NAMEONLY, "PLONGLISTER_changeFocuseAct",  nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLONGLISTER_writeItem,        HANDLE_NAMEONLY, "PLONGLISTER_writeItem",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PLONGLISTER_penDownAct,       HANDLE_NAMEONLY, "PLONGLISTER_penDownAct",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeLongListItemSum,        HANDLE_NAMEONLY, "ChangeLongListItemSum",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeLongListRowHeight,      HANDLE_NAMEONLY, "ChangeLongListRowHeight",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSysColorConfig,            HANDLE_NAMEONLY, "SetSysColorConfig",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSysColorConfig,            HANDLE_NAMEONLY, "GetSysColorConfig",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSysColor,                  HANDLE_NAMEONLY, "GetSysColor",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSysColor,                  HANDLE_NAMEONLY, "SetSysColor",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDefaultSysIconCfg,         HANDLE_NAMEONLY, "SetDefaultSysIconCfg",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_DrawGradientRect,             HANDLE_NAMEONLY, "DrawGradientRect",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_TimePicker,                   HANDLE_NAMEONLY, "TimePicker",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetOpenFileName,             HANDLE_NAMEONLY, "_GetOpenFileName",             nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetSaveFileName,             HANDLE_NAMEONLY, "_GetSaveFileName",             nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetNextFileName,             HANDLE_NAMEONLY, "_GetNextFileName",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertFileFilter,             HANDLE_NAMEONLY, "InsertFileFilter",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_draw,             HANDLE_NAMEONLY, "PFILEFILTER_draw",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_handleEvent,      HANDLE_NAMEONLY, "PFILEFILTER_handleEvent",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_chgColumnText,    HANDLE_NAMEONLY, "PFILEFILTER_chgColumnText",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_setPathInfo,      HANDLE_NAMEONLY, "PFILEFILTER_setPathInfo",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_getFillMode,      HANDLE_NAMEONLY, "PFILEFILTER_getFillMode",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_setFillMode,      HANDLE_NAMEONLY, "PFILEFILTER_setFillMode",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_getItemName,      HANDLE_NAMEONLY, "PFILEFILTER_getItemName",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PFILEFILTER_writeColumn0,     HANDLE_NAMEONLY, "PFILEFILTER_writeColumn0",     nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetFileExecuteFunction,       HANDLE_NAMEONLY, "GetFileExecuteFunction",       nullptr);
    DEFINE_INTERRUPT(SDKLIB__EditFileName,                HANDLE_NAMEONLY, "_EditFileName",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetHandActiveState,       HANDLE_NAMEONLY, "IME_GetHandActiveState",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetCtrlSize,              HANDLE_NAMEONLY, "IME_GetCtrlSize",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_ActiveCtrl,               HANDLE_NAMEONLY, "IME_ActiveCtrl",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_IsActiveInputer,          HANDLE_NAMEONLY, "IME_IsActiveInputer",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SetDefault,               HANDLE_NAMEONLY, "IME_SetDefault",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetLXResult,              HANDLE_NAMEONLY, "IME_GetLXResult",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SetHandFilter,            HANDLE_NAMEONLY, "IME_SetHandFilter",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetHandFilter,            HANDLE_NAMEONLY, "IME_GetHandFilter",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_IsFuncKey,                HANDLE_NAMEONLY, "IME_IsFuncKey",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SearchKeyMap,             HANDLE_NAMEONLY, "IME_SearchKeyMap",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_IsSelectKey,              HANDLE_NAMEONLY, "IME_IsSelectKey",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SaveUserInfo,             HANDLE_NAMEONLY, "IME_SaveUserInfo",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_ReadUserInfo,             HANDLE_NAMEONLY, "IME_ReadUserInfo",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_IsEmptyFont,              HANDLE_NAMEONLY, "IME_IsEmptyFont",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SetHandWrtConfig,         HANDLE_NAMEONLY, "IME_SetHandWrtConfig",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SetHandWrtStudyFlag,      HANDLE_NAMEONLY, "IME_SetHandWrtStudyFlag",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SetRecognizeFlag,         HANDLE_NAMEONLY, "IME_SetRecognizeFlag",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetCtrl,                  HANDLE_NAMEONLY, "IME_GetCtrl",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetCtrlState,             HANDLE_NAMEONLY, "IME_GetCtrlState",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_GetViewInfo,              HANDLE_NAMEONLY, "IME_GetViewInfo",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SCcorrection,                 HANDLE_NAMEONLY, "SCcorrection",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_SCwildcard,                   HANDLE_NAMEONLY, "SCwildcard",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SCverify,                     HANDLE_NAMEONLY, "SCverify",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_InitialSpelling,              HANDLE_NAMEONLY, "InitialSpelling",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SCopenDatabase,               HANDLE_NAMEONLY, "SCopenDatabase",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SCcloseDatabase,              HANDLE_NAMEONLY, "SCcloseDatabase",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SCGetMaxWordLen,              HANDLE_NAMEONLY, "SCGetMaxWordLen",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetTotalItem,                 HANDLE_NAMEONLY, "GetTotalItem",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMaxBlockSize,              HANDLE_NAMEONLY, "GetMaxBlockSize",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetKeyWord,                   HANDLE_NAMEONLY, "GetKeyWord",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetAllContent,                HANDLE_NAMEONLY, "GetAllContent",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSaveAddress,               HANDLE_NAMEONLY, "GetSaveAddress",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetAlpExit,                   HANDLE_NAMEONLY, "GetAlpExit",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetAlpExit,                   HANDLE_NAMEONLY, "SetAlpExit",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDictFont,                  HANDLE_NAMEONLY, "SetDictFont",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetDictFont,                  HANDLE_NAMEONLY, "GetDictFont",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_UniversalCrossSearch,         HANDLE_NAMEONLY, "UniversalCrossSearch",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_UniversalDictList,            HANDLE_NAMEONLY, "UniversalDictList",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_UniversalDictListForAll,      HANDLE_NAMEONLY, "UniversalDictListForAll",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_UniversalCardList,            HANDLE_NAMEONLY, "UniversalCardList",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SearchEveryWay,               HANDLE_NAMEONLY, "SearchEveryWay",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetAddDictPosInList,          HANDLE_NAMEONLY, "GetAddDictPosInList",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCurrentCardDictNum,        HANDLE_NAMEONLY, "GetCurrentCardDictNum",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_SearchCurrentCardDictInfo,    HANDLE_NAMEONLY, "SearchCurrentCardDictInfo",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_ReleaseCardDictInfo,          HANDLE_NAMEONLY, "ReleaseCardDictInfo",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_SetBufferAttrib,    HANDLE_NAMEONLY, "PRICHVIEW_SetBufferAttrib",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_SpeechForRich,      HANDLE_NAMEONLY, "PRICHVIEW_SpeechForRich",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_IsDictOuYu,                   HANDLE_NAMEONLY, "IsDictOuYu",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateRichView,               HANDLE_NAMEONLY, "CreateRichView",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_AddScrollBarToRichview,       HANDLE_NAMEONLY, "AddScrollBarToRichview",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeScrollBarToRichview,    HANDLE_NAMEONLY, "ChangeScrollBarToRichview",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_ChangeScrollBarPos,           HANDLE_NAMEONLY, "ChangeScrollBarPos",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDisplayMode,               HANDLE_NAMEONLY, "SetDisplayMode",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetShowSearchLayer,           HANDLE_NAMEONLY, "SetShowSearchLayer",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetWindowState,               HANDLE_NAMEONLY, "SetWindowState",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetWindowState,               HANDLE_NAMEONLY, "GetWindowState",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetBaseLine,                  HANDLE_NAMEONLY, "GetBaseLine",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetBaseLine,                  HANDLE_NAMEONLY, "SetBaseLine",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_BackupWindowState,            HANDLE_NAMEONLY, "BackupWindowState",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_RestoreWindowState,           HANDLE_NAMEONLY, "RestoreWindowState",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_draw,               HANDLE_NAMEONLY, "PRICHVIEW_draw",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_handleEvent,        HANDLE_NAMEONLY, "PRICHVIEW_handleEvent",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMarkString,                HANDLE_NAMEONLY, "GetMarkString",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetRichWindowState,           HANDLE_NAMEONLY, "GetRichWindowState",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetShowMode,                  HANDLE_NAMEONLY, "SetShowMode",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_ShowSearchLayer,    HANDLE_NAMEONLY, "PRICHVIEW_ShowSearchLayer",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_SetStateForUser,    HANDLE_NAMEONLY, "PRICHVIEW_SetStateForUser",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_WriteString,        HANDLE_NAMEONLY, "PRICHVIEW_WriteString",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_RichViewSetColor,             HANDLE_NAMEONLY, "RichViewSetColor",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetTotalLine,                 HANDLE_NAMEONLY, "SetTotalLine",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowRichContentOnly,          HANDLE_NAMEONLY, "ShowRichContentOnly",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_SetCttsType,        HANDLE_NAMEONLY, "PRICHVIEW_SetCttsType",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDefLineEditorInfo,         HANDLE_NAMEONLY, "SetDefLineEditorInfo",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_LineEditor,                   HANDLE_NAMEONLY, "LineEditor",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_LineEditor_handleEvent,       HANDLE_NAMEONLY, "LineEditor_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_DictEditor_handleEvent,       HANDLE_NAMEONLY, "DictEditor_handleEvent",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_LEModuleForUser,              HANDLE_NAMEONLY, "LEModuleForUser",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_LineEditor_ModuleForUser,     HANDLE_NAMEONLY, "LineEditor_ModuleForUser",     nullptr);
    DEFINE_INTERRUPT(SDKLIB_LEDictWiseSearch,             HANDLE_NAMEONLY, "LEDictWiseSearch",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_IsWildCard,                   HANDLE_NAMEONLY, "IsWildCard",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_LEditor_draw,                 HANDLE_NAMEONLY, "LEditor_draw",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_LINEEDITOR_changeFocuseAct,   HANDLE_NAMEONLY, "LINEEDITOR_changeFocuseAct",   nullptr);
    DEFINE_INTERRUPT(SDKLIB_LEDSK_handleEvent,            HANDLE_NAMEONLY, "LEDSK_handleEvent",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_LineEditorCompareStr,         HANDLE_NAMEONLY, "LineEditorCompareStr",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_LineEditorWiseSearch,         HANDLE_NAMEONLY, "LineEditorWiseSearch",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_SearchDict,                   HANDLE_NAMEONLY, "SearchDict",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSearchDictShortName,       HANDLE_NAMEONLY, "GetSearchDictShortName",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCurrentMainDictNum,        HANDLE_NAMEONLY, "GetCurrentMainDictNum",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_OKSpeechForRich,    HANDLE_NAMEONLY, "PRICHVIEW_OKSpeechForRich",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetIndexHcaOffset,            HANDLE_NAMEONLY, "GetIndexHcaOffset",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetIndexIconOffset,           HANDLE_NAMEONLY, "GetIndexIconOffset",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetAppIndexIconOffset,        HANDLE_NAMEONLY, "GetAppIndexIconOffset",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadHCAToGraphicByIdx,        HANDLE_NAMEONLY, "LoadHCAToGraphicByIdx",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadHCAToGraphicHeadByIdx,    HANDLE_NAMEONLY, "LoadHCAToGraphicHeadByIdx",    nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetIndexHCAHeight,            HANDLE_NAMEONLY, "GetIndexHCAHeight",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetIndexHCAWidth,             HANDLE_NAMEONLY, "GetIndexHCAWidth",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowHCAByOffset,              HANDLE_NAMEONLY, "ShowHCAByOffset",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetHCATransparentMode,        HANDLE_NAMEONLY, "SetHCATransparentMode",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_ReleaseHcaCache,              HANDLE_NAMEONLY, "ReleaseHcaCache",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ResetCache,                   HANDLE_NAMEONLY, "ResetCache",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetLibDataindexOffset,        HANDLE_NAMEONLY, "SetLibDataindexOffset",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetLibAddrFromID,             HANDLE_NAMEONLY, "GetLibAddrFromID",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowBookProFromFile,          HANDLE_NAMEONLY, "ShowBookProFromFile",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_RunBook,                      HANDLE_NAMEONLY, "RunBook",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowBook,                     HANDLE_NAMEONLY, "ShowBook",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowBookPreview,              HANDLE_NAMEONLY, "ShowBookPreview",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowBookEx,                   HANDLE_NAMEONLY, "ShowBookEx",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_USBMassStorageRun,            HANDLE_NAMEONLY, "USBMassStorageRun",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetPassThroughCallBack,       HANDLE_NAMEONLY, "SetPassThroughCallBack",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_ctts_predict,                 HANDLE_NAMEONLY, "ctts_predict",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_ctts_nounce,                  HANDLE_NAMEONLY, "ctts_nounce",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetAllVoiceState,             HANDLE_NAMEONLY, "SetAllVoiceState",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_OpenPCMCodec,                 HANDLE_NAMEONLY, "OpenPCMCodec",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_ClosePCMCodec,                HANDLE_NAMEONLY, "ClosePCMCodec",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_AudioPlayBackPause,           HANDLE_NAMEONLY, "AudioPlayBackPause",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_AudioPlayBackContinue,        HANDLE_NAMEONLY, "AudioPlayBackContinue",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PlayWaveData,                 HANDLE_NAMEONLY, "PlayWaveData",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_StopWaveDataPlay,             HANDLE_NAMEONLY, "StopWaveDataPlay",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_PauseWaveDataPlay,            HANDLE_NAMEONLY, "PauseWaveDataPlay",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_ContinueWaveDataPlay,         HANDLE_NAMEONLY, "ContinueWaveDataPlay",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_RecordAdpcmEx,                HANDLE_NAMEONLY, "RecordAdpcmEx",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_StopADPCMRecord,              HANDLE_NAMEONLY, "StopADPCMRecord",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_IsWaveDataPlayStopped,        HANDLE_NAMEONLY, "IsWaveDataPlayStopped",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_PlayMP3Data,                  HANDLE_NAMEONLY, "PlayMP3Data",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_StopMP3DataPlay,              HANDLE_NAMEONLY, "StopMP3DataPlay",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_PausePlayMP3Data,             HANDLE_NAMEONLY, "PausePlayMP3Data",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_ContinuePlayMP3Data,          HANDLE_NAMEONLY, "ContinuePlayMP3Data",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_InitBestlkHandle,             HANDLE_NAMEONLY, "InitBestlkHandle",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetBestlkVoiceHandle,         HANDLE_NAMEONLY, "SetBestlkVoiceHandle",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_RegistInputer,            HANDLE_NAMEONLY, "IME_RegistInputer",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetLanguageAttr,              HANDLE_NAMEONLY, "SetLanguageAttr",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetDialectAttr,               HANDLE_NAMEONLY, "SetDialectAttr",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetLineEditorAttr,            HANDLE_NAMEONLY, "SetLineEditorAttr",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetBookLibDataHFile,          HANDLE_NAMEONLY, "SetBookLibDataHFile",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetBookVocLibDataHFile,       HANDLE_NAMEONLY, "SetBookVocLibDataHFile",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_SetGlobalVar,             HANDLE_NAMEONLY, "IME_SetGlobalVar",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_IME_Functions,                HANDLE_NAMEONLY, "IME_Functions",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_LE_SupportMultiLangFunc,      HANDLE_NAMEONLY, "LE_SupportMultiLangFunc",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_ShowBookFromHANDLE,           HANDLE_NAMEONLY, "ShowBookFromHANDLE",           nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfnsplit,                    HANDLE_NAMEONLY, "_wfnsplit",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfnmerge,                    HANDLE_NAMEONLY, "_wfnmerge",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfcreate,                    HANDLE_NAMEONLY, "_wfcreate",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfcreateSz,                  HANDLE_NAMEONLY, "_wfcreateSz",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB___wfopen,                     HANDLE_NAMEONLY, "__wfopen",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfindfirst,                  HANDLE_NAMEONLY, "_wfindfirst",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfindnext,                   HANDLE_NAMEONLY, "_wfindnext",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfgetattr,                   HANDLE_NAMEONLY, "_wfgetattr",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfsetattr,                   HANDLE_NAMEONLY, "_wfsetattr",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB___wremove,                    HANDLE_NAMEONLY, "__wremove",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB__wrename,                     HANDLE_NAMEONLY, "_wrename",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfcopy,                      HANDLE_NAMEONLY, "_wfcopy",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__wmkdir,                      HANDLE_NAMEONLY, "_wmkdir",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__wrmdir,                      HANDLE_NAMEONLY, "_wrmdir",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__wchdir,                      HANDLE_NAMEONLY, "_wchdir",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__wgetcurdir,                  HANDLE_NAMEONLY, "_wgetcurdir",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB__afsettime,                   HANDLE_NAMEONLY, "_afsettime",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfsettime,                   HANDLE_NAMEONLY, "_wfsettime",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__afullpath,                   HANDLE_NAMEONLY, "_afullpath",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB__wfullpath,                   HANDLE_NAMEONLY, "_wfullpath",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_RunApplicationW,              HANDLE_NAMEONLY, "RunApplicationW",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationNameW,          HANDLE_NAMEONLY, "GetApplicationNameW",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadProgramW,                 HANDLE_NAMEONLY, "LoadProgramW",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCurrentPathW,              HANDLE_NAMEONLY, "GetCurrentPathW",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_ProgramIsRunningW,            HANDLE_NAMEONLY, "ProgramIsRunningW",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadHFileProgramW,            HANDLE_NAMEONLY, "LoadHFileProgramW",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadHFileProgramA,            HANDLE_NAMEONLY, "LoadHFileProgramA",            nullptr);
    DEFINE_INTERRUPT(SDKLIB__LoadLibraryA,                HANDLE_IMPLEMENTED, "_LoadLibraryA",             _LoadLibraryA);
    DEFINE_INTERRUPT(SDKLIB__GetModuleFileNameA,          HANDLE_NAMEONLY, "_GetModuleFileNameA",          nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetModuleHandleA,            HANDLE_NAMEONLY, "_GetModuleHandleA",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationProcA,          HANDLE_NAMEONLY, "GetApplicationProcA",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_StayResidentProgramA,         HANDLE_NAMEONLY, "StayResidentProgramA",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_UnStayResidentProgramA,       HANDLE_NAMEONLY, "UnStayResidentProgramA",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_CheckProgramIsStayResident,   HANDLE_NAMEONLY, "CheckProgramIsStayResident",   nullptr);
    DEFINE_INTERRUPT(SDKLIB__FindResourceA,               HANDLE_NAMEONLY, "_FindResourceA",               nullptr);
    DEFINE_INTERRUPT(SDKLIB__FindResourceExA,             HANDLE_NAMEONLY, "_FindResourceExA",             nullptr);
    DEFINE_INTERRUPT(SDKLIB__LoadLibraryW,                HANDLE_NAMEONLY, "_LoadLibraryW",                nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetModuleFileNameW,          HANDLE_NAMEONLY, "_GetModuleFileNameW",          nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetModuleHandleW,            HANDLE_NAMEONLY, "_GetModuleHandleW",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationProcW,          HANDLE_NAMEONLY, "GetApplicationProcW",          nullptr);
    DEFINE_INTERRUPT(SDKLIB__FindResourceW,               HANDLE_IMPLEMENTED, "_FindResourceW",            _FindResourceW);
    DEFINE_INTERRUPT(SDKLIB__FindResourceExW,             HANDLE_NAMEONLY, "_FindResourceExW",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_StayResidentProgramW,         HANDLE_NAMEONLY, "StayResidentProgramW",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_UnStayResidentProgramW,       HANDLE_NAMEONLY, "UnStayResidentProgramW",       nullptr);
    DEFINE_INTERRUPT(SDKLIB__FreeLibrary,                 HANDLE_IMPLEMENTED, "_FreeLibrary",              _FreeLibrary);
    DEFINE_INTERRUPT(SDKLIB__GetProcAddress,              HANDLE_NAMEONLY, "_GetProcAddress",              nullptr);
    DEFINE_INTERRUPT(SDKLIB__SizeofResource,              HANDLE_NAMEONLY, "_SizeofResource",              nullptr);
    DEFINE_INTERRUPT(SDKLIB__OpenResourceItemFile,        HANDLE_NAMEONLY, "_OpenResourceItemFile",        nullptr);
    DEFINE_INTERRUPT(SDKLIB__CloseResourceItemFile,       HANDLE_NAMEONLY, "_CloseResourceItemFile",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCardSN,                    HANDLE_NAMEONLY, "GetCardSN",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetCardSize,                  HANDLE_NAMEONLY, "GetCardSize",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetThaiWord,                  HANDLE_NAMEONLY, "GetThaiWord",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadThaiGrammarLib,           HANDLE_NAMEONLY, "LoadThaiGrammarLib",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeThaiGrammarLib,           HANDLE_NAMEONLY, "FreeThaiGrammarLib",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteComDebugMsg,             HANDLE_IMPLEMENTED, "WriteComDebugMsg",           dbgMsg);
    DEFINE_INTERRUPT(SDKLIB_CreateIconButton,             HANDLE_NAMEONLY, "CreateIconButton",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadImageFile,                HANDLE_NAMEONLY, "LoadImageFile",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetResourceCfg,               HANDLE_NAMEONLY, "GetResourceCfg",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetSystemDefaultLangID,       HANDLE_NAMEONLY, "GetSystemDefaultLangID",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSystemDefaultLangID,       HANDLE_NAMEONLY, "SetSystemDefaultLangID",       nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateFile,                   HANDLE_NAMEONLY, "CreateFile",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_DeleteFile,                   HANDLE_NAMEONLY, "DeleteFile",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_ReadFile,                     HANDLE_NAMEONLY, "ReadFile",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB_WriteFile,                    HANDLE_NAMEONLY, "WriteFile",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetFilePointer,               HANDLE_NAMEONLY, "SetFilePointer",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DeviceIoControl,              HANDLE_NAMEONLY, "DeviceIoControl",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_CloseHandle,                  HANDLE_NAMEONLY, "CloseHandle",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_DictLastWord,                 HANDLE_NAMEONLY, "DictLastWord",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetTransBuffer,               HANDLE_NAMEONLY, "GetTransBuffer",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DictIsYuanYinPhonetic,        HANDLE_NAMEONLY, "DictIsYuanYinPhonetic",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_InsertButtonTeam,             HANDLE_NAMEONLY, "InsertButtonTeam",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_CreateSearchInfo,             HANDLE_NAMEONLY, "CreateSearchInfo",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeSearchInfo,               HANDLE_NAMEONLY, "FreeSearchInfo",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_DefProcessContent,            HANDLE_NAMEONLY, "DefProcessContent",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_DefInsertPic,                 HANDLE_NAMEONLY, "DefInsertPic",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_DefGetWholeWord,              HANDLE_NAMEONLY, "DefGetWholeWord",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_DefAddALine,                  HANDLE_NAMEONLY, "DefAddALine",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_GetFlagLength,      HANDLE_NAMEONLY, "PRICHVIEW_GetFlagLength",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_FilterMark,         HANDLE_NAMEONLY, "PRICHVIEW_FilterMark",         nullptr);
    DEFINE_INTERRUPT(SDKLIB_AddNewWord,                   HANDLE_NAMEONLY, "AddNewWord",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMaxSearchLayer,            HANDLE_NAMEONLY, "GetMaxSearchLayer",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_FormatMessage,                HANDLE_NAMEONLY, "FormatMessage",                nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetFont,                      HANDLE_NAMEONLY, "SetFont",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetFont,                      HANDLE_NAMEONLY, "GetFont",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetFontWidth,                 HANDLE_NAMEONLY, "GetFontWidth",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetStringLengthEx,            HANDLE_NAMEONLY, "GetStringLengthEx",            nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetFontStyle,                 HANDLE_NAMEONLY, "SetFontStyle",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_RegisterUserFont,             HANDLE_NAMEONLY, "RegisterUserFont",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_UnRegisterUserFont,           HANDLE_NAMEONLY, "UnRegisterUserFont",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetCurrentVideoDevice,        HANDLE_NAMEONLY, "SetCurrentVideoDevice",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_SetSupportDoubleLCD,          HANDLE_NAMEONLY, "SetSupportDoubleLCD",          nullptr);
    DEFINE_INTERRUPT(SDKLIB__GetDriverType,               HANDLE_NAMEONLY, "_GetDriverType",               nullptr);
    DEFINE_INTERRUPT(SDKLIB_EditControlFunc,              HANDLE_NAMEONLY, "EditControlFunc",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMasterSerialNumber,        HANDLE_NAMEONLY, "GetMasterSerialNumber",        nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetMasterVendorInfo,          HANDLE_NAMEONLY, "GetMasterVendorInfo",          nullptr);
    DEFINE_INTERRUPT(SDKLIB_PRICHVIEW_SetDisplayPosition, HANDLE_NAMEONLY, "PRICHVIEW_SetDisplayPosition", nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationHeadInfoA,      HANDLE_NAMEONLY, "GetApplicationHeadInfoA",      nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetApplicationHeadInfoW,      HANDLE_NAMEONLY, "GetApplicationHeadInfoW",      nullptr);
    DEFINE_INTERRUPT(SDKLIB__FreeFindResInfo,             HANDLE_NAMEONLY, "_FreeFindResInfo",             nullptr);
    DEFINE_INTERRUPT(SDKLIB_GetWholeWord,                 HANDLE_NAMEONLY, "GetWholeWord",                 nullptr);
    DEFINE_INTERRUPT(SDKLIB_LoadWordGrammarLib,           HANDLE_NAMEONLY, "LoadWordGrammarLib",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_FreeWordGrammarLib,           HANDLE_NAMEONLY, "FreeWordGrammarLib",           nullptr);
    DEFINE_INTERRUPT(SDKLIB_mount_file_disk,              HANDLE_NAMEONLY, "mount_file_disk",              nullptr);
    DEFINE_INTERRUPT(SDKLIB_umount_file_disk,             HANDLE_NAMEONLY, "umount_file_disk",             nullptr);
    DEFINE_INTERRUPT(SDKLIB___close,                      HANDLE_NAMEONLY, "__close",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB___commit,                     HANDLE_NAMEONLY, "__commit",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB___creat,                      HANDLE_NAMEONLY, "__creat",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB___dup,                        HANDLE_NAMEONLY, "__dup",                        nullptr);
    DEFINE_INTERRUPT(SDKLIB___wcreat,                     HANDLE_NAMEONLY, "__wcreat",                     nullptr);
    DEFINE_INTERRUPT(SDKLIB___eof,                        HANDLE_NAMEONLY, "__eof",                        nullptr);
    DEFINE_INTERRUPT(SDKLIB___get_errno,                  HANDLE_NAMEONLY, "__get_errno",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB___lseek,                      HANDLE_NAMEONLY, "__lseek",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB___lseeki64,                   HANDLE_NAMEONLY, "__lseeki64",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB___open,                       HANDLE_NAMEONLY, "__open",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB___wopen,                      HANDLE_NAMEONLY, "__wopen",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB___read,                       HANDLE_NAMEONLY, "__read",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB___set_errno,                  HANDLE_NAMEONLY, "__set_errno",                  nullptr);
    DEFINE_INTERRUPT(SDKLIB___tell,                       HANDLE_NAMEONLY, "__tell",                       nullptr);
    DEFINE_INTERRUPT(SDKLIB___telli64,                    HANDLE_NAMEONLY, "__telli64",                    nullptr);
    DEFINE_INTERRUPT(SDKLIB___truncate,                   HANDLE_NAMEONLY, "__truncate",                   nullptr);
    DEFINE_INTERRUPT(SDKLIB___write,                      HANDLE_NAMEONLY, "__write",                      nullptr);
    DEFINE_INTERRUPT(SDKLIB__MultiByteToWideChar,         HANDLE_NAMEONLY, "_MultiByteToWideChar",         nullptr);
    DEFINE_INTERRUPT(SDKLIB__WideCharToMultiByte,         HANDLE_NAMEONLY, "_WideCharToMultiByte",         nullptr);
}


void Executor::execute()
{
    uint32_t ins = 0x0;
    uint32_t SP0, PC0, SP1, PC1;
    PC0 = m_exec->get_entry();
    uc_reg_read(m_uc, UC_ARM_REG_SP, &SP1);
    SP0 = SP1;
    printf("Starting execution at 0x%X\n\n", PC0);
    while (m_err == UC_ERR_OK)
    {
        m_err = uc_emu_start(m_uc, PC0, 0, 0, ins);

        uc_reg_read(m_uc, UC_ARM_REG_PC, &PC1);
        uc_reg_read(m_uc, UC_ARM_REG_SP, &SP1);

        printf("Executed 0x%X instructions\n PC: %8X | SP: %8X\n", ins, PC1, SP1);

        //_sleep(100);

        //if ((SP0 == SP1) && (PC0 == PC1))
        //{
        //    printf("Not moving, broke loop\n");
            //break;
        //}

        SP0 = SP1;
        PC0 = PC1;
    }
    
    if (m_err != UC_ERR_OK) {
        uint32_t lr;
        uc_reg_read(m_uc, UC_ARM_REG_LR, &lr);
        printf("Execution aborted on error: %s!\nLR: 0x%08X\n", uc_strerror(m_err), lr);
    }

}

void interrupt_hook(uc_engine *uc, uint64_t address, uint32_t size, void *user_data)
{

    uint32_t r0, r1, r2, r3, sp, pc, lr;
    void* args[6] = { &r0, &r1, &r2, &r3, &sp, &pc };
    int regs[6] = { UC_ARM_REG_R0, UC_ARM_REG_R1, UC_ARM_REG_R2, UC_ARM_REG_R3, UC_ARM_REG_SP, UC_ARM_REG_PC };


    uc_reg_read_batch(uc, regs, args, sizeof(args) / sizeof(void*));

    uint32_t SVC;
    uc_mem_read(uc, pc - 4, &SVC, 4);
    uc_mem_read(uc, sp, &lr, 4);


    SVC &= 0xFFFFF;


    auto _int = sExecutor->m_interrupts.find(static_cast<InterruptID>(SVC));
    if (_int != sExecutor->m_interrupts.end()) {
        auto _handle = _int->second;

        uint32_t result = 0;
        switch (_handle->status) {
        case HANDLE_IMPLEMENTED:
            printf("[%05X] %s() called\n", _handle->id, _handle->name);
            result = _handle->callback(uc, r0, r1, r2, r3, sp, sExecutor->m_stack, sExecutor->m_exec->get_mem());
            break;
        case HANDLE_NAMEONLY:
            printf("[%05X] %s() UNHANDLED\n", _handle->id, _handle->name);
            printf("    r0: %08X|%i\n    r1: %08X|%i\n    r2: %08X|%i\n    r3: %08X|%i\n    sp: %08X\n", r0, r0, r1, r1, r2, r2, r3, r3, sp);
            break;
        case HANDLE_UNKOWN:
        default:
            printf("[%05X] UNNAMED UNHANDLED\n", _handle->id);
            break;
        }
        uc_reg_write(uc, UC_ARM_REG_R0, &result);
    }
    else
        printf("[%05X] UNDEFINED\n", SVC);
    printf("    Caller: %08X\n", lr - 4);

    
    sp += 8;

    uc_reg_write(uc, UC_ARM_REG_SP, &sp);
    uc_reg_write(uc, UC_ARM_REG_PC, &lr);

    /*if (err != UC_ERR_OK)
        printf("COULD NOT READ R0");
    else*/
    //    printf("+[0x%05X] Interrupt called\n    PC=%08X\n    LR=%08X\n", SVC, pc, LR);

    //_sleep(1500);

}