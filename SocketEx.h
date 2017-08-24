#pragma once
#include "../PublicLibrary/Macro.h"

#ifdef WIN32
#include <winsock2.h>
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define INVALID_SOCKET		-1
#define SOCKET_ERROR		-1
#define SD_BOTH	2
#endif // WIN32


#define SOCKET_BUFF_SIZE	(1024*16) //Socket大小

class CSocketEx
{
protected:
	//Socket
	SOCKET m_Socket;
public:
	CSocketEx(void);
	virtual ~CSocketEx(void);
	//创建
	virtual bool Create(int nSocketType=SOCK_STREAM,SOCKET s=INVALID_SOCKET);
	//绑定
	virtual bool Bind(const char* IP,unsigned short Port);
	//断开
	virtual void CloseSocket(void);
    //
    virtual int CheckConnectState();
	//监听
	virtual bool Listen(unsigned short Port);
	//接受连接
	virtual SOCKET Accept(struct sockaddr* addr,socklen_t* addrlen);
	//连接
	virtual int Connect(const char* IP,unsigned short Port);
	//TCP
	virtual int Send(const char* buf, int len);
	virtual int Recv(char* buf,int len);
	//UDP
	virtual int SendTo(const char* buf,int len,const struct sockaddr* to,socklen_t tolen);
	virtual int RecvFrom(char* buf,int len,struct sockaddr* from,socklen_t* fromlen);
public:
	//
	virtual const char* GetRemoteIP();
	//
	virtual unsigned short GetRemotePort();
};
