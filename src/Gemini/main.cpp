#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#include "resource.h"
#include <string>
#include <sstream>
#include <cstdio>
#include <cstdlib>

#include "luautil.h"
#include "injections.h"

#include "../pipe.h"

PipeServer server("\\\\.\\pipe\\sc2gemini");

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


int lua_startexe(lua_State *l)
{
	const char *exefile = lua_tostring(l,1);
	const char *exestring = lua_tostring(l,2);

	char mod[strlen(exestring)];
	strcpy(mod,exestring);
	lua_pushinteger(l,(int)startexe(exefile,mod));

	return 1;
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

int lua_connectServer(lua_State *l)
{
    static bool connected = false;
    if(!connected)
    {
        try
        {
            if(!server.Connect())
                throw("Failed to initialize pipe\n");
            connected = true;
        } catch(const char* err) {
            fprintf(stderr,"Error: %s\n",err);
            return -1;
        }
    }
    return 1;
}

int lua_readMessage(lua_State *l)
{
    char buffer[1024];
    server.Read(buffer,1024);
    //size_t msgLen = wcslen((wchar_t*)(buffer+4))>>1;
    //if(strncmp("msg.",buffer,4))//&& buffer[4] != 0x00)
        lua_pushstring(l,buffer+4);
    //else
    //    lua_pushboolean(l,false);
    return 1;
}

void initluastate(lua_State *lua)
{
	luaL_openlibs(lua);
	lua_newtable(lua);

    // must-have functions
	lua_registert(lua,"connectServer",lua_connectServer);
	lua_registert(lua,"readMessage",lua_readMessage);
	lua_registert(lua,"errorExit",lua_errorExit);
	lua_registert(lua,"startExe",lua_startexe);
	lua_registert(lua,"freeProcHandle",lua_freeProcHandle);
	lua_registert(lua,"injectDll",lua_injectDll);
	lua_registert(lua,"sleep",lua_sleep);
	lua_registert(lua,"messageBox",lua_messagebox);

    // from grimoire, may be useful
    lua_registert(lua,"browse",lua_browseforfolder);
    lua_registert(lua,"getArgs",lua_getargs);
    lua_registert(lua,"openLink",lua_openlink);
	lua_registert(lua,"getCwd",lua_getcwd);
	lua_registert(lua,"getRegPair",lua_regopenkey);
	lua_registert(lua,"deleteRegPair",lua_regdeletekey);
	lua_registert(lua,"setRegString",lua_regsetstring);
	lua_registert(lua,"setRegDword",lua_regsetdword);
	lua_registert(lua,"exists",lua_exists);
	lua_registert(lua,"browseForFolder",lua_browseforfolder);
	lua_registert(lua,"getModuleHandle",lua_getmodulehandle);
	lua_registert(lua,"regKeyExists",lua_regKeyExists);

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
        server.Disconnect();

    } catch(const char *err) {
		//dprintf("Error: %s\n",err);
		fprintf(stderr,"Error: %s\n",err);
		return 1;
	}
    return 0;
}

