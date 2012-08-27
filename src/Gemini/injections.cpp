#include "injections.h"

#include <iostream>

PROCESS_INFORMATION* startexe(const wchar_t *exefile)
{
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si)); // Clean
	si.cb = sizeof(si); // Set size
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION(); // Info
	ZeroMemory(pi,sizeof(pi)); // Clean

#ifdef DEBUG
	std::wcout << L"Starting " << exefile << std::endl;
#endif

	CreateProcess(exefile,NULL,NULL,NULL,false,0,NULL,NULL,&si,pi);
	if(!(*pi).hProcess) {
        std::wcout << L"Could not start" << exefile << std::endl;
		return pi;
	}
	//ResumeThread(pi.hThread);
	return pi;
}

//*
void injectDll(PROCESS_INFORMATION* pi, const wchar_t* dllName)
{
    try {
        const size_t stringBufLen = wcslen(dllName) * sizeof(wchar_t) + 1;

        void* addr_dllString = VirtualAllocEx( pi->hProcess , NULL, stringBufLen, MEM_COMMIT, PAGE_READWRITE);
        WriteProcessMemory( pi->hProcess , addr_dllString, dllName, stringBufLen, NULL);
        size_t addr_loadLibrary = (size_t) GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");

        HANDLE hThread;
        DWORD hLibModule;

        hThread = CreateRemoteThread(pi->hProcess, NULL, 0,
                                     (LPTHREAD_START_ROUTINE) addr_loadLibrary,
                                     addr_dllString, 0, NULL);
        WaitForSingleObject(hThread, INFINITE);
        GetExitCodeThread(hThread, &hLibModule);
        CloseHandle(hThread);
        VirtualFreeEx(pi->hProcess, addr_dllString, stringBufLen, MEM_RELEASE);

    } catch(const char* err) {
        std::wcerr << "Error while injecting " << dllName << ": " << err;
        return;
    }
}
/*/
void injectDll(PROCESS_INFORMATION* pi, const wchar_t* dllName)
{
    try {
        const char loadDll[] = { // little endian x86 (_64)
            0x68, 0xEF, 0xBE, 0xAD, 0xDE,   // PUSH 0DEADBEEFh      ; placeholder oldEip
            0x9C,                           // PUSHFD               ; save registers
            0x60,                           // PUSHAD               ;
            0x68, 0xEF, 0xBE, 0xAD, 0xDE,   // PUSH 0DEADBEFh       ; placeholder dllString
            0xB8, 0xEF, 0xBE, 0xAD, 0xDE,   // MOV EAX, 0DEADBEEFh  ; placeholder addr_loadLibrary
            0xFF, 0xD0,                     // CALL EAX             ; loadLibrary(&dllString);
            0x61,                           // POPAD                ; restore registers
            0x9D,                           // POPFD                ;
            0xC3                            // RETN                 ; back to oldEip, continue process
        };
        const size_t stubLen = sizeof(loadDll);

        SuspendThread( pi->hThread );

        void* dllString = VirtualAllocEx( pi->hProcess , NULL, (wcslen(dllName) * sizeof(wchar_t) + 1), MEM_COMMIT, PAGE_READWRITE);
        void* stub      = VirtualAllocEx( pi->hProcess , NULL, stubLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        WriteProcessMemory( pi->hProcess , dllString, dllName, wcslen(dllName) * sizeof(wchar_t) + 1, NULL);

        size_t oldIP;
        CONTEXT ctx;
        ctx.ContextFlags = CONTEXT_CONTROL;
        GetThreadContext( pi->hThread, &ctx);
        oldIP   = ctx.Eip;
        ctx.Eip = (DWORD)stub;
        ctx.ContextFlags = CONTEXT_CONTROL;

        size_t addr_loadLibrary = (size_t) GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryW");
        DWORD oldprot;
        VirtualProtect((void *)loadDll, stubLen, PAGE_EXECUTE_READWRITE, &oldprot);
        memcpy((void *)((size_t)loadDll + 1), &oldIP, 4);
        memcpy((void *)((size_t)loadDll + 8), &dllString, 4);
        memcpy((void *)((size_t)loadDll + 13), &addr_loadLibrary, 4);

        WriteProcessMemory( pi->hProcess, stub, (void *)((size_t)loadDll), stubLen, NULL);
        SetThreadContext( pi->hThread, &ctx);
        ResumeThread( pi->hThread );
    } catch(const char* err) {
        std::wcerr << "Error while injecting " << dllName << ": " << err;
        return;
    }
}
//*/
