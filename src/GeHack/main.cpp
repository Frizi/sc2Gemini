#include "main.h"
#include <cstdio>
#include "iat.h"

#include <sstream>


HANDLE __stdcall CreateFileWWrap(WCHAR *filename,DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sec, DWORD disp, DWORD flags, HANDLE templatef)
{
    // anything will be done before executing original CreateFileW
    // goal is to detect map save and send on message on next read



    //bool triggered = false;

    //WideCharToMultiByte(CP_ACP,0,fn,-1,filename,maxMessageLen,NULL,NULL);



    /* // debug filename output
    strcpy(msg, "msg.print.CreateFileW: ");
    strcat(msg,filename);
    if(access & GENERIC_READ)
        strcat(msg, " r");
    if(access & GENERIC_WRITE)
        strcat(msg, " w");
    client.Write(msg,maxMessageLen);
    //*/

    if((!(access & GENERIC_WRITE)) && (access & GENERIC_READ)) // this handle should have only read permissions
    {
        // buffer for message output
        // store filename in buffer as ANSI

        const size_t strEnd = wcslen(filename);
        // pattern of last accessed file for save before .SC2Map packing
        const wchar_t* pattern = L".SC2Map.temp\\ComponentList.SC2Components";
        //const char* pattern = ".SC2Map.temp\\";
        const size_t patternLen = wcslen(pattern);



        // match end of string
        if((strEnd >= patternLen) && !memcmp(filename+(strEnd-patternLen), pattern, patternLen))
        // match substring
        //if((strstr(filename, pattern) != NULL))
        {
            //triggered = true;
            //redirectRead = true;
            // this is save
            // find temp directory with unpacked map data
            const wchar_t* pattern = L".SC2Map.temp\\";
            const size_t patternLen = wcslen(pattern);

            const wchar_t* found = wcsstr(filename, pattern);
            unsigned int foundPos = found-filename;
            filename[foundPos + patternLen-1] = 0; // terminate string just before slash


            // wait for message from external process
            MessageBox(NULL,L"Components ready to edit",L"GeHack.dll",MB_ICONEXCLAMATION);
        }
    }
    // finally, execute original funcion
    HANDLE h = CreateFileW(filename,access,share,sec,disp,flags,templatef);

    // and save temps if triggered
    /*
    if(triggered)
    {
        tempFileHandle = h;
        tempFn = fn;
        tempAccess = access;
        tempShare = share;
        tempSec = sec;
        tempDisp = disp;
        tempFlags = flags;
        tempTemplatef = templatef;
    }
    //*/

	return h;
}

DWORD ThreadMain(void* param)
{
    try {
            CIAT iat;
            iat.SetParentModule(GetModuleHandle(NULL));
            //iat.SetParentModule(module);

            HMODULE hKernel = GetModuleHandle(L"kernel32.dll");

            if(!hKernel)
                throw("kernel32.dll handle is 0");
            if( !iat.LocateForModule(hKernel) )
				throw(L"Could not locate kernel32");
			if( !iat.RedirectImport("CreateFileW",(void *)CreateFileWWrap) )
				throw(L"Could not redirect CreateFileW");

        }   catch(const wchar_t *s) {
            MessageBox(NULL,s,L"GeHack.dll",MB_ICONERROR);
			return false;
        }

        MessageBox(NULL,L"GeHack thread injection created",L"GeHack.dll",MB_ICONEXCLAMATION);
        return 0;
}


extern "C" BOOL APIENTRY DllMain(HMODULE module, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadMain, NULL, 0, NULL);
    }
    else if(fdwReason == DLL_PROCESS_DETACH)
    {
        //const char* msg = "msg.exit";
        //client.Write(msg,strlen(msg)+1);
        //client.Disconnect();
    }
    return TRUE;
}

