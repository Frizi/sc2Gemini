#include "folderbrowser.h"

FolderBrowser::FolderBrowser(HWND owner)
{
	bi.hwndOwner = owner;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = folder;
	bi.ulFlags = 0;
	bi.lpfn = NULL;
}

FolderBrowser::~FolderBrowser()
{
}

unsigned long FolderBrowser::BrowseForFolder(const char *title, char *buffer) {
	bi.lpszTitle = title;
	memset(folder, 0, MAX_PATH * sizeof(char));
	LPITEMIDLIST lst = SHBrowseForFolder(&bi);
	if (lst == NULL)
		memset(folder, 0, MAX_PATH * sizeof(char));
	else {
		SHGetPathFromIDList(lst, folder);
		IMalloc* pMalloc = NULL;
		if(SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free((PVOID)lst);
			pMalloc->Release();
		} 
	}
	strncpy(buffer,folder,MAX_PATH);
	unsigned buflen = strlen(buffer);
	if (buflen > 0 && buflen < MAX_PATH - 1) {
		if (buffer[buflen - 1] == '\\') {
			buffer[buflen - 1] = 0;
			buflen--;
		} else {
			buffer[buflen] = 0;
		}
		return buflen;
	}
	return 0;
}
