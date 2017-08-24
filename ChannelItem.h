#pragma once
#include "IocpSocket.h"
#include "IChannel.h"
#include "../PublicLibrary/UnknownEx.h"

class IChannelEvent;
// {ED4C8874-E649-4199-807F-7B4B90553BDF}
static const GUID GUID_OF(CChannelItem) = { 0xed4c8874, 0xe649, 0x4199, { 0x80, 0x7f, 0x7b, 0x4b, 0x90, 0x55, 0x3b, 0xdf } };
class CChannelItem : public CUnknownEx, public IChannelItem
{
protected:
	//通道
	CLock m_Lock;
	IChannelEvent* m_pChannelEvent;
public:
	CChannelItem(void);
	virtual ~CChannelItem(void);

	UNKNOWNEX_INTERFACE(CUnknownEx);
public:
	//设置通道
	virtual void SetChannelEvent(IChannelEvent* pChannelEvent);
public:
	//连接消息
	virtual void OnConnect(IChannelItem* pChannelItem, const char* IP,unsigned short Port);
	//发送数据消息
	virtual void OnWrite(IChannelItem* pChannelItem, unsigned int size,void* Param);
	//接收到数据消息
	virtual void OnRead(IChannelItem* pChannelItem, const char* Buffer,int Length,sockaddr_in* addr,int addrlen);
	//关闭
	virtual void OnClose(IChannelItem* pChannelItem,sockaddr_in* addr,int addrlen);
};
