#include "main.h"
#include <cstdio>
// a sample exported function


extern "C" BOOL APIENTRY DllMain(HMODULE module, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        try {
            DisableThreadLibraryCalls(module);
            MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "ya rly!", MB_ICONINFORMATION + MB_OK);
        }   catch(const char *s) {
            MessageBoxA(NULL,s,"GeHack.dll",MB_ICONERROR);
			return false;
        }
    }
    return TRUE;
}

