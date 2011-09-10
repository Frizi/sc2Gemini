// Interprocess Messenger client and server api
#ifndef __IPSMES_H__
#define __IPSMES_H__

#include <string>
#include <vector>
#include <streambuf>
#include <queue>

typedef enum {
    T_INT,
    T_FLOAT,
    T_STR,
    T_WSTR,
    T_END // for header end
} ParamType;

typedef union {
    int integer;
    float floating;
    std::string* string;
    std::wstring* wstring;
} ParamValue;

/* EventMessage didn't have any extra features
 * like endianess correction or similar.
 * It will be used on the same machine.
 */

class EventMessage {
public:
    EventMessage();
    EventMessage( std::string eventType );
    EventMessage( std::streambuf &sb ); // shorthand to serialize
    ~EventMessage();
    void clear();
    void serialize(std::streambuf &sb);
    void deserialize(std::streambuf &sb);

    std::string getEventType();

    void pushParam( const int param );
    void pushParam( const std::string param );
    void pushParam( const std::wstring param );
    void pushParam( const float param );

    int          paramCount();
    ParamType    getParameterType(unsigned int which);
    int          getParamInt(unsigned int which);
    std::string  getParamString(unsigned int which);
    std::wstring getParamWstring(unsigned int which);
    float        getParamFloat(unsigned int which);

private:
    std::string event;
    std::vector<ParamValue> parameters;
    std::vector<ParamType> parameterTypes;
};

class MessengerClient {
public:
    MessengerClient( std::string name, std::string servername );
    ~MessengerClient();
    void pushEvent( std::string event );
    void registerEvent( std::string event );
    void syncWaitForEvent( std::string event );

    bool peekEvent( std::string* event );

private:
    std::queue<EventMessage> events;
};

class MessengerServer {
public:
    MessengerServer( std::string name );
    ~MessengerServer();

    void pushEvent ( std::string event, std::string destName );
    void broadcastEvent( std::string event );

    bool peekEvent( std::string* event );
private:
    std::queue<EventMessage> events;
};

#endif // __IPSMES_H__
