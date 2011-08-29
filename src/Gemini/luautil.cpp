#include <windows.h>
#include <unistd.h>
#include <string.h>

#include "luautil.h"
//#include "libbs.h"
#include "folderbrowser.h"

/*
int lua_patchdword(lua_State *l)
{
	HANDLE proc = (HANDLE)lua_tointeger(l,1);
	int addr = lua_tointeger(l,2);
	int dword = lua_tointeger(l,3);
	inject(proc,addr,&dword,4);
	lua_pushinteger(l,0);
	return 1;
}

int lua_patchbyte(lua_State *l)
{
	HANDLE proc = (HANDLE)lua_tointeger(l,1);
	int addr = lua_tointeger(l,2);
	char byte = lua_tointeger(l,3);
	inject(proc,addr,&byte,1);
	lua_pushinteger(l,0);
	return 1;
}

int lua_patchstr(lua_State *l)
{
	HANDLE proc = (HANDLE)lua_tointeger(l,1);
	int addr = lua_tointeger(l,2);
	const char *str = lua_tostring(l,3);
	inject(proc,addr,str,strlen(str));
	lua_pushinteger(l,0);
	return 1;
}
*/

int lua_sleep(lua_State *l)
{
	int ms = lua_tointeger(l,1);
	Sleep(ms);
	lua_pushinteger(l,0);
	return 1;
}

HKEY convertkeyname(const char *&path) {
	char *pos = strstr(path, "\\");
	char base[30];
	if (!pos)
		return HKEY_CURRENT_USER;
	memset(base, 0, 30);
	memcpy(base, path, pos - path);
	if (strcmp(base, "HKEY_CLASSES_ROOT") == 0) {
		path = pos + 1;
		return HKEY_CLASSES_ROOT;
	} else if (strcmp(base, "HKEY_CURRENT_USER") == 0) {
		path = pos + 1;
		return HKEY_CURRENT_USER;
	} else if (strcmp(base, "HKEY_LOCAL_MACHINE") == 0) {
		path = pos + 1;
		return HKEY_LOCAL_MACHINE;
	} else if (strcmp(base, "HKEY_USERS") == 0) {
		path = pos + 1;
		return HKEY_USERS;
	} else if (strcmp(base, "HKEY_PERFORMANCE_DATA") == 0) {
		path = pos + 1;
		return HKEY_PERFORMANCE_DATA;
	} else if (strcmp(base, "HKEY_CURRENT_CONFIG") == 0) {
		path = pos + 1;
		return HKEY_CURRENT_CONFIG;
	} else if (strcmp(base, "HKEY_DYN_DATA") == 0) {
		path = pos + 1;
		return HKEY_DYN_DATA;
	}
	return HKEY_CURRENT_USER;
}

bool regKeyExists(const char *keyname,const char *valuename)
{
    int ret;
    HKEY key;
    try {
		HKEY base = convertkeyname(keyname);
		ret = RegOpenKeyEx(base,keyname,0,KEY_QUERY_VALUE,&key);
		if(ret != ERROR_SUCCESS)
            return 0;
        else
            return 1;
    }catch(const char *error) {
        printf("Attempt to check if %s\\%s exists failed: %s\n",keyname,valuename,error);
    }
    return 0;
}


DWORD regopenkey(const char *keyname,const char *valuename,void *out)
{
	int ret;
	HKEY key;
	DWORD len=1024;
	try {
		HKEY base = convertkeyname(keyname);
		ret = RegOpenKeyEx(base,keyname,0,KEY_QUERY_VALUE,&key);
		if(ret != ERROR_SUCCESS || !key) throw("Could not open registry key");
		DWORD keytype = 0;
		ret = RegQueryValueExA(key,valuename,NULL,&keytype,(LPBYTE)out,&len);
		RegCloseKey(key);
		if(ret != ERROR_SUCCESS) throw("Could not open registry value");
		if(keytype == REG_SZ) printf("found %s\\%s : %s\n",keyname,valuename,(char *)out);
		else if(keytype == REG_DWORD) printf("found %s\\%s : %d\n",keyname,valuename,*(int *)out);
		return keytype;
	}catch(const char *error) {
		printf("Attempt to open %s\\%s failed: %s\n",keyname,valuename,error);
/*		char buf[1024];
		sprintf(buf,"Attempt to open key %s\\%s failed: %s\n",keyname,valuename,error);
		MessageBox(NULL,buf,"exehack.exe",MB_ICONERROR);*/
		return 0;
	}
}

int lua_regKeyExists(lua_State *l)
{
	const char *keyname = lua_tostring(l,1);
	const char *value = lua_tostring(l,2);
	lua_pushboolean(l,regKeyExists(keyname,value));
	return 1;
}

int lua_regopenkey(lua_State *l)
{
	const char *keyname = lua_tostring(l,1);
	const char *value = lua_tostring(l,2);
	char buf[1024];
	int ret = regopenkey(keyname,value,buf);
	switch(ret) {
		case REG_SZ:	lua_pushstring(l,buf);break;
		case REG_DWORD:	lua_pushinteger(l,*(int *)buf);break;
		default:	lua_pushinteger(l,0);break;
	}
	return 1;
}

int lua_regdeletekey(lua_State *l)
{
	const char *keyname = lua_tostring(l,1);
	HKEY base = convertkeyname(keyname);
	RegDeleteKey(base,keyname);
	return 1;
}

