#include "./IocpTcpClient.h"
#include "Iocp.h"
#include "../PublicLibrary/UnknownEx.h"

#define MAX_PACKET_SIZE		(1024*8) //封包大小

CIocpTcpClient::CIocpTcpClient(void)
{
}

CIocpTcpClient::~CIocpTcpClient(void)
{
	OnClose();
}

//查询接口
bool CIocpTcpClient::QueryInterface( const GUID& guid, void **ppvObject)
{
	QUERYINTERFACE(CIocpTcpClient);
	IF_TRUE(QUERYINTERFACE_PARENT(CCircularMemory)||QUERYINTERFACE_PARENT(CIocpSocket)||QUERYINTERFACE_PARENT(CChannelItem));

	return false;
}

//初始化
bool CIocpTcpClient::Initailize(const char* Name,unsigned short Port)
{
    bool b = false;
	RandKey();
	if (Create())
	{
		CIocp* pIocp = CIocp::CreateInstance();
		if (pIocp)
		{
#ifdef WIN32
			if (pIocp->Associate(this))
#endif
			{
				m_ReadOverlappedPlus.Opcode = _IOCP_ITEM::CONNECT;
				if (Connect(Name,Port))
				{
#ifndef WIN32
                    if (pIocp->Associate(this))
#endif
                    {
#ifndef WIN32
                        OnConnect(Name, Port);
#endif
                        b = true;
                    }
				}
			}
			pIocp->Release();
		}
	}

    return b;
}

const char* CIocpTcpClient::GetRemoteIP()
{
	return CIocpSocket::GetRemoteIP();
}

unsigned short CIocpTcpClient::GetRemotePort()
{
	return CIocpSocket::GetRemotePort();
}

int CIocpTcpClient::CheckConnectState()
{
    return CIocpSocket::CheckConnectState();
}

//连接
void CIocpTcpClient::OnConnect(const char* IP,unsigned short Port)
{
	m_ReadOverlappedPlus.Opcode = _IOCP_ITEM::RECEIVE;
	CChannelItem::OnConnect(this,IP,Port);
}

//关闭
void CIocpTcpClient::OnClose(void)
{
	CChannelItem::OnClose(this,NULL,0);
	CIocpSocket::OnClose();
}

void CIocpTcpClient::OnRead( void* lpBuffer, unsigned int nNumberOfBytesToRead )
{
#ifdef WIN32
    CChannelItem::OnRead(this,(const char*)lpBuffer,nNumberOfBytesToRead,0,0);
#else
    int r = CSocketEx::Recv((char*)lpBuffer, sizeof(m_Buffer));
    if(r>=0)
    {
        if(r>0)
        {
            nNumberOfBytesToRead = r;
            CChannelItem::OnRead(this,(const char*)lpBuffer,nNumberOfBytesToRead,0,0);
        }
    }
    else
    {
        Close();
    }
#endif
}

//关闭
void CIocpTcpClient::Close(void)
{
	CIocpSocket::Close();
}

// 写缓存
int CIocpTcpClient::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite)
{
	return CIocpSocket::Write(lpBuffer,nNumberOfBytesToWrite);
}

int CIocpTcpClient::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen)
{
	return CIocpSocket::Write(lpBuffer,nNumberOfBytesToWrite,addr,addrlen);
}



