// Interprocess Messanger client and server implementation
#include "ipsmes.h"

#include <assert.h>

// we would use pipes
#include <windows.h>

EventMessage::EventMessage(const char* event)
{
    strcpy(this->event, event);
}

EventMessage::~EventMessage()
{
    // cleanup parameters, they are all private (copied)
    while (!parameters.empty())
    {
        // ParamType vector empty too early
        assert(!parameterTypes.empty());

        void* param = parameters.back();
        ParamType type = parameterTypes.back();

        parameters.pop_back();
        parameterTypes.pop_back();

        switch(type)
        {
            case T_INT:
                delete  (int*)param;
                break;
            case T_FLOAT:
                delete  (float*)param;
                break;
            case T_CSTR:
            case T_CSTRL:
                delete[] (char*)param;
                break;
            default:
                assert(!"unknown parameter type!");
        }
    }
    // ParamType vector not empty after freeing
    assert(parameterTypes.empty());
}

void EventMessage::pushParam( const int param )
{
    int* copiedParam = new int(param);
    this->parameters.push_back((void*)copiedParam);
    this->parameterTypes.push_back(T_INT);
}

void EventMessage::pushParam( const char* param )
{
    const int length = strlen(param);
    char* copiedParam = new char[length];
    strcpy(copiedParam, param);
    this->parameters.push_back((void*)copiedParam);
    this->parameterTypes.push_back(T_CSTR);
}

void EventMessage::pushParam( const char* param, const int length )
{
    char* copiedParam = new char[length];
    memcpy(copiedParam, param, length);
    this->parameters.push_back((void*)copiedParam);
    this->parameterTypes.push_back(T_CSTRL);
}

void EventMessage::pushParam( const float param )
{
    float* copiedParam = new float(param);
    this->parameters.push_back((void*)copiedParam);
    this->parameterTypes.push_back(T_FLOAT);
}

int   EventMessage::getParamInt(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_INT)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return *((int*)(this->parameters[which]));
}
const char*  EventMessage::getParamCstr(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_CSTR)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return (char*)this->parameters[which];
}
const char*  EventMessage::getParamCstrL(const unsigned int which, int *length)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_CSTRL)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");


    return (char*)(this->parameters[which]);
}
float EventMessage::getParamFloat(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_INT)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return *(float*)(this->parameters[which]);
}
void*  EventMessage::getParamAny(const unsigned int which, ParamType &type, int *length = NULL)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");

    type = this->parameterTypes[which];
    return this->parameters[which];
}
