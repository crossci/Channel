#pragma once
#include "SocketEx.h"
#include "IocpItem.h"
#include "../PublicLibrary/Macro.h"

#ifdef WIN32
#include "mswsock.h"
#pragma comment(lib,"mswsock.lib")
#endif

// {4A4A5EA0-58F2-4BE2-A72E-5BF516C24428}
static const GUID GUID_OF(CIocpSocket) = { 0x4a4a5ea0, 0x58f2, 0x4be2, { 0xa7, 0x2e, 0x5b, 0xf5, 0x16, 0xc2, 0x44, 0x28 } };
class CIocpSocket : public CIocpItem, public CSocketEx
{
public:
	CIocpSocket(void);
	virtual ~CIocpSocket(void);

	__QueryInterface;
public:
	//获得句柄
	virtual HANDLE GetHandle(void);
	//创建
	virtual bool Create(int nSocketType=SOCK_STREAM,SOCKET s=INVALID_SOCKET);
	//断开
	virtual void CloseSocket(void);
	//关闭消息
	virtual void OnClose();
    //
    virtual int CheckConnectState();

	//连接
	virtual int Connect(const char* IP,unsigned short Port);
	//关闭
	virtual void Close(void);
	//TCP
	virtual int Send(const char* buf, int len);
	virtual int Recv(char* buf,int len);
	//UDP
	virtual int SendTo(const char* buf,int len,const struct sockaddr* to,int tolen);
	virtual int RecvFrom(char* buf,int len,struct sockaddr* from,int* fromlen);
public:
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite);
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* to,int len);
	
	// 读缓存
	virtual int Read(void* lpBuffer, unsigned int nNumberOfBytesToRead);
public:
	//
	virtual const char* GetRemoteIP();
	//
	virtual unsigned short GetRemotePort();
};
