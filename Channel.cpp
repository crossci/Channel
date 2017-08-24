// Channel.cpp : 定义 DLL 应用程序的导出函数。
//

#include "IChannel.h"
#include "Channel.h"
#include "../PublicLibrary/log.h"
#include "IocpItem.h"
#include "IocpTcpServer.h"
#include "Iocp.h"
#include "IocpTcpClient.h"

CChannel::CChannel(void)
:m_pChannelItem(NULL)
{
}

CChannel::~CChannel(void)
{
	Close();
}

bool CChannel::QueryInterface( const GUID& guid, void **ppvObject)
{
	QUERYINTERFACE(CChannel);
	QUERYINTERFACE(IChannel);
	QUERYINTERFACE(IChannelEvent);
	IF_TRUE(QUERYINTERFACE_PARENT(CChannelItem));

	return false;
}

bool CChannel::Initailize( _CHANNEL::CHANNEL_MODE mode,IChannelEvent* pChannelEvent,const char* name,unsigned short port )
{
	bool b = false;

	m_Lock.Lock();
	if (m_pChannelItem)
	{
		m_pChannelItem->SetChannelEvent(0);
		m_pChannelItem->Close();
		m_pChannelItem->Release();
		m_pChannelItem = 0;
	}
	m_Lock.Unlock();

	SetChannelEvent(pChannelEvent);

	CIocpItem* pIocpItem = NULL;
	switch(mode)
	{
	case _CHANNEL::TCP_SERVER:
		{
			CIocpTcpServer* pIocpTcpServer = CIocpTcpServer::CreateInstance();
			if (pIocpTcpServer)
			{
				pIocpTcpServer->SetChannelEvent(this);
				pIocpItem = (CIocpItem*)pIocpTcpServer;
			}
			else
			{
				GetLogger()->log(_LOG_WARN,"CChannel::Initailize::AllocMemory(CIocpTcpServer::CreateInstance):FAILED\n");
			}
		}
		break;
	case _CHANNEL::TCP_CLIENT:
		{
			CIocpTcpClient* pIocpTcpClient = CIocpTcpClient::CreateInstance();
			if (pIocpTcpClient)
			{
				pIocpTcpClient->SetChannelEvent(this);
				pIocpItem = (CIocpItem*)pIocpTcpClient;
			}
			else
			{
				GetLogger()->log(_LOG_WARN,"CChannel::Initailize::AllocMemory(CreateIocpTcpClient::CreateInstance):FAILED\n");
			}
		}
		break;
	}
	if (pIocpItem)
	{
		IF_QUERYINTERFACE(pIocpItem,m_pChannelItem,CChannelItem)
		{
			if (pIocpItem->Initailize(name,port))
			{
				b = true;
			}
			else
			{
				Close();
			}
		}

		pIocpItem->Release();
	}
	else
	{
		GetLogger()->log(_LOG_WARN,"CChannel::Initailize::pIocpItem=NULL\n");
	}
	return b;
}

void CChannel::Close( void )
{
	CChannelItem* pIocpItem = 0;
	m_Lock.Lock();
	if (m_pChannelItem)
	{
		pIocpItem = m_pChannelItem;
		m_pChannelItem = 0;
	}
	m_Lock.Unlock();

	OnClose(pIocpItem, 0, 0);
	if (pIocpItem)
	{
		pIocpItem->Close();
		pIocpItem->Release();
	}
}

int CChannel::CheckConnectState()
{
    int ret = -1;
    CIocpItem* pIocpItem = GetIocpItem();
    if (pIocpItem)
    {
        ret = pIocpItem->CheckConnectState();
        pIocpItem->Release();
    }
    
    return ret;
}

int CChannel::Write( const void* lpBuffer, unsigned int nNumberOfBytesToWrite )
{
	int ret = 0;
	CIocpItem* pIocpItem = GetIocpItem();
	if (pIocpItem)
	{
		ret = pIocpItem->Write(lpBuffer,nNumberOfBytesToWrite);
		pIocpItem->Release();
	}

	return ret;
}

int CChannel::Write( const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen )
{
	int ret = 0;
	CIocpItem* pIocpItem = GetIocpItem();

	if (pIocpItem)
	{
		ret = pIocpItem->Write(lpBuffer,nNumberOfBytesToWrite,addr,addrlen);
		pIocpItem->Release();
	}

	return ret;
}

void CChannel::OnConnect( IChannelItem* pChannelItem, const char* IP,unsigned short Port )
{
	CChannelItem::OnConnect(pChannelItem,IP,Port);
}

void CChannel::OnWrite( IChannelItem* pChannelItem, unsigned int size,void* Param )
{
	CChannelItem::OnWrite(pChannelItem,size,Param);
}

void CChannel::OnRead( IChannelItem* pChannelItem, const char* Buffer,int Length,sockaddr_in* addr,int addrlen )
{
	CChannelItem::OnRead(pChannelItem,Buffer,Length,addr,addrlen);
}

void CChannel::OnClose( IChannelItem* pChannelItem,sockaddr_in* addr,int addrlen )
{
	CChannelItem::OnClose(pChannelItem,addr,addrlen);
}

const char* CChannel::GetRemoteIP()
{
	const char* ip = "127.0.0.1";
	CIocpItem* pIocpItem = GetIocpItem();
	if (pIocpItem)
	{
		ip = pIocpItem->GetRemoteIP();
	}
	return ip;
}

unsigned short CChannel::GetRemotePort()
{
	unsigned short port = 0;
	CIocpItem* pIocpItem = GetIocpItem();
	if (pIocpItem)
	{
		port = pIocpItem->GetRemotePort();
	}
	return port;
}

CIocpItem* CChannel::GetIocpItem()
{
	CIocpItem* pIocpItem = 0;
	m_Lock.Lock();
	if (m_pChannelItem)
	{
		IF_QUERYINTERFACE(m_pChannelItem,pIocpItem,CIocpItem)
		{

		}
	}
	m_Lock.Unlock();
	return pIocpItem;
}

CHANNEL_API bool InitChannelModule(int num)
{
	bool b = false;
	CIocp* pIocp = CIocp::CreateInstance();
	if (pIocp)
	{
		b = pIocp->Initailize(num);
		pIocp->Release();
	}
	return b;
}

CHANNEL_API void CloseChannelModule(void)
{
	CIocp* pIocp = CIocp::CreateInstance();
	if (pIocp)
	{
		pIocp->Close();
		pIocp->Release();
	}
}

CHANNEL_API IChannel* CreateChannel(void)
{
	return CChannel::CreateInstance();
}


