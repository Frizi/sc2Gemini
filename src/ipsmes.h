// Interprocess Messanger client and server api
#ifndef __MAIN_H__
#define __MAIN_H__

#inlude <string>

class messangerClient {
public:
	messangerClient( std::string name, std::string servername );
	
	void pushEvent( std::string event );
	void registerEvent( std::string event );
	void syncWaitForEvent( std::string event );
	
	bool peekEvent( std::string* event );
	void pushParam( int param );
	void pushParam( std::string param );
	int popParamInt();
	std::string popParamString();
};

class messangerServer {
public:
	messengerServer( std::string name );
	
	void pushEvent ( string event, string destName );
	void broadcastEvent( string event);
	
	bool peekEvent( std::string* event );
	void pushParam( int param );
	void pushParam( std::string param );
	int popParamInt();
	std::string popParamString();
};

#endif // __MAIN_H__