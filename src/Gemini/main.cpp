#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

#include "luautil.h"

void ErrorExit(const char* lpszFunction)
{
    // Retrieve the system error message for the last-error code
    LPVOID lpMsgBuf;
    std::stringstream displayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    // Display the error message and exit the process
    displayBuf << lpszFunction << " failed with error " << dw << ":" << std::endl << (char*)lpMsgBuf;
    MessageBox(NULL, displayBuf.str().c_str(), "oh shi~", MB_OK);
    LocalFree(lpMsgBuf);
    ExitProcess(dw);
}

PROCESS_INFORMATION* startexe(const char *exefile,char *exestring)
{
	//printf("%s\t%s\n",exefile,exestring);
	STARTUPINFO si;
	ZeroMemory(&si,sizeof(si)); // Clean
	si.cb = sizeof(si); // Set size
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION(); // Info
	ZeroMemory(pi,sizeof(pi)); // Clean
	printf("starting %s %s\n",exefile,exestring);
	CreateProcessA(exefile,exestring,NULL,NULL,false,0,NULL,NULL,&si,pi);
	if(!(*pi).hProcess) {
		printf("Could not start %s\n",exefile);
		return pi;
	}
	//ResumeThread(pi.hThread);
	return pi;
}

int lua_startexe(lua_State *l)
{
	const char *exefile = lua_tostring(l,1);
	const char *exestring = lua_tostring(l,2);

	// sometimes malloc+free would corrupt the lua state -> dofile(we.conf.lua) won't work -> no dlls injected -> me starting to hate lua
	//char *mod = (char *)malloc(strlen(exestring));
	char mod[strlen(exestring)];
	strcpy(mod,exestring);
	lua_pushinteger(l,(int)startexe(exefile,mod));
	//free(mod);
	return 1;
}

