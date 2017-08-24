#pragma once
#include "../PublicLibrary/UnknownEx.h"
#include "../PublicLibrary/VerifyID.h"
#include "../PublicLibrary/Lock.h"
#include "overlappedplus.h"
#include "../PublicLibrary/Macro.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif // WIN32


#define IOCP_ITEM_BUFF_SIZE	(1024*4) //Socket大小

namespace _IOCP_ITEM
{
	enum 
	{
		UNKNOWN = 0,
		CLOSE,
		CONNECT,
		WRITE,
		RECEIVE,
	};
}

// {45CE4566-1E11-4BC5-B28E-4921AC59DACC}
static const GUID GUID_OF(IIocpItem) = { 0x45ce4566, 0x1e11, 0x4bc5, { 0xb2, 0x8e, 0x49, 0x21, 0xac, 0x59, 0xda, 0xcc } };
class IIocpItem : public IUnknownEx
{
public:
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite) = 0;
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen) = 0;

	//关闭
	virtual void Close(void) = 0;
};

class CIocp;
// {B68F17E7-64C2-483C-8B94-53AEB1820148}
static const GUID GUID_OF(CIocpItem) = { 0xb68f17e7, 0x64c2, 0x483c, { 0x8b, 0x94, 0x53, 0xae, 0xb1, 0x82, 0x1, 0x48 } };
class CIocpItem : public CUnknownEx, public IIocpItem, public CRandomKey
{
protected:
	//接收数据
	_OVERLAPPEDPLUS m_ReadOverlappedPlus;
	//缓存
	char m_Buffer[IOCP_ITEM_BUFF_SIZE];
	//
	CLock m_Lock;
public:
	CIocpItem(void);
	virtual ~CIocpItem(void);

	UNKNOWNEX_INTERFACE(CUnknownEx);
public:
	//连接事件
	virtual void ConnectEvent(void);
	//接收字节事件
	virtual void WriteEvent(unsigned int size,void* Param);
	//接收字节事件
	virtual void ReceiveEvent(unsigned int size);
	//关闭事件
	virtual void CloseEvent(void);

	//连接消息
	virtual void OnConnect(const char* IP,unsigned short Port){};
	//发送数据消息
	virtual void OnWrite(unsigned int size,void* Param){};
	//接收到数据消息
	virtual void OnRead(void* lpBuffer, unsigned int nNumberOfBytesToRead){};
	//关闭消息
	virtual void OnClose(void){};
public:
	//关闭
	virtual void Close(void);
	//接收
	virtual bool Receive(void);
public:
	//获得句柄
	virtual HANDLE GetHandle(void) = 0;
    //
    virtual int CheckConnectState() = 0;
	//
	virtual const char* GetRemoteIP() = 0;
	virtual unsigned short GetRemotePort() = 0;
public:
	//初始化
	virtual bool Initailize(const char* Name,unsigned short Port) = 0;
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite)=0;
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen) = 0;
	// 读缓存
	virtual int Read(void* lpBuffer, unsigned int nNumberOfBytesToRead)=0;
};
