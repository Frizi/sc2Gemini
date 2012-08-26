#include "main.h"
#include <csipc/Client.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <queue>


#define HANDLE_NOT_CONNECTED \
    do\
    {\
        if(globalData.m_connected == false)\
        {\
            globalData.lastError = e_not_connected;\
            return false;\
        }\
    } while(0)

struct GlobalData_t {
    CsIpc::Client *clientRef;
    bool m_connected;
    geminiClientError lastError;
};

GlobalData_t globalData;

bool DLL_EXPORT GEM_ConnectAs(std::string clientname)
{
    if(globalData.m_connected == true)
    {
        globalData.lastError = e_already_connected;
        return false;
    }

    try
    {
        globalData.clientRef = new CsIpc::Client(clientname, "gemini");
    }
    catch(...) {
        globalData.lastError = e_not_connected;
        return false;
    }

    globalData.m_connected = false;
    globalData.lastError = e_no_error;
    return true;
}

bool DLL_EXPORT GEM_Send(CsIpc::EventMessage &msg)
{
    HANDLE_NOT_CONNECTED;

    globalData.clientRef->Send(msg);

    globalData.lastError = e_no_error;
    return true;
}

bool DLL_EXPORT GEM_Peek(CsIpc::EventMessage &msg)
{
    HANDLE_NOT_CONNECTED;

    if(!globalData.clientRef->Peek(msg))
    {
        globalData.lastError = e_no_pending_events;
        return false;
    }

    globalData.lastError = e_no_error;
    return true;
}

bool DLL_EXPORT GEM_WaitForEvent(std::string eventType, CsIpc::EventMessage &msg, unsigned int timeout)
{
    HANDLE_NOT_CONNECTED;

    globalData.clientRef->WaitForEvent(msg, eventType, timeout);

    globalData.lastError = e_no_error;
    return true;
}

bool DLL_EXPORT GEM_RegisterEvent(std::string eventType)
{
    HANDLE_NOT_CONNECTED;

    globalData.lastError = e_no_error;
    return true;
}

bool DLL_EXPORT GEM_UnregisterEvent(std::string eventType)
{
    HANDLE_NOT_CONNECTED;

    globalData.lastError = e_no_error;
    return true;
}

bool DLL_EXPORT GEM_Disconnect()
{
    HANDLE_NOT_CONNECTED;

    delete globalData.clientRef;
    globalData.clientRef = NULL;
    globalData.m_connected = false;

    globalData.lastError = e_no_error;
    return true;
}

geminiClientError DLL_EXPORT GEM_GetLastError()
{
    return globalData.lastError;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            // attach to process

            // init global var
            globalData.clientRef = NULL;
            globalData.lastError = e_no_error;
            globalData.m_connected = false;

            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
