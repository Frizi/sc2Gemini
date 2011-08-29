#include "injections.h"

#include <cstdio>

PROCESS_INFORMATION* startexe(const char *exefile,char *exestring)
{
	//printf("%s\t%s\n",exefile,exestring);
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si)); // Clean
	si.cb = sizeof(si); // Set size
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION(); // Info
	ZeroMemory(pi,sizeof(pi)); // Clean
	printf("starting %s %s\n",exefile,exestring);
	CreateProcessA(exefile,exestring,NULL,NULL,false,0,NULL,NULL,&si,pi);
	if(!(*pi).hProcess) {
		printf("Could not start %s\n",exefile);
		return pi;
	}
	//ResumeThread(pi.hThread);
	return pi;
}

void injectDll(PROCESS_INFORMATION* pi, const char* dllName)
{
    try {
        const char loadDll[] = { // little endian x86 (_64)
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

        SuspendThread( pi->hThread );

        void* dllString = VirtualAllocEx( pi->hProcess , NULL, (strlen(dllName) + 1), MEM_COMMIT, PAGE_READWRITE);
        void* stub      = VirtualAllocEx( pi->hProcess , NULL, stubLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

        WriteProcessMemory( pi->hProcess , dllString, dllName, strlen(dllName), NULL);

        size_t oldIP;
        CONTEXT ctx;
        ctx.ContextFlags = CONTEXT_CONTROL;
        GetThreadContext( pi->hThread, &ctx);
        oldIP   = ctx.Eip;
        ctx.Eip = (DWORD)stub;
        ctx.ContextFlags = CONTEXT_CONTROL;

        size_t addr_loadLibraryA = (unsigned long)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
        DWORD oldprot;
        VirtualProtect((void *)loadDll, stubLen, PAGE_EXECUTE_READWRITE, &oldprot);
        memcpy((void *)((size_t)loadDll + 1), &oldIP, 4);
        memcpy((void *)((size_t)loadDll + 8), &dllString, 4);
        memcpy((void *)((size_t)loadDll + 13), &addr_loadLibraryA, 4);

        WriteProcessMemory( pi->hProcess, stub, (void *)((size_t)loadDll), stubLen, NULL);
        SetThreadContext( pi->hThread, &ctx);
        ResumeThread( pi->hThread );
    } catch(const char* err) {
        fprintf(stderr,"Error while injecting %s: %s\n",dllName,err);
        return;
    }
}
