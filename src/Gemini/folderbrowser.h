#ifndef FOLDERBROWSER_H
#define FOLDERBROWSER_H

#include <windows.h>
#include <shlobj.h>

class FolderBrowser {
private:
    BROWSEINFO bi;
    char folder[MAX_PATH];

public:
    FolderBrowser(HWND owner);
    virtual ~FolderBrowser();

    unsigned long BrowseForFolder(const char *title,  char *buffer);
};

#endif
