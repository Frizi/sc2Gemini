#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

#include <lua.h>

HINSTANCE hInst;

//const DWORD addrCreateFile = 0x00cb20ea;

void ErrorExit(const char* lpszFunction);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    const uint8_t loadDll[] = { // little endian x86 (_64)
        0x68, 0xEF, 0xBE, 0xAD, 0xDE,   // PUSH 0DEADBEEFh      ; placeholder oldEip
        0x9C,                           // PUSHFD               ; save registers
        0x60,                           // PUSHAD               ;
        0x68, 0xEF, 0xBE, 0xAD, 0xDE,   // PUSH 0DEADBEFh       ; placeholder dllString
        0xB8, 0xEF, 0xBE, 0xAD, 0xDE,   // MOV EAX, 0DEADBEEFh  ; placeholder addr_loadLibraryA
        0xFF, 0xD0,                     // CALL EAX             ; loadLibraryA(&dllString);
        0x61,                           // POPAD                ; restore registers
        0x9D,                           // POPFD                ;
        0xC3                            // RETN                 ; back to oldEip, continue process
    };
    const size_t stubLen = sizeof(loadDll);

    // open editor as child process
    std::stringstream sstream;

    std::string editorPath = "Support\\SC2Editor.exe";

    CONTEXT ctx;
    PROCESS_INFORMATION xProcInfo;
    STARTUPINFOA xStartupInfo;
    memset(&xStartupInfo, 0, sizeof(xStartupInfo));
    memset(&xProcInfo, 0, sizeof(xProcInfo));
    xStartupInfo.cb = sizeof(xStartupInfo);
    CreateProcessA(editorPath.c_str(), GetCommandLineA(), NULL, NULL, NULL, NULL, NULL, NULL, &xStartupInfo, &xProcInfo);
    if(!xProcInfo.hProcess)
    {
        ErrorExit("CreateProcessA");
    }
    // let it run for a while
    Sleep(5);
    SuspendThread( xProcInfo.hThread );
    // inject dll :)

    char* dllName = (char*)"GeHack.dll";
    void* dllString = VirtualAllocEx( xProcInfo.hProcess , NULL, (strlen(dllName) + 1), MEM_COMMIT, PAGE_READWRITE);
    void* stub      = VirtualAllocEx( xProcInfo.hProcess , NULL, stubLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    WriteProcessMemory( xProcInfo.hProcess , dllString, dllName, strlen(dllName), NULL);

    size_t oldIP;
    ctx.ContextFlags = CONTEXT_CONTROL;
    GetThreadContext( xProcInfo.hThread, &ctx);
    oldIP   = ctx.Eip;
    ctx.Eip = (DWORD)stub;
    ctx.ContextFlags = CONTEXT_CONTROL;

    size_t addr_loadLibraryA = (unsigned long)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    DWORD oldprot;
    VirtualProtect((void *)loadDll, stubLen, PAGE_EXECUTE_READWRITE, &oldprot);
    memcpy((void *)((size_t)loadDll + 1), &oldIP, 4);
    memcpy((void *)((size_t)loadDll + 8), &dllString, 4);
    memcpy((void *)((size_t)loadDll + 13), &addr_loadLibraryA, 4);

    for(unsigned int i=0; i<stubLen; i++)
    {
        sstream << (uint16_t)(loadDll[i]) << std::endl;
    }
    MessageBox(NULL, sstream.str().c_str(), "Information", MB_ICONINFORMATION + MB_OK);

    WriteProcessMemory( xProcInfo.hProcess, stub, (void *)((size_t)loadDll), stubLen, NULL);
    SetThreadContext( xProcInfo.hThread, &ctx);
    ResumeThread( xProcInfo.hThread );
    return 0;
}

void ErrorExit(const char* lpszFunction)
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    std::stringstream displayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process
    displayBuf << lpszFunction << " failed with error " << dw << ":" << std::endl << (char*)lpMsgBuf;
    MessageBox(NULL, displayBuf.str().c_str(), "oh shi~", MB_OK);
    LocalFree(lpMsgBuf);
    ExitProcess(dw);
}

