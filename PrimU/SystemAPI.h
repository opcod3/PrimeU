#pragma once
#pragma once

#include "common.h"

#include "interrupts.h"
#include "InterruptHandler.h"

class SystemAPI
{
public:
    static SystemAPI* GetInstance() { return !_instance ? _instance = new SystemAPI : _instance; }

    uint32_t Call(InterruptID id, Arguments args);

private:
    SystemAPI();
    ~SystemAPI() { delete _instance; }
    SystemAPI(SystemAPI const&) = delete;
    void operator=(SystemAPI const&) = delete;
    static SystemAPI* _instance;

    std::map<InterruptID, InterruptHandler*> _handlers;

    void RegisterHandler(InterruptHandler* handler);
};


#define sSystemAPI SystemAPI::GetInstance()
