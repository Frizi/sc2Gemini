#include "main.h"
#include <cstdio>
#include "iat.h"

#include <sstream>


/*
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
*/

/*
bool redirectRead = false;
char globalMsg[maxMessageLen];

HANDLE tempFileHandle;
WCHAR *tempFn;
DWORD tempAccess;
DWORD tempShare;
LPSECURITY_ATTRIBUTES tempSec;
DWORD tempDisp;
DWORD tempFlags;
HANDLE tempTemplatef;
*/

/*
HANDLE __stdcall CreateFileWWrap(WCHAR *fn,DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sec, DWORD disp, DWORD flags, HANDLE templatef)
{
    // anything will be done before executing original CreateFileW
    // goal is to detect map save and send on message on next read


    //bool triggered = false;

    //WideCharToMultiByte(CP_ACP,0,fn,-1,filename,maxMessageLen,NULL,NULL);



    //* // debug filename output
    strcpy(msg, "msg.print.CreateFileW: ");
    strcat(msg,filename);
    if(access & GENERIC_READ)
        strcat(msg, " r");
    if(access & GENERIC_WRITE)
        strcat(msg, " w");
    client.Write(msg,maxMessageLen);


    if((!(access & GENERIC_WRITE)) && (access & GENERIC_READ)) // this handle should have only read permissions
    {
        // buffer for message output
        // store filename in buffer as ANSI

        const size_t strEnd = strlen(filename);
        // pattern of last accessed file for save before .SC2Map packing
        const char* pattern = ".SC2Map.temp\\ComponentList.SC2Components";
        //const char* pattern = ".SC2Map.temp\\";
        const size_t patternLen = strlen(pattern);



        // match end of string
        if((strEnd >= patternLen) && !memcmp(filename+(strEnd-patternLen), pattern, patternLen))
        // match substring
        //if((strstr(filename, pattern) != NULL))
        {
            //triggered = true;
            //redirectRead = true;
            // this is save
            // find temp directory with unpacked map data
            const char* pattern = ".SC2Map.temp\\";
            const size_t patternLen = strlen(pattern);

            const char* found = strstr(filename, pattern);
            unsigned int foundPos = found-filename;
            filename[foundPos + patternLen-1] = 0; // terminate string just before slash

            strcpy(msg, "msg.save.");
            strcat(msg, filename);
            msg[maxMessageLen-1] = 0; // anti overflow
            client.Write(msg,maxMessageLen);

            // wait for message from external process
            char msg[maxMessageLen];
            strcpy(msg, "msg.aftersave");
            char buf[maxMessageLen];
            do
            {
                client.Read(buf,maxMessageLen);
            } while(strcmp(buf,msg));
        }
    }
    // finally, execute original funcion
    HANDLE h = CreateFileW(fn,access,share,sec,disp,flags,templatef);

    // and save temps if triggered
    //*
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

	return h;
}
*/
/*
BOOL WINAPI ReadFileWrap(HANDLE hFile,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPOVERLAPPED lpOverlapped)
{
    if(redirectRead)
    {
        redirectRead = false;
        // interrupt and send save message, we got first file read after map save
        // close handle, because we need write to it
        //CloseHandle(hFile);
        client.Write(globalMsg,maxMessageLen);

        // wait for message from external process
        char msg[maxMessageLen];
        strcpy(msg, "msg.aftersave");
        char buf[maxMessageLen];
        do
        {
            client.Read(buf,maxMessageLen);
        } while(strcmp(buf,msg));

        // reopen file handle
        //hFile = CreateFileW(tempFn,tempAccess,tempShare,tempSec,tempDisp,tempFlags,tempTemplatef);
    }
    return ReadFile(hFile,lpBuffer,nNumberOfBytesToRead,lpNumberOfBytesRead,lpOverlapped);
}
*/

extern "C" BOOL APIENTRY DllMain(HMODULE module, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        MessageBox(NULL,L"GeHack injection successful",L"GeHack.dll",MB_ICONEXCLAMATION);
        try {
            /*
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
            //if (!iat.RedirectImport("CreateDirectoryW",(void *)CreateDirectoryWWrap))
			//	throw("Could not redirect CreateDirectoryW");
			if (!iat.RedirectImport("CreateFileW",(void *)CreateFileWWrap))
				throw("Could not redirect CreateFileW");
			//if (!iat.RedirectImport("ReadFile",(void *)ReadFileWrap))
			//	throw("Could not redirect ReadFile");

            //if (!iat.LocateForModule("storm.dll"))
			//	throw("Could not locate storm.dll");
			*/

        }   catch(const char *s) {
            MessageBoxA(NULL,s,"GeHack.dll",MB_ICONERROR);
			return false;
        }
    }
    else if(fdwReason == DLL_PROCESS_DETACH)
    {
        //const char* msg = "msg.exit";
        //client.Write(msg,strlen(msg)+1);
        //client.Disconnect();
    }
    return TRUE;
}

