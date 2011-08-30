// this file is from grimoire source
// see humans.txt for more info

#include <windows.h>
#include <stdio.h>


#include "pipe.h"

const static int timeout = 50;

PipeServer::PipeServer(const char *pipename)
	: m_pipename(pipename),m_connected(false)
{
}
PipeServer::~PipeServer()
{
	CloseHandle(m_pipe);
}

bool PipeServer::Connect()
{
	m_pipe = CreateNamedPipe(m_pipename,PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
			1,1024,1024,0,NULL);

	if(!m_pipe) {
	       //dprintf("Could not create pipe %s\n",m_pipename);
	       throw("Pipeserver creation failed\n");
	}

	m_connected = ConnectNamedPipe(m_pipe,NULL);
	return m_connected;
}

void PipeServer::Disconnect()
{
	m_connected = false;
	DisconnectNamedPipe(m_pipe);
}

bool PipeServer::Write(const char *msg,DWORD msglen)
{
	if(!m_connected) return false;
	DWORD nwritten;
	if(!WriteFile(m_pipe,msg,msglen,&nwritten,NULL)) return false;
	return true;
}

bool PipeServer::Print(const char *msg)
{
	return Write(msg,strlen(msg)+1);
}

bool PipeServer::Read(char *buf, DWORD buflen)
{
	if(!m_connected) return false;
	DWORD nread;
	if(!ReadFile(m_pipe,buf,buflen,&nread,NULL)) return false;
	buf[nread] = 0;	//buffer overflow waiting to happen
	return true;
}


PipeClient::PipeClient(const char *pipename)
	: m_pipename(pipename),m_connected(false)
{
}

PipeClient::~PipeClient()
{
	CloseHandle(m_pipe);
}

bool PipeClient::Connect()
{
	m_connected = false;
	while(1) {
		m_pipe = CreateFile(m_pipename,
				GENERIC_READ|GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

		if(m_pipe != INVALID_HANDLE_VALUE) break;

		if(!WaitNamedPipe(m_pipename,timeout)) {
			printf("Could not open pipe\n");
			return false;
		}
	}

	DWORD mode = PIPE_READMODE_MESSAGE;
	bool s = SetNamedPipeHandleState(m_pipe,&mode,NULL,NULL);
	if(!s) {
		printf("SetNamedPipeHandleState failed\n");
		return false;
	}
	m_connected = true;
	return m_connected;
}

void PipeClient::Disconnect()
{
	m_connected = false;
	CloseHandle(m_pipe);
	m_pipe = NULL;
}

bool PipeClient::Write(const char *msg,DWORD msglen)
{
	if(!m_connected) return false;
	DWORD nwritten;
	bool s = WriteFile(m_pipe,msg,msglen,&nwritten,NULL);
	return s;
}

bool PipeClient::Print(const char *msg)
{
	return Write(msg,strlen(msg)+1);
}

bool PipeClient::Read(char *buf, DWORD buflen)
{
	if(!m_connected) return false;
	DWORD nread;
	ReadFile(m_pipe,buf,buflen,&nread,NULL);
	return true;
}

/*
 * Wrappers for python ctypes
 */

extern "C" {
PipeClient * PipeClient_Create(const char *pipename)
{
	return new PipeClient(pipename);
}

void PipeClient_Destroy(PipeClient *self)
{
	delete self;
}

bool PipeClient_Connect(PipeClient *self)
{
	return self->Connect();
}

void PipeClient_Disconnect(PipeClient *self)
{
	self->Disconnect();
}

bool PipeClient_Write(PipeClient *self,const char *msg, DWORD msglen)
{
	return self->Write(msg,msglen);
}

bool PipeClient_Print(PipeClient *self,const char *msg)
{
	return self->Print(msg);
}

bool PipeClient_Read(PipeClient *self, char *buf, DWORD buflen)
{
	return self->Read(buf,buflen);
}
}