void injectDll(PROCESS_INFORMATION* pi, const char* dllName)
{
    const uint8_t loadDll[] = { // little endian x86 (_64)
        0x68, 0xEF, 0xBE, 0xAD, 0xDE,   // PUSH 0DEADBEEFh      ; placeholder oldEip
        0x9C,                           // PUSHFD               ; save registers
        0x60,                           // PUSHAD               ;
        0x68, 0xEF, 0xBE, 0xAD, 0xDE,   // PUSH 0DEADBEFh       ; placeholder dllString
        0xB8, 0xEF, 0xBE, 0xAD, 0xDE,   // MOV EAX, 0DEADBEEFh  ; placeholder addr_loadLibraryA
        0xFF, 0xD0,                     // CALL EAX             ; loadLibraryA(&dllString);
        0x61,                           // POPAD                ; restore registers
        0x9D,                           // POPFD                ;
        0xC3                            // RETN                 ; back to oldEip, continue process
    };
    const size_t stubLen = sizeof(loadDll);

    SuspendThread( pi->hThread );

    void* dllString = VirtualAllocEx( pi->hProcess , NULL, (strlen(dllName) + 1), MEM_COMMIT, PAGE_READWRITE);
    void* stub      = VirtualAllocEx( pi->hProcess , NULL, stubLen, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    WriteProcessMemory( pi->hProcess , dllString, dllName, strlen(dllName), NULL);

    size_t oldIP;
    CONTEXT ctx;
    ctx.ContextFlags = CONTEXT_CONTROL;
    GetThreadContext( pi->hThread, &ctx);
    oldIP   = ctx.Eip;
    ctx.Eip = (DWORD)stub;
    ctx.ContextFlags = CONTEXT_CONTROL;

    size_t addr_loadLibraryA = (unsigned long)GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
    DWORD oldprot;
    VirtualProtect((void *)loadDll, stubLen, PAGE_EXECUTE_READWRITE, &oldprot);
    memcpy((void *)((size_t)loadDll + 1), &oldIP, 4);
    memcpy((void *)((size_t)loadDll + 8), &dllString, 4);
    memcpy((void *)((size_t)loadDll + 13), &addr_loadLibraryA, 4);

    WriteProcessMemory( pi->hProcess, stub, (void *)((size_t)loadDll), stubLen, NULL);
    SetThreadContext( pi->hThread, &ctx);
    ResumeThread( pi->hThread );
}

int lua_injectDll(lua_State *l)
{
    PROCESS_INFORMATION* pi = (PROCESS_INFORMATION*) lua_tointeger(l,1);
	const char *dllName = lua_tostring(l,2);
	injectDll(pi, dllName);
    return 1;
}

int lua_freeProcHandle(lua_State *l)
{
    PROCESS_INFORMATION* pi = (PROCESS_INFORMATION*) lua_tointeger(l,1);
    CloseHandle( pi->hProcess );
    CloseHandle( pi->hThread );
    // handle with care :P
    delete pi;
    lua_pushinteger(l,0);
    return 1;
}

int lua_errorExit(lua_State *l)
{
    const char *lpszFunction = lua_tostring(l,1);
    ErrorExit(lpszFunction);
    return 1;
}

int lua_messagebox(lua_State *l)
{
	const char *msg = lua_tostring(l,1);
	const char *title = lua_tostring(l,2);
	MessageBoxA(NULL,msg,title,0);
	return 1;
}

int xargc;
const char **xargv;
char argbuf[8096];

const char *getargs(void)
{
	std::stringstream s;
	for(int i=1;i<xargc;i++) {
		//dprintf("%d %s\n",i,xargv[i]);
		if (strchr(xargv[i],' '))
			s << "\"" << xargv[i] << "\" ";
		else
			s << xargv[i] << " ";
	}
	strcpy(argbuf, s.str().c_str());
	return argbuf;
}

int lua_getargs(lua_State *l)
{
	lua_pushstring(l,getargs());
	return 1;
}


void initluastate(lua_State *lua)
{
	luaL_openlibs(lua);
	lua_newtable(lua);

    // must-have functions
	lua_registert(lua,"errorExit",lua_errorExit);
	lua_registert(lua,"startexe",lua_startexe);
	lua_registert(lua,"freeProcHandle",lua_freeProcHandle);
	lua_registert(lua,"injectDll",lua_injectDll);
	lua_registert(lua,"sleep",lua_sleep);
	lua_registert(lua,"messagebox",lua_messagebox);

    // from grimoire, may be useful
    lua_registert(lua,"getargs",lua_getargs);
    lua_registert(lua,"openlink",lua_openlink);
	lua_registert(lua,"getcwd",lua_getcwd);
	lua_registert(lua,"regopenkey",lua_regopenkey);
	lua_registert(lua,"regdeletekey",lua_regdeletekey);
	lua_registert(lua,"regsetstring",lua_regsetstring);
	lua_registert(lua,"regsetdword",lua_regsetdword);
	lua_registert(lua,"exists",lua_exists);
	lua_registert(lua,"browseforfolder",lua_browseforfolder);
	lua_registert(lua,"getmodulehandle",lua_getmodulehandle);

    lua_setglobal(lua,"gem");
}

//Sends the next arg on the line back
const char *getopt_arg(int &argc, const char **argv, const char *optname)
{
	for(int i=0;i<argc;i++) {
		if(strcmp(optname,argv[i]) == 0) {
			if (i+1 < argc) {
				argc-=2;
				const char *arg = argv[i+1];
				for(int j=i;j<argc;j++) {
					argv[j] = argv[j+2];
					argv[j+2] = argv[j+3];

				}
				return arg;
			}
		}
	}
	return NULL;
}

const char *getscript(int &argc, const char **argv)
{
	static const char def[] = "script.lua";
	const char *s = getopt_arg(argc,argv,"-s");
	if(s == NULL) s = getopt_arg(argc,argv,"--script");
	if(s == NULL) s = def;
	return s;
}

void report_errors(lua_State *lua, int status)
{
	if ( status!=0 ) {
		fprintf(stderr,"-- %s\n", lua_tostring(lua,-1));
		lua_pop(lua, 1); // remove error message
	}
}

int runscript(lua_State *lua, const char *scriptfile)
{
	int s = luaL_loadfile(lua,scriptfile);
	if (s == 0) {
		s = lua_pcall(lua,0,LUA_MULTRET,0);
	}
	report_errors(lua,s);
	return lua_tointeger(lua, lua_gettop(lua));
}

//int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
int main(int argc, const char **argv)
{
    xargc = argc;
    xargv = argv;
    const char *scriptfile = getscript(argc,argv);

    try {
        lua_State *lua = lua_open();
        initluastate(lua);
        runscript(lua, scriptfile);

        lua_close(lua);
    } catch(const char *err) {
		//dprintf("Error: %s\n",err);
		fprintf(stderr,"Error: %s\n",err);
		return 1;
	}
    return 0;
}

