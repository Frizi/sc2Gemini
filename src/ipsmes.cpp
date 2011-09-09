// Interprocess Messanger client and server implementation
#include "ipsmes.h"

#include <iostream>
#include <assert.h>

// we would use pipes
#include <windows.h>

EventMessage::EventMessage()
{
    this->event = "";
}
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
    // ParamType vector may not be empty after freeing, because of exceptions
    parameterTypes.clear();
}

void EventMessage::serialize(std::streambuf &sb)
{
    assert(parameters.size() == parameterTypes.size());
    std::ostream out(&sb);

    size_t size = event.size();
    out.write((char*)&size, sizeof(size_t));
    out.write(event.data(), size);

    for(unsigned int i=0; i<parameters.size();i++)
    {
        ParamType type = parameterTypes[i];
        out.put(static_cast<char>(type));
    }
    out.put(static_cast<char>(T_END));
    for(unsigned int i=0; i<parameters.size();i++)
    {
        ParamValue param = parameters[i];
        ParamType type = parameterTypes[i];
        switch(type)
        {
            case T_STR:
                size = param.string->size();
                out.write((char*)&size, sizeof(size_t));
                out.write(param.string->data(), size);
                break;
            case T_INT:
                out.write(reinterpret_cast<char*>(&(param.integer)), sizeof(int));
                break;
            case T_FLOAT:
                out.write(reinterpret_cast<char*>(&(param.floating)), sizeof(float));
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


    size_t size = 0;
    in.read((char*)&size, sizeof(size_t));

    char* data = new char[size];
    in.read(data,size);
    event.assign(data,size);
    delete[] data;
    data = 0;

    char type = 0;
    while(in.get(type) && type != T_END)
    {
        parameterTypes.push_back(static_cast<ParamType>(type));
    }

    for(unsigned int i=0; i<parameterTypes.size();i++)
    {
        ParamValue param;
        ParamType type = parameterTypes[i];
        switch(type)
        {
            case T_STR:
                {
                    in.read((char*)&size, sizeof(size_t));
                    char* data = new char[size];
                    in.read(data,size);
                    std::string* str = new std::string();
                    str->assign(data,size);
                    param.string = str;
                    delete[] data;
                }
                break;
            case T_INT:
                in.read(reinterpret_cast<char*>(&(param.integer)), sizeof(int));
                break;
            case T_FLOAT:
                in.read(reinterpret_cast<char*>(&(param.floating)), sizeof(float));
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
    if(parameterTypes[which] != T_FLOAT)
        throw("EventMessage: Attempt to retrieve parameter with wrong type");

    return parameters[which].floating;
}
