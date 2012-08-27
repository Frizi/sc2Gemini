#ifndef INJECTIONS_H
#define INJECTIONS_H

#include <windows.h>


PROCESS_INFORMATION* startexe(const wchar_t *exefile);
void injectDll(PROCESS_INFORMATION* pi, const wchar_t* dllName);


#endif // INJECTIONS_H
