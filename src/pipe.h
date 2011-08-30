// this file is from grimoire source
// see humans.txt for more info

#inndef __PIPE_H__
#define __PIPE_H_

class PipeServer
{
public:
	PipeServer(const char *pipename);
	~PipeServer();

	bool Connect();
	void Disconnect();

	bool Write(const char *msg,DWORD msglen);
	bool Print(const char *msg);
	bool Read(char *buf, DWORD buflen);
protected:
	HANDLE m_pipe;
	const char *m_pipename;
	bool m_connected;
};

class PipeClient
{
public:
	PipeClient(const char *pipename);
	~PipeClient();

	bool Connect();
	void Disconnect();

	bool Write(const char *msg,DWORD msglen);
	bool Print(const char *msg);
	bool Read(char *buf, DWORD buflen);
protected:
	HANDLE m_pipe;
	const char *m_pipename;
	bool m_connected;
};

/*
 * Wrap PipeClient for python ctypes
 */
/*
extern "C" {
PipeClient * PipeClient_Create(const char *pipename);
void PipeClient_Destroy(PipeClient *self);
bool PipeClient_Connect(PipeClient *self);
void PipeClient_Disconnect(PipeClient *self);
bool PipeClient_Write(PipeClient *self,const char *msg, DWORD msglen);
bool PipeClient_Print(PipeClient *self,const char *msg);
bool PipeClient_Read(PipeClient *self,char *buf, DWORD buflen);
}
*/
#endif
