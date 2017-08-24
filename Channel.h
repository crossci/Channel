#pragma once
#include "../PublicLibrary/UnknownEx.h"
#include "IChannel.h"
#include "ChannelItem.h"
#include "../PublicLibrary/Macro.h"

class CIocpItem;
// {2CB08651-FD82-46A1-9E0C-70132AAA6629}
static const GUID GUID_OF(CChannel) = { 0x2cb08651, 0xfd82, 0x46a1, { 0x9e, 0xc, 0x70, 0x13, 0x2a, 0xaa, 0x66, 0x29 } };
class CChannel : public CChannelItem, public IChannel, public IChannelEvent
{
private:
	CLock m_Lock;
	CChannelItem* m_pChannelItem;
public:
	CChannel(void);
	virtual ~CChannel(void);

	UNKNOWNEX_INTERFACE(CUnknownEx);
public:
	//初始化
	virtual bool Initailize(_CHANNEL::CHANNEL_MODE mode,IChannelEvent* pChannelEvent,const char* name,unsigned short port);
	//关闭
	virtual void Close(void);
    //
    virtual int CheckConnectState();
	//
	virtual const char* GetRemoteIP();
	//
	virtual unsigned short GetRemotePort();
public:
	// 写缓存
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite);
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen);
public:
	//连接
	virtual void OnConnect(IChannelItem* pChannelItem, const char* IP,unsigned short Port);
	//发送数据消息
	virtual void OnWrite(IChannelItem* pChannelItem, unsigned int size,void* Param);
	//接收数据
	virtual void OnRead(IChannelItem* pChannelItem, const char* Buffer,int Length,sockaddr_in* addr,int addrlen);
	//关闭
	virtual void OnClose(IChannelItem* pChannelItem,sockaddr_in* addr,int addrlen);
public:
	virtual CIocpItem* GetIocpItem();

	NEW_CREATE(CChannel);
};