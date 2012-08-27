#include <iostream>

#include <windows.h>
#include <shlobj.h>

#include <string>
#include <sstream>
#include <cstdio>

int exists(const wchar_t* fname);
std::wstring browseForFolder(const wchar_t* title);

const wchar_t* drives[] = {
    L"C:",
    L"D:",
};

const wchar_t* guesses[] = {
    L"\\Program Files (x86)\\StarCraft II",
    L"\\Program Files\\StarCraft II",
    L"\\Games\\StarCraft II",
};

const wchar_t* relPath = L"\\Support\\SC2Editor.exe";
const wchar_t* confFileName = L".editorpath";

int main()
{
    bool done = false;

    // try to use saved path if exists
    if(exists(confFileName))
    {
        FILE *confFile = _wfopen(confFileName, L"r");
        wchar_t readBuf[MAX_PATH];
        fgetws(readBuf, MAX_PATH, confFile);

        fclose(confFile);

        if(exists(readBuf))
        {
            std::wcout << readBuf;
            return 0;
        }
    }

    std::wstringstream pathStream;
    // try to guess path
    if(!done)
    {
        size_t drivesCount = (sizeof(drives)/sizeof(drives[0]));
        size_t guessesCount = (sizeof(guesses)/sizeof(guesses[0]));

        for(size_t i = 0; i < drivesCount && !done; i++)
        {
            for(size_t j = 0; j < guessesCount && !done; j++)
            {
                pathStream.str(L"");
                pathStream << drives[i] << guesses[j] << relPath;
                if(exists(pathStream.str().c_str()))
                    done = true;
            }
        }
    }

    // ask user for path
    if(!done)
    {
        pathStream.str(L"");
        pathStream << browseForFolder(L"Select folder containing StarCraft II.exe");
        pathStream << relPath;
        if(exists(pathStream.str().c_str()))
            done = true;
    }

    // save and emit path
    if(done)
    {
        FILE *confFile = _wfopen(confFileName, L"w");

        const wchar_t* widePath = pathStream.str().c_str();
        /*
        const size_t bufSize = wcslen(widePath)+1;
        char* utf8Path = new char[bufSize];

        WideCharToMultiByte(CP_UTF8, 0, widePath, -1, utf8Path, bufSize, NULL, NULL);
        */
        fputws(widePath, confFile);
        fclose(confFile);

        std::wcout << pathStream.str();
    }
    return 0;
}

std::wstring browseForFolder(const wchar_t* title)
{
    BROWSEINFO bi;
    bi.hwndOwner = NULL;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = new wchar_t[MAX_PATH];
	bi.ulFlags = 0 ;
	bi.lpfn = NULL;
    bi.lpszTitle = title;

	memset(bi.pszDisplayName, 0, MAX_PATH * sizeof(wchar_t));
	LPITEMIDLIST lst = SHBrowseForFolder(&bi);
	if (lst != NULL)
    {
		SHGetPathFromIDList(lst, bi.pszDisplayName);
		IMalloc* pMalloc = NULL;
		if(SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free((PVOID)lst);
			pMalloc->Release();
		}
	}

	std::wstring retPath = bi.pszDisplayName;
    delete[] bi.pszDisplayName;

    // remove trailing backslash
    if( wcsncmp(&retPath[retPath.length()-1], L"\\", 1) == 0 )
        retPath.resize(retPath.length()-1);

    return retPath;
}


int exists(const wchar_t*fname)
{
    FILE *f = _wfopen(fname, L"r");
	if(!f) return false;
	fclose(f);
	return true;
}

