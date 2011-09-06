// Interprocess Messanger client and server implementation
#include "ipsmes.h"

// we would use pipes
#include <windows.h>

EventMessage::EventMessage(const char* event) {
    strcpy(this->event, event);
}

EventMessage::~EventMessage()
{
    // TODO: free all params
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
