#ifndef INJECTIONS_H
#define INJECTIONS_H

#include <windows.h>

PROCESS_INFORMATION* startexe(const char *exefile,char *exestring);
void injectDll(PROCESS_INFORMATION* pi, const char* dllName);

#endif // INJECTIONS_H
