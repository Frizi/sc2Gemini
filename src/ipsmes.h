// Interprocess Messanger client and server api
#ifndef __MAIN_H__
#define __MAIN_H__

#include <string>
#include <vector>
#include <queue>

typedef enum {
    T_INT,
    T_STR,
    T_FLOAT,
    T_END // for header end
} ParamType;

typedef union {
    int integer;
    float floating;
    std::string* string;
} ParamValue;

class EventMessage {
public:
    EventMessage( const char* eventType );
    ~EventMessage();
    const std::string getEventType();

    void pushParam( const int param );
    void pushParam( const std::string param );
    void pushParam( const std::string, const int length );
    void pushParam( const float param );

    int         paramCount();
    ParamType   getParameterType(unsigned int which);
	int         getParamInt(unsigned int which);
    std::string getParamString(unsigned int which);
    float       getParamFloat(unsigned int which);

private:
    char* event;
    std::vector<ParamValue> parameters;
    std::vector<ParamType> parameterTypes;
};

class MessangerClient {
public:
	MessangerClient( std::string name, std::string servername );
	~MessangerClient();
	void pushEvent( std::string event );
	void registerEvent( std::string event );
	void syncWaitForEvent( std::string event );

	bool peekEvent( std::string* event );

private:
    std::queue<EventMessage> events;
};

class MessangerServer {
public:
	MessangerServer( std::string name );
	~MessangerServer();
	//void acceptClient( std::string name );
	void pushEvent ( std::string event, std::string destName );
	void broadcastEvent( std::string event );

	bool peekEvent( std::string* event );
private:
    std::queue<EventMessage> events;
};

#endif // __MAIN_H__
