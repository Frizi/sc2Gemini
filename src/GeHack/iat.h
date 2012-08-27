// this file is from grimoire source
// see humans.txt for more info

// IAT patching class
// - Andy Scott aka xttocs
//////////////////////////////////////////////////////////////////////

#pragma once

class CIAT
{

private:
	HMODULE m_hModule;
	HMODULE m_hApiModule;
	PIMAGE_THUNK_DATA m_pThunk;

public:
	CIAT();
	virtual ~CIAT();
	void SetParentModule(HMODULE hParent);
	void SetParentModule(const wchar_t *ParentModuleName);
	bool LocateForModule(const wchar_t *ApiModuleName);
	bool LocateForModule(HMODULE hApiModule);
	bool RedirectImport(int ApiOrdinal, void *NewAddress);
	bool RedirectImport(const char *Api, void *NewAddress);

};

