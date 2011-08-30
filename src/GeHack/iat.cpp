// this file is from grimoire source
// see humans.txt for more info

// CIAT.cpp: implementation of the CIAT class.
// - Andy Scott aka xttocs
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "iat.h"


CIAT::CIAT()
{
}

CIAT::~CIAT()
{
}

void CIAT::SetParentModule(HMODULE hParent)
{
	m_hModule = hParent;
}

void CIAT::SetParentModule(const char *ParentModuleName)
{
	m_hModule = GetModuleHandle(ParentModuleName);
}

bool CIAT::LocateForModule(const char *ApiModuleName)
{
	return LocateForModule(GetModuleHandle(ApiModuleName));
}

bool CIAT::LocateForModule(HMODULE hApiModule)
{
	m_hApiModule = hApiModule;

	if (hApiModule == 0)
		return false;

	PIMAGE_NT_HEADERS pNTHeader;
	if (PIMAGE_DOS_HEADER(m_hModule)->e_magic != IMAGE_DOS_SIGNATURE)
		return false;

	pNTHeader = PIMAGE_NT_HEADERS(PBYTE(m_hModule) + PIMAGE_DOS_HEADER(m_hModule)->e_lfanew);
	if (pNTHeader->Signature != IMAGE_NT_SIGNATURE)
		return false;

	DWORD importRVA = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	PIMAGE_IMPORT_DESCRIPTOR pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)((ULONG_PTR)(m_hModule) + (ULONG_PTR)(importRVA));

	while(pImportDesc->FirstThunk)
	{
		/*
		PSTR szImportModuleName = (PSTR)((ULONG_PTR)(m_hModule) + (ULONG_PTR)(pImportDesc->Name));
		if (lstrcmpi(szImportModuleName, lpDllName) == 0)
		if (GetModuleHandle(szImportModuleName) == hApiModule)
			break;
		*/
		if (GetModuleHandle((PSTR)((ULONG_PTR)(m_hModule) + (ULONG_PTR)(pImportDesc->Name))) == hApiModule)
			break;

		pImportDesc++;
	}

	if (pImportDesc->FirstThunk == 0)
		return false;

	m_pThunk = (PIMAGE_THUNK_DATA)((PBYTE)m_hModule + (DWORD)pImportDesc->FirstThunk);

	return true;
}

bool CIAT::RedirectImport(int ApiOrdinal, void *NewLocation)
{
	return RedirectImport((const char*)(ULONG_PTR)ApiOrdinal, NewLocation);
}

bool CIAT::RedirectImport(const char *Api, void *NewLocation)
{
	ULONG_PTR NewAddress = (ULONG_PTR)NewLocation;
	ULONG_PTR RoutineAddress = (ULONG_PTR)GetProcAddress(m_hApiModule, Api);
	PIMAGE_THUNK_DATA pThunk = m_pThunk;

	if (RoutineAddress == 0)
		return false;

	while (pThunk->u1.Function)
	{
		ULONG_PTR* Address = (ULONG_PTR*)&pThunk->u1.Function;
		if (*Address == RoutineAddress)
		{
			DWORD dwOldProtect = 0, dwOldProtect2 = 0;
			VirtualProtect((void*)Address, 4, PAGE_WRITECOPY, &dwOldProtect);
			*Address = NewAddress;
			VirtualProtect((void*)Address, 4, dwOldProtect, &dwOldProtect2);
			return true;
		}
		pThunk++;
	}
	return false;
}
