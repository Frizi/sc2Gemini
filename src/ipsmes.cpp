// Interprocess Messanger client and server implementation
#include "ipsmes.h"

#include <iostream>
#include <assert.h>

// we would use pipes
#include <windows.h>

EventMessage::EventMessage(std::string event)
{
    this->event = event;
}

EventMessage::~EventMessage()
{
    this->clear();
}

void EventMessage::clear()
{
    // cleanup parameters, they are all private (copied)
    while( !parameters.empty() )
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

void EventMessage::serialize(std::streambuf &sb)
{
    assert(parameters.size() == parameterTypes.size());
    std::ostream out(&sb);

    for(unsigned int i; i<parameters.size();i++)
    {
        ParamType type = parameterTypes[i];
        out << (int)type;
    }
    out << T_END;
    for(unsigned int i=0; i<parameters.size();i++)
    {
        ParamValue param = parameters[i];
        ParamType type = parameterTypes[i];
        switch(type)
        {
            case T_STR:
                out << param.string->size();
                out << param.string->data();
                break;
            case T_INT:
                out << param.integer;
                break;
            case T_FLOAT:
                out << param.floating;
                break;
            default:
                assert(false);
        }
    }
}

void EventMessage::deserialize(std::streambuf &sb)
{
    this->clear();
    std::istream in(&sb);

    int type;

    in >> type;
    while(type != T_END)
    {
        parameterTypes.push_back(static_cast<ParamType>(type));
        in >> type;
    }

    for(unsigned int i=0; i<parameterTypes.size();i++)
    {
        ParamValue param;
        ParamType type = parameterTypes[i];
        switch(type)
        {
            case T_STR:
                {
                    unsigned int size;
                    in >> size;
                    char data[size];
                    in.read(data,size);
                    std::string* str = new std::string();
                    str->assign(data,size);
                    param.string = str;
                }
                break;
            case T_INT:
                in >> param.integer;
                break;
            case T_FLOAT:
                in >> param.floating;
                break;
            default:
                throw("EventMessage: Wrong parameter type in deserialization");
        }
        parameters.push_back(param);
    }

    assert(parameters.size() == parameterTypes.size());
}

std::string EventMessage::getEventType()
{
    return event;
}

int EventMessage::paramCount()
{
    assert(parameterTypes.size() == parameters.size());
    return parameters.size();
}

void EventMessage::pushParam( const int param )
{
    ParamValue val;
    val.integer = param;
    parameters.push_back(val);
    parameterTypes.push_back(T_INT);
}

void EventMessage::pushParam( const std::string param )
{
    std::string* paramPtr = new std::string(param);
    ParamValue val;
    val.string = paramPtr;
    parameters.push_back(val);
    parameterTypes.push_back(T_STR);
}

void EventMessage::pushParam( const float param )
{
    ParamValue val;
    val.floating = param;
    parameters.push_back(val);
    parameterTypes.push_back(T_FLOAT);
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

    return parameters[which].integer;
}

std::string  EventMessage::getParamString(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_STR)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return *(parameters[which].string);
}

float EventMessage::getParamFloat(const unsigned int which)
{
    if(parameters.size() <= which)
        throw("EventMessage: Attempt to access nonexistent parameter");
    if(parameterTypes[which] != T_INT)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return parameters[which].floating;
}
