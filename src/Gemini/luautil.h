#ifndef __LUAUTIL_H__
#define __LUAUTIL_H__

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#define lua_registert(L,n,f) (lua_pushstring(L,n), lua_pushcfunction(L,f), lua_settable(L,-3))

DWORD regopenkey(const char *keyname,const char *valuename,void *out);

int lua_getmodulehandle(lua_State *l);
int lua_browseforfolder(lua_State *l);
int lua_patchdword(lua_State *l);
int lua_patchbyte(lua_State *l);
int lua_sleep(lua_State *l);
int lua_patchstr(lua_State *l);
int lua_regopenkey(lua_State *l);
int lua_exists(lua_State *l);
int lua_getcwd(lua_State *l);
int lua_setcwd(lua_State *l);
int lua_regsetstring(lua_State *l);
int lua_regsetdword(lua_State *l);
int lua_regdeletekey(lua_State *l);
int lua_loadmpq(lua_State *l);
int lua_log(lua_State *l);
int lua_logf(lua_State *l);
int lua_logt(lua_State *l);
int lua_openlink(lua_State *l);

#endif
