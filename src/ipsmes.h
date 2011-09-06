// Interprocess Messanger client and server api
#ifndef __MAIN_H__
#define __MAIN_H__

#inlude <string>
#inlude <vector.h>
#inlude <queue.h>

typedef enum {
    T_INT,
    T_CSTR,
    T_CSTRL,
    T_FLOAT
} paramType

class EventMessage{
public:
    EventMessage( const char* eventType );
    void pushParam( const int param );
    void pushParam( const char* param );
    void pushParam( const char*, const int length );
    void pushParam( const float param );

    int    paramCount();
	int    getParamInt(int which);
    char*  getParamCstr(int which);
    char*  getParamCstrL(int which, int &length);
    float  getParamFloat(int which);
    void*  getParamAny(int which, paramType &type, int &length);

private:
    char* event;
    std::vector<void*> parameters;
    std::vector<paramType> parameterTypes;
}

class messangerClient {
public:
	messangerClient( std::string name, std::string servername );
	~messangerClient();
	void pushEvent( std::string event );
	void registerEvent( std::string event );
	void syncWaitForEvent( std::string event );

	bool peekEvent( std::string* event );

private:
    std::queue<EventMessage> events;
};

class messangerServer {
public:
	messengerServer( std::string name );
	~messengerServer();
	acceptClient( std::string name );
	void pushEvent ( string event, string destName );
	void broadcastEvent( string event);

	bool peekEvent( std::string* event );
private:
    std::queue<EventMessage> events;
};

#endif // __MAIN_H__
