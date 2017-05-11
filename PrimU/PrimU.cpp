// PrimU.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include "executable.h"
#include "executor.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: %s armfir.elf\n", argv[0]);
        return 1;
    }

    Executable exec(argv[1]);

    if (exec.get_state() == EXEC_LOAD_FAILED)
    {
        printf("Failed to load executable");
        return 1;
    }

    if (!sExecutor->Initialize(&exec))
    {
        printf("Initializing VM failed. Returned:%i", sExecutor->GetLastError());
        return 1;
    }

    sExecutor->Execute();
    sExecutor->Cleanup();
    getchar();

    return 0;
}

