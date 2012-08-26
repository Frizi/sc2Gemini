#ifndef __GEMINI_CLIENT_H__
#define __GEMINI_CLIENT_H__

#include <windows.h>
#include <csipc/EventMessage.h>
#include <string>

#ifndef __cplusplus
    #error this is c++ library
#endif

#ifdef BUILD_GEMINI_CLIENT_DLL
    #define DLL_EXPORT __declspec(dllexport)
#else
    #define DLL_EXPORT __declspec(dllimport)
#endif

typedef enum {
    e_no_error = 0,
    e_not_connected,
    e_already_connected,
    e_no_pending_events,
    e_timeout_reached,
} geminiClientError;


extern "C" bool DLL_EXPORT GEM_ConnectAs(std::string clientname);
extern "C" bool DLL_EXPORT GEM_Send(CsIpc::EventMessage &msg);
extern "C" bool DLL_EXPORT GEM_Peek(CsIpc::EventMessage &msg);
extern "C" bool DLL_EXPORT GEM_WaitForEvent(std::string eventType, CsIpc::EventMessage &msg, unsigned int timeout = 0);
extern "C" bool DLL_EXPORT GEM_RegisterEvent(std::string eventType);
extern "C" bool DLL_EXPORT GEM_UnregisterEvent(std::string eventType);
extern "C" bool DLL_EXPORT GEM_Disconnect();
extern "C" geminiClientError DLL_EXPORT GEM_GetLastError();

#endif // __GEMINI_CLIENT_H__
