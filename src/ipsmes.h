// Interprocess Messanger client and server api
#ifndef __MAIN_H__
#define __MAIN_H__

#include <string>
#include <vector>
#include <queue>

typedef enum {
    T_INT,
    T_CSTR,
    T_CSTRL,
    T_FLOAT
} ParamType;

class EventMessage {
public:
    EventMessage( const char* eventType );
    ~EventMessage();
    const char* getEventType();

    void pushParam( const int param );
    void pushParam( const char* param );
    void pushParam( const char*, const int length );
    void pushParam( const float param );

    int    paramCount();
	int    getParamInt(int which);
    char*  getParamCstr(int which);
    char*  getParamCstrL(int which, int &length);
    float  getParamFloat(int which);
    void*  getParamAny(int which, ParamType &type, int &length);

private:
    char* event;
    std::vector<void*> parameters;
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
