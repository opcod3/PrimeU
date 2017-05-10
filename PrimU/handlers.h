#pragma once

#include "stdafx.h"

class Memory;


uint32_t dbgMsg(Arguments* args);


uint32_t getCurrentDir(Arguments* args);
uint32_t prgrmIsRunning(Arguments* args);
uint32_t _FindResourceW(Arguments* args);
uint32_t _OpenFile(Arguments* args);
uint32_t _LoadLibraryA(Arguments* args);
uint32_t _FreeLibrary(Arguments* args);
uint32_t OSInitCriticalSection(Arguments* args);
uint32_t OSSetEvent(Arguments* args);
uint32_t OSCreateEvent(Arguments* args);
uint32_t LCDOn(Arguments* args);
uint32_t GetActiveLCD(Arguments* args);

uint32_t lcalloc(Arguments* args);
uint32_t lmalloc(Arguments* args);
uint32_t _lfree(Arguments* args);
uint32_t lrealloc(Arguments* args);

uint32_t _amkdir(Arguments* args);
uint32_t _achdir(Arguments* args);
uint32_t __wfopen(Arguments* args);

uint32_t OSCreateThread(Arguments* args);