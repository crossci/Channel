#include "./IocpTcpServer.h"
#include "Iocp.h"
#include "../PublicLibrary/UnknownEx.h"
#include "../PublicLibrary/log.h"
#include "IocpTcpClient.h"

extern CIocp g_Iocp;

#pragma pack(push,1)
typedef struct _CONNECT_ADDR
{
	char n[10];
	sockaddr_in local_addr;
	char n1[12];
	sockaddr_in remote_addr;
}*PCONNECT_ADDR;
#pragma pack(pop)

CIocpTcpServer::CIocpTcpServer(void)
{
	m_AcceptItem = NULL;
}

CIocpTcpServer::~CIocpTcpServer(void)
{
	OnClose();
}

//≤È—ØΩ”ø⁄
bool CIocpTcpServer::QueryInterface( const GUID& guid, void **ppvObject)
{
	QUERYINTERFACE(CIocpTcpServer);
	IF_TRUE(QUERYINTERFACE_PARENT(CCircularMemory)||QUERYINTERFACE_PARENT(CIocpSocket)||QUERYINTERFACE_PARENT(CChannelItem));

	return false;
}

//≥ı ºªØ
bool CIocpTcpServer::Initailize(const char* Name,unsigned short Port)
{
	bool b = false;
	RandKey();
	if (Create())
	{
		if(Bind(Name,Port))
		{
			if (Listen(Port))
			{
				m_ReadOverlappedPlus.Opcode = _IOCP_ITEM::CONNECT;
				CIocp* pIocp = CIocp::CreateInstance();
				if (pIocp)
				{
					if (pIocp->Associate(this))
					{
						b = Receive();
					}
					pIocp->Release();
				}
			}
		}
	}

	return b;
}

const char* CIocpTcpServer::GetRemoteIP()
{
	return CIocpSocket::GetRemoteIP();
}

unsigned short CIocpTcpServer::GetRemotePort()
{
	return CIocpSocket::GetRemotePort();
}

// ∂¡ª∫¥Ê
#ifdef WIN32
int CIocpTcpServer::Read(void* lpBuffer, unsigned int nNumberOfBytesToRead)
{
	int b = 0;

	unsigned int NumberOfBytesRead = 0;
	CIocpItem* pIocpItem = AllocItem();
	if (pIocpItem)
	{
		pIocpItem->AddRef();
		m_AcceptItem = pIocpItem;
		int rc = AcceptEx(m_Socket,(SOCKET)pIocpItem->GetHandle(),m_Buffer,0,
			sizeof(sockaddr_in)+16,sizeof(sockaddr_in)+16,(LPDWORD)&NumberOfBytesRead,(LPOVERLAPPED)&m_ReadOverlappedPlus);
		if(rc)
		{
			b = 1;
		}
		else
		{
			unsigned int err = ERRNO;
			if(err==ERROR_IO_PENDING)
			{
				b = 1;
			}
			else
			{
				SAFE_RELEASE(m_AcceptItem);
				GetLogger()->log(_LOG_ERROR,"AcceptEx failed(%d)\n",ERRNO);
			}
		}

		pIocpItem->Release();
	}
	else
	{
		GetLogger()->log(_LOG_ERROR,"AllocItem failed(%d)\n",ERRNO);
	}
	return b;
}
#else //linux
int CIocpTcpServer::Read(void* lpBuffer, unsigned int nNumberOfBytesToRead)
{
	 return CIocpSocket::Read(lpBuffer, nNumberOfBytesToRead);
}
#endif

//∑÷≈‰œÓƒø
CIocpItem* CIocpTcpServer::AllocItem(SOCKET s)
{
	CIocpTcpClient* pIocpTcpClient = 0;
	pIocpTcpClient = CIocpTcpClient::CreateInstance();
	if (pIocpTcpClient)
	{
		//pIocpTcpClient->SetChannelEvent(this);
        pIocpTcpClient->Create(SOCK_STREAM,s);
	}
	return pIocpTcpClient;
}

//¡¨Ω”œ˚œ¢
void CIocpTcpServer::OnConnect(const char* IP,unsigned short Port)
{
#ifdef WIN32
	if (m_AcceptItem)
	{
		CIocp* pIocp = CIocp::CreateInstance();
		if (pIocp)
		{
			if (pIocp->Associate(m_AcceptItem))
			{
				if (m_pChannelEvent)
				{
					PCONNECT_ADDR pConnectAddr = (PCONNECT_ADDR)m_Buffer;
					const char* RemoteIP = (const char*)inet_ntoa(pConnectAddr->remote_addr.sin_addr);
					unsigned short RemotePort = ntohs(pConnectAddr->remote_addr.sin_port);

					IChannelItem* pChannelItem;
					IF_QUERYINTERFACE(m_AcceptItem,pChannelItem,IChannelItem)
					{
						m_pChannelEvent->OnConnect(pChannelItem, RemoteIP, RemotePort);
						pChannelItem->Release();
					}
					int err = setsockopt( (SOCKET)m_AcceptItem->GetHandle(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&m_Socket, sizeof(m_Socket) );
					m_AcceptItem->OnConnect(RemoteIP, RemotePort);
					m_AcceptItem->Receive();
				}
				else
				{
					m_AcceptItem->Close();
				}
			}
			pIocp->Release();
		}

		m_AcceptItem->Release();
		m_AcceptItem = NULL;
	}
#endif
}

void CIocpTcpServer::CloseEvent( void )
{
	AddRef();
	Receive();
}

//πÿ±’
void CIocpTcpServer::OnClose(void)
{
//	DebugBreak();
// 	CChannelItem::OnClose(this,NULL,0);
// 	CIocpSocket::OnClose();
}

void CIocpTcpServer::OnRead( void* lpBuffer, unsigned int nNumberOfBytesToRead )
{
    struct sockaddr_in addr;
    socklen_t len;
	SOCKET s = Accept((sockaddr*)&addr, &len);
    CIocpItem* pIocpItem = AllocItem(s);
	if (pIocpItem)
	{
        if (m_pChannelEvent)
        {
            const char* IP = (const char*)inet_ntoa(addr.sin_addr);
            unsigned short Port = ntohs(addr.sin_port);
            
            IChannelItem* pChannelItem;
			IF_QUERYINTERFACE(pIocpItem,pChannelItem,IChannelItem)
            {
                m_pChannelEvent->OnConnect(pChannelItem,IP,Port);
                pChannelItem->Release();
            }
            pIocpItem->OnConnect(IP,Port);
            CIocp* pIocp = CIocp::CreateInstance();
            if (pIocp)
            {
                if (pIocp->Associate(pIocpItem))
                {
                    
                }
                pIocp->Release();
            }
            pIocpItem->Receive();
        }
        else
        {
            pIocpItem->Close();
        }
        pIocpItem->Release();
    }
}

//πÿ±’
void CIocpTcpServer::Close(void)
{
	CIocpSocket::Close();
}

// –¥ª∫¥Ê
int CIocpTcpServer::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite)
{
	return CIocpSocket::Write(lpBuffer,nNumberOfBytesToWrite);
}

int CIocpTcpServer::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen)
{
	return CIocpSocket::Write(lpBuffer,nNumberOfBytesToWrite,addr,addrlen);
}

