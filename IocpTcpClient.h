#pragma once
#include "IocpSocket.h"
#include "ChannelItem.h"
#include "../PublicLibrary/MemoryPool.h"
#include "../PublicLibrary/Macro.h"

// {30280A1B-E52C-41DB-BF71-3C3A496C9C64}
static const GUID GUID_OF(CIocpTcpClient) = { 0x30280a1b, 0xe52c, 0x41db, { 0xbf, 0x71, 0x3c, 0x3a, 0x49, 0x6c, 0x9c, 0x64 } };
class CIocpTcpClient : public CCircularMemory, public CIocpSocket, public CChannelItem
{
public:
	CIocpTcpClient(void);
	virtual ~CIocpTcpClient(void);

	MEMORY_INTERFACE;
public:
	//初始化
	virtual bool Initailize(const char* Name,unsigned short Port);
public:
	//
	virtual const char* GetRemoteIP();
	//
	virtual unsigned short GetRemotePort();
    //
    virtual int CheckConnectState();
public:
	//连接消息
	virtual void OnConnect(const char* IP,unsigned short Port);
	//关闭
	virtual void OnClose(void);
	//接收到数据消息
	virtual void OnRead(void* lpBuffer, unsigned int nNumberOfBytesToRead);
public:
	//关闭
	virtual void Close(void);
public:
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite);
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen);
public:
	POOL_CREATE(CIocpTcpClient);
};