int lua_regsetstring(lua_State *l)
{
	const char *keyname = lua_tostring(l,1);
	const char *value = lua_tostring(l,2);
	const char *str =lua_tostring(l,3);
	HKEY key;
	try {
		HKEY base = convertkeyname(keyname);
		int ret = RegOpenKeyEx(base,keyname,0,KEY_WRITE,&key);
		if(ret != ERROR_SUCCESS || !key)
			if (RegCreateKey(base, keyname, &key) != ERROR_SUCCESS || !key)
				throw("Could not open registry key");
		ret = RegSetValueEx(key,value,0,REG_SZ,(const BYTE *)str,strlen(str)+1);
		RegCloseKey(key);
		if(ret != ERROR_SUCCESS) throw("Could not write registry value");
	}
	catch(const char *error) {
		printf("Attempt to set %s\\%s failed: %s\n",keyname,value,error);
	}
	return 1;
}

int lua_regsetdword(lua_State *l)
{
	const char *keyname = lua_tostring(l,1);
	const char *value = lua_tostring(l,2);
	int dword = lua_tointeger(l,3);
	HKEY key;
	try {
		HKEY base = convertkeyname(keyname);
		int ret = RegOpenKeyEx(base,keyname,0,KEY_WRITE,&key);
		if(ret != ERROR_SUCCESS || !key)
			if (RegCreateKey(base, keyname, &key) != ERROR_SUCCESS || !key)
				throw("Could not open registry key");
		ret = RegSetValueEx(key,value,0,REG_DWORD,(BYTE *)&dword,sizeof(int));
		RegCloseKey(key);
		if(ret != ERROR_SUCCESS) throw("Could not write registry value");
	}
	catch(const char *error) {
		printf("Attempt to set %s\\%s failed: %s\n",keyname,value,error);
	}
	return 1;
}

int exists(const char *fname)
{
	FILE *f = fopen(fname,"r");
	if(!f) return false;
	fclose(f);
	return true;
}

int lua_exists(lua_State *l)
{
	const char *fname = lua_tostring(l,1);
	lua_pushboolean(l,exists(fname));
	return 1;
}

int lua_getcwd(lua_State *l)
{
	char path[1024];
	getcwd(path,1024);
	lua_pushstring(l,path);
	return 1;
}

int lua_setcwd(lua_State *l) {
	const char *path = lua_tostring(l,1);
	SetCurrentDirectory(path);
	return 1;
}

int lua_browseforfolder(lua_State *l)
{
	const char *title = lua_tostring(l,1);
	char path[1024];
	getcwd(path,1024);
	FolderBrowser br(NULL);
	char buffer[MAX_PATH];
	if(br.BrowseForFolder(title,buffer) > 0) {
		SetCurrentDirectory(path);
		lua_pushstring(l,buffer);
	} else {
		SetCurrentDirectory(path);
		lua_pushstring(l,"");
	}
	return 1;
}

int lua_getmodulehandle(lua_State *l)
{
	const char *name = lua_tostring(l,1);
	lua_pushinteger(l,(int)GetModuleHandle(name));
	return 1;
}

/*
typedef BOOL __stdcall (* pSFileOpenArchive)(const char *fn, long prio, long flags, long *mpq);
typedef void __stdcall (* pStoreMpqHandle)(long mpqhandle, const char* archive);
int lua_loadmpq(lua_State *l)
{
	static pSFileOpenArchive SFileOpenArchive = NULL;
	static pStoreMpqHandle StoreMpqHandle = NULL;
	static int firsttime = true;
	const int prio = lua_tointeger(l,1);
	const char *mpqname = lua_tostring(l,2);
	long mpq;
	try {
		if(SFileOpenArchive == NULL) {
			if(firsttime) {
				HMODULE storm;
				while(1) {
					storm = GetModuleHandle("storm.dll");
					if(storm != NULL) break;
					Sleep(1);
				}
				//if(!storm) throw("Could not locate storm.dll for grim.loadmpq");
				SFileOpenArchive = (pSFileOpenArchive)GetProcAddress(storm,(const char *)266);
				if(!SFileOpenArchive) throw("Could not find SFileOpenArchive in storm for grim.loadmpq");

			} else throw((const char *)NULL);
		}
		if(!SFileOpenArchive(mpqname,prio,0,&mpq) || !mpq) {
			char buf[1024];
			sprintf(buf,"Could not open archive %s",mpqname);
			throw(buf);
		}
		//dprintf("Loaded %s\n",mpqname);
		if (StoreMpqHandle == NULL) {
			HMODULE wehack = GetModuleHandle("wehack.dll");
			StoreMpqHandle = (pStoreMpqHandle)GetProcAddress(wehack,"StoreMpqHandle");
		}
		if (StoreMpqHandle)
			StoreMpqHandle(mpq, mpqname);
		lua_pushboolean(l,true);
	} catch(const char *err) {
		if(err != NULL)
			MessageBoxA(NULL,err,"Lua Utils",MB_ICONERROR);
		lua_pushboolean(l,false);
	}


	firsttime = false;
	return 1;
}

int lua_log(lua_State *l)
{
	const char *logstr = lua_tostring(l,1);
	dprintf("%s\n", logstr);
	return 1;
}

int lua_logf(lua_State *l) {
	int n = lua_gettop(l); // nr of args
	for (int i = 1; i <= n; i++)
		dprintf("%s\n", lua_tostring(l, i));
	return 1;
}

int lua_logt(lua_State *l) {
	// table is on the stack at index 1
	lua_pushnil(l);  // signal start of a traversal
	while(lua_next(l, 1) != 0) {
		// key is at -2 and value at -1 in the stack now
		dprintf("%s\n", lua_tostring(l, -1));
		lua_pop(l, 1); // pops value but keeps key for the next iteration
	}
	return 1;
}
*/

int lua_openlink(lua_State *l) {
	const char* link = lua_tostring(l,1);
	ShellExecute(NULL, NULL, link, NULL, NULL, SW_SHOW);
	return 1;
}
