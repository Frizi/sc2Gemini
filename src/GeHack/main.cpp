#include "main.h"
#include <cstdio>
#include "iat.h"

#include <sstream>

#include "../pipe.h"

// a sample exported function

// todo: move additional pipe functions and constants to separated file
PipeClient client("\\\\.\\pipe\\sc2gemini");
const unsigned int maxMessageLen = 4096;

HWND __stdcall CreateWindowExAWrap(DWORD dwExStyle,LPCSTR lpClassName,LPCSTR lpWindowName,DWORD dwStyle,int X,int Y,int nWidth,int nHeight,HWND hWndParent,HMENU hMenu,HINSTANCE hInstance,LPVOID lpParam)
{
	HWND window = CreateWindowExA(dwExStyle,lpClassName,lpWindowName,dwStyle ,X,Y,nWidth,nHeight,hWndParent,hMenu,hInstance,lpParam);

	if(0 == strcmp(lpClassName,"STATIC"));
	else if(0 == strcmp(lpClassName,"BUTTON"));
	else if(0 == strcmp(lpClassName,"COMBOBOX"));
	else if(0 == strcmp(lpClassName,"EDIT"));
    else if(0 == strcmp(lpClassName,"SCROLLBAR"));
	else
	{
        std::stringstream out;
        out << lpClassName << " " << lpWindowName;
	    MessageBoxA(window, out.str().c_str(), "CreateWindowExA", MB_ICONINFORMATION + MB_OK);
	}

    return window;
}

bool redirectRead = false;
char globalMsg[maxMessageLen];

HANDLE __stdcall CreateFileWWrap(WCHAR *fn,DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sec, DWORD disp, DWORD flags, HANDLE templatef)
{
    // anything will be done before executing original CreateFileW

    // buffer for message output
    char msg[maxMessageLen];

    // store filename in buffer as ANSI
	char filename[maxMessageLen];
    WideCharToMultiByte(CP_ACP,0,fn,-1,filename,maxMessageLen,NULL,NULL);
    filename[maxMessageLen-1]=0; // anti overflow

    const size_t strEnd = strlen(filename);
    // pattern of last saved (!! before cleanup) file before .SC2Map save
    const char* pattern = ".SC2Map.temp\\MapScript.galaxy";
    //const char* pattern = ".SC2Map.temp\\";
    const size_t patternLen = strlen(pattern);

    //* // debug filename output
    strcpy(msg, "msg.print.CreateFileW: ");
    strcat(msg,filename);
    client.Write(msg,maxMessageLen);
    //*/

    // match end of string
    if((strEnd >= patternLen) && !memcmp(filename+(strEnd-patternLen), pattern, patternLen))
    // match substring
    //if((strstr(filename, pattern) != NULL))
    {
        redirectRead = true;
        // this is save
        // find temp directory with unpacked map data
        const char* pattern = ".SC2Map.temp\\";
        const size_t patternLen = strlen(pattern);

        const char* found = strstr(filename, pattern);
        unsigned int foundPos = found-filename;
        filename[foundPos + patternLen-1] = 0; // terminate string just before slash

        strcpy(globalMsg, "msg.save.");
        strcat(globalMsg, filename);
        globalMsg[maxMessageLen-1] = 0; // anti overflow

    }
    // finally, execute original funcion
    HANDLE h = CreateFileW(fn,access,share,sec,disp,flags,templatef);
	return h;
}

BOOL WINAPI ReadFileWrap(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
    if(redirectRead)
    {
        redirectRead = false;
        // interrupt and send save message, we got first file read after map save

        client.Write(globalMsg,maxMessageLen);

        // wait for message from external process
        char msg[maxMessageLen];
        strcpy(msg, "msg.aftersave");
        char buf[maxMessageLen];
        do
        {
            client.Read(buf,maxMessageLen);
        } while(strcmp(buf,msg));
    }
    return ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
}


extern "C" BOOL APIENTRY DllMain(HMODULE module, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        try {
            if(!client.Connect())
                throw("Could not initialize client");

            CIAT iat;
            iat.SetParentModule(GetModuleHandle(NULL));

            //if (!iat.LocateForModule("user32.dll"))
			//	throw("Could not locate user32.dll");
            //if(!iat.RedirectImport("CreateWindowExA",(void *)CreateWindowExAWrap))
			//	throw("Could not redirect CreateWindowExA");

            if (!iat.LocateForModule("kernel32.dll"))
				throw("Could not locate kernel32.dll");

			if (!iat.RedirectImport("CreateFileW",(void *)CreateFileWWrap))
				throw("Could not redirect CreateFileW");
			if (!iat.RedirectImport("ReadFile",(void *)ReadFileWrap))
				throw("Could not redirect ReadFile");

            //if (!iat.LocateForModule("storm.dll"))
			//	throw("Could not locate storm.dll");

        }   catch(const char *s) {
            MessageBoxA(NULL,s,"GeHack.dll",MB_ICONERROR);
			return false;
        }
    }
    else if(fdwReason == DLL_PROCESS_DETACH)
    {
        const char* msg = "msg.exit";
        client.Write(msg,strlen(msg)+1);
        client.Disconnect();
    }
    return TRUE;
}

