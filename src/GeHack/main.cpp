#include "main.h"
#include <cstdio>
#include "iat.h"

#include <sstream>
// a sample exported function


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

HANDLE __stdcall CreateFileWWrap(WCHAR *fn,DWORD access, DWORD share, LPSECURITY_ATTRIBUTES sec, DWORD disp, DWORD flags, HANDLE templatef)
{
	HANDLE h = CreateFileW(fn,access,share,sec,disp,flags,templatef);
	MessageBoxW(NULL, fn, L"CreateFileW", MB_ICONINFORMATION + MB_OK);
	return h;
}

extern "C" BOOL APIENTRY DllMain(HMODULE module, DWORD fdwReason, LPVOID lpvReserved)
{
    if(fdwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(module);
        try {
            //MessageBoxA(NULL, "DLL_PROCESS_ATTACH", "ya rly!", MB_ICONINFORMATION + MB_OK);

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





        }   catch(const char *s) {
            MessageBoxA(NULL,s,"GeHack.dll",MB_ICONERROR);
			return false;
        }
    }
    return TRUE;
}

