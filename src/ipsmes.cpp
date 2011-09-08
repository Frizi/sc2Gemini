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

        ParamValue param = parameters.back();
        ParamType type = parameterTypes.back();

        parameters.pop_back();
        parameterTypes.pop_back();

        if(type == T_STR)
            delete param.string;
    }
    // ParamType vector not empty after freeing
    assert(parameterTypes.empty());
}

int EventMessage::paramCount()
{
    assert(this->parameterTypes.size() == this->parameters.size());
    return this->parameters.size();
}

void EventMessage::pushParam( const int param )
{
    ParamValue val;
    val.integer = param;
    this->parameters.push_back(val);
    this->parameterTypes.push_back(T_INT);
}

void EventMessage::pushParam( const std::string param )
{
    std::string* paramPtr = new std::string(param);
    ParamValue val;
    val.string = paramPtr;
    this->parameters.push_back(val);
    this->parameterTypes.push_back(T_STR);
}

void EventMessage::pushParam( const float param )
{
    ParamValue val;
    val.floating = param;
    this->parameters.push_back(val);
    this->parameterTypes.push_back(T_FLOAT);
}

ParamType EventMessage::getParameterType(const unsigned int which)
{
    if(parameterTypes.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter's type");
    return parameterTypes[which];
}

int   EventMessage::getParamInt(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_INT)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return this->parameters[which].integer;
}

std::string  EventMessage::getParamString(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_STR)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return *this->parameters[which].string;
}

float EventMessage::getParamFloat(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_INT)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return this->parameters[which].floating;
}
