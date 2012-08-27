// IAT patching class
// by Andy Scott aka xttocs
// modified
///////////////////////////

#ifndef IAT_H
#define IAT_H

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

#endif // IAT_H
