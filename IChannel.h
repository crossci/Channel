#pragma once

#include "../PublicLibrary/UnknownEx.h"
//#include "../PublicLibrary/MemoryPool.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
//#include <sys/socket.h>
#endif

#ifdef CHANNEL_EXPORTS
	#define CHANNEL_API __declspec(dllexport)
#else
	#define CHANNEL_API //__declspec(dllimport)
#endif

class IChannelEvent;
// {8C7E231A-A277-4E5D-9439-120CEEA69C0F}

// {9AF9BBA1-0B4B-4ACF-81F0-4A8BC09C2263}
static const GUID GUID_OF(IChannelItem) = { 0x9af9bba1, 0xb4b, 0x4acf, { 0x81, 0xf0, 0x4a, 0x8b, 0xc0, 0x9c, 0x22, 0x63 } };
class IChannelItem : public IUnknownEx
{
public:
	virtual const char* GetRemoteIP() = 0;
	virtual unsigned short GetRemotePort() = 0;
	//设置通道
	virtual void SetChannelEvent(IChannelEvent* pChannelEvent) = 0;
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite) = 0;
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen) = 0;
	//关闭
	virtual void Close(void) = 0;
};

// {167C91D4-BD8D-4457-B47D-0C785BAAE89A}
static const GUID GUID_OF(IChannelEvent) = { 0x167c91d4, 0xbd8d, 0x4457, { 0xb4, 0x7d, 0xc, 0x78, 0x5b, 0xaa, 0xe8, 0x9a } };
class IChannelEvent : public IUnknownEx
{
public:
	//连接
	virtual void OnConnect(IChannelItem* pChannelItem, const char* IP,unsigned short Port) = 0;
	//发送数据消息
	virtual void OnWrite(IChannelItem* pChannelItem, unsigned int size,void* Param) = 0;
	//接收数据
	virtual void OnRead(IChannelItem* pChannelItem, const char* Buffer,int Length,sockaddr_in* addr,int addrlen) = 0;
	//关闭
	virtual void OnClose(IChannelItem* pChannelItem,sockaddr_in* addr,int addrlen) = 0;
};

namespace _CHANNEL
{
	typedef enum _CHANNEL_MODE
	{
		UNKNOWN = 0,
		TCP_SERVER,
		TCP_CLIENT,
		UDP,
		PIPE_SERVER,
		PIPE_CLIENT,
	}CHANNEL_MODE;
};

// {83293CB0-7AF4-4A1E-8A7C-6E4CEB6C4E98}
static const GUID GUID_OF(IChannel) = { 0x83293cb0, 0x7af4, 0x4a1e, { 0x8a, 0x7c, 0x6e, 0x4c, 0xeb, 0x6c, 0x4e, 0x98 } };
class IChannel : public IUnknownEx
{
public:
	//初始化
	virtual bool Initailize(_CHANNEL::CHANNEL_MODE mode,IChannelEvent* pChannelEvent,const char* name=0,unsigned short port=0) = 0;
	//关闭
	virtual void Close(void) = 0;
    //
    virtual int CheckConnectState() = 0;
public:
	//
	virtual const char* GetRemoteIP() = 0;
	//
	virtual unsigned short GetRemotePort() = 0;
public:
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite) = 0;
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen) = 0;
};

class IChannelServer;
// {FE5D88E0-2BFC-4FDA-8C2B-F020741600D8}
static const GUID GUID_OF(IChannelEventManage) = { 0xfe5d88e0, 0x2bfc, 0x4fda, { 0x8c, 0x2b, 0xf0, 0x20, 0x74, 0x16, 0x0, 0xd8 } };
class IChannelEventManage : public IUnknownEx
{
public:
	virtual IChannelEvent* AllocClient(IChannelServer* pChannelServer,void* param) = 0;
};

CHANNEL_API bool InitChannelModule(int num = 1);
CHANNEL_API void CloseChannelModule(void);

CHANNEL_API IChannel* CreateChannel(void);


