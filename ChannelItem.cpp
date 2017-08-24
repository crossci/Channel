#include "./ChannelItem.h"
#include "../PublicLibrary/UnknownEx.h"
#include "../PublicLibrary/log.h"

CChannelItem::CChannelItem(void)
{
	m_pChannelEvent = NULL;
}

CChannelItem::~CChannelItem(void)
{
	IChannelEvent* pChannelEvent = 0;
	m_Lock.Lock();
	if (m_pChannelEvent)
	{
		IF_QUERYINTERFACE(m_pChannelEvent,pChannelEvent,IChannelEvent)
		{
		}
		SAFE_RELEASE(m_pChannelEvent);
	}
	m_Lock.Unlock();
	if (pChannelEvent)
	{
		pChannelEvent->Release();
	}
}

//查询接口
bool CChannelItem::QueryInterface( const GUID& guid, void **ppvObject)
{
	QUERYINTERFACE(CChannelItem);
	QUERYINTERFACE(IChannelItem);
	IF_TRUE(QUERYINTERFACE_PARENT(CUnknownEx));

	return false;
}

//设置通道
void CChannelItem::SetChannelEvent(IChannelEvent* pChannelEvent)
{
	IChannelEvent* p;
	m_Lock.Lock();
	p = m_pChannelEvent;
	m_pChannelEvent = 0;

	if (pChannelEvent)
	{
		IF_QUERYINTERFACE(pChannelEvent,m_pChannelEvent,IChannelEvent)
		{
		}
	}
	m_Lock.Unlock();

	SAFE_RELEASE(p);
}

void CChannelItem::OnConnect(IChannelItem* pChannelItem, const char* IP,unsigned short Port)
{
	IChannelEvent* pChannelEvent = 0;
	m_Lock.Lock();
	if (m_pChannelEvent)
	{
		IF_QUERYINTERFACE(m_pChannelEvent,pChannelEvent,IChannelEvent)
		{
		}
	}
	m_Lock.Unlock();

	if (pChannelEvent)
	{
		pChannelEvent->OnConnect(pChannelItem,IP,Port);
		pChannelEvent->Release();
	}
}

void CChannelItem::OnWrite( IChannelItem* pChannelItem, unsigned int size,void* Param )
{
	IChannelEvent* pChannelEvent = 0;
	m_Lock.Lock();
	if (m_pChannelEvent)
	{
		IF_QUERYINTERFACE(m_pChannelEvent,pChannelEvent,IChannelEvent)
		{
		}
	}
	m_Lock.Unlock();

	if (pChannelEvent)
	{
		pChannelEvent->OnWrite(pChannelItem,size,Param);
		pChannelEvent->Release();
	}
}

void CChannelItem::OnRead(IChannelItem* pChannelItem, const char* Buffer,int Length,sockaddr_in* addr,int addrlen)
{
	IChannelEvent* pChannelEvent = 0;
	m_Lock.Lock();
	if (m_pChannelEvent)
	{
		IF_QUERYINTERFACE(m_pChannelEvent,pChannelEvent,IChannelEvent)
		{
		}
	}
	m_Lock.Unlock();

	if (pChannelEvent)
	{
		pChannelEvent->OnRead(pChannelItem,Buffer,Length,addr,addrlen);
		pChannelEvent->Release();
	}
}

void CChannelItem::OnClose(IChannelItem* pChannelItem,sockaddr_in* addr,int addrlen)
{
	IChannelEvent* pChannelEvent = 0;
	m_Lock.Lock();
	if (m_pChannelEvent)
	{
		pChannelEvent = m_pChannelEvent;
		m_pChannelEvent = 0;
	}
	m_Lock.Unlock();

	if (pChannelEvent)
	{
		pChannelEvent->OnClose(pChannelItem,addr,addrlen);
		pChannelEvent->Release();
	}
}
