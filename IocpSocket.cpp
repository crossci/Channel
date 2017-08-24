#include "./IocpSocket.h"
#include "../PublicLibrary/log.h"
#include "Iocp.h"

#define SOCKET_BUFF_SIZE	(1024*16) //Socket��С

#ifdef WIN32
static GUID g_GuidConnectEx = WSAID_CONNECTEX;
#include <Winsock2.h> 
#include <mswsock.h> 
#include <mstcpip.h> 
#else
#include "fcntl.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>

#if !defined(ANDROID)

#define SOL_TCP 6
#define TCP_KEEPIDLE 4  
#define TCP_KEEPINTVL 5  
#define TCP_KEEPCNT 6 
#define _KEVENT
#endif

#endif


CIocpSocket::CIocpSocket(void)
{
}

CIocpSocket::~CIocpSocket(void)
{
}

//��ѯ�ӿ�
bool CIocpSocket::QueryInterface( const GUID& guid, void **ppvObject)
{
    QUERYINTERFACE(CIocpSocket);
    IF_TRUE(QUERYINTERFACE_PARENT(CIocpItem));
    
    return false;
}

//��þ��
HANDLE CIocpSocket::GetHandle(void)
{
    return (HANDLE)m_Socket;
}

//����
bool CIocpSocket::Create(int nSocketType,SOCKET s)
{
	CloseSocket();
	int ipproto = 0;
	switch(nSocketType)
	{
	case SOCK_STREAM:
		ipproto = IPPROTO_TCP;
		break;
	case SOCK_DGRAM:
		ipproto = IPPROTO_UDP;
		break;

	}
    m_Socket = s;
    if (m_Socket==INVALID_SOCKET)
    {
#ifdef WIN32
        m_Socket = WSASocket( AF_INET, nSocketType, ipproto, NULL, 0, WSA_FLAG_OVERLAPPED );
#else
        m_Socket = socket( AF_INET, nSocketType, 0 );
        if( m_Socket != INVALID_SOCKET )
        {
//            int flags = fcntl(m_Socket, F_GETFL, 0);
//            if(flags < 0) {
//                return 0;
//            }
//            fcntl(m_Socket, F_SETFL, flags | O_NONBLOCK);
        }
#endif
    }
	if( m_Socket == INVALID_SOCKET )
	{
		GetLogger()->log(_LOG_ERROR,"WSASocket failed(%d)\n",ERRNO);
		return false;
	}

#ifdef WIN32
	//����TIME_WAIT,��ǿ�ƹر�
	int flag = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (char *)&flag, sizeof(flag));
#endif
    

	//���������ٹر�
	struct linger lingerOpt;

	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;

	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt));

	int socklen;
	//���ý��ջ���
	socklen = SOCKET_BUFF_SIZE*4;
	setsockopt( m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&socklen, sizeof(socklen));

	//���÷��ͻ���
	socklen = SOCKET_BUFF_SIZE*4;
	setsockopt( m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&socklen, sizeof(socklen));

	//����������
#ifdef WIN32
	unsigned int dwBytes = 0;
	struct tcp_keepalive keepin = { 0 };
	struct tcp_keepalive keepout = { 0 };

	keepin.keepaliveinterval = 1000;//10s ÿ10S����1��̽�ⱨ�ģ���5��û�л�Ӧ���ͶϿ�
	keepin.keepalivetime = 5000;//60s ����60Sû�����ݣ��ͷ���̽���
	keepin.onoff = 1;

	WSAIoctl(m_Socket, SIO_KEEPALIVE_VALS, &keepin, sizeof(keepin), &keepout, sizeof(keepout), (LPDWORD)&dwBytes, NULL, NULL);
#else
// 	int keepalive = 1; // ����keepalive����
// 	int keepidle = 5; // ���������60����û���κ���������,�����̽��
// 	int keepinterval = 1; // ̽��ʱ������ʱ����Ϊ5 ��
// 	int keepcount = 3; // ̽�Ⳣ�ԵĴ���.�����1��̽������յ���Ӧ��,���2�εĲ��ٷ�.
// 	setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
// 	setsockopt(m_Socket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle, sizeof(keepidle));
// 	setsockopt(m_Socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
// 	setsockopt(m_Socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
#endif


	return true;
}

//�Ͽ�
void CIocpSocket::CloseSocket(void)
{
	CSocketEx::CloseSocket();
}

int CIocpSocket::CheckConnectState()
{
    return CSocketEx::CheckConnectState();
}

void CIocpSocket::OnClose()
{
#ifndef WIN32
#ifndef _KEVENT
// 	CIocp* pIocp = CIocp::CreateInstance();
// 	if (pIocp)
// 	{
// 		pIocp->Associate(this,2);
// 		pIocp->Release();
// 	}
#endif
#endif

	CloseSocket();
	CIocpItem::OnClose();
}

//����
int CIocpSocket::Connect(const char* IP,unsigned short Port)
{
	//��ʼ����ַ
	sockaddr_in addr; //��ַ�ṹ
	memset((void *)&addr, 0, sizeof(addr)); 
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons((u_short)Port);

	int dwErr = 0;
#ifdef WIN32
	static LPFN_CONNECTEX lpfnConnectEx = 0;
	if (!lpfnConnectEx)
	{
		unsigned int dwBytes = 0;
		m_ReadOverlappedPlus.Opcode = _IOCP_ITEM::CONNECT;
		dwErr = WSAIoctl(m_Socket, SIO_GET_EXTENSION_FUNCTION_POINTER, &g_GuidConnectEx, sizeof(g_GuidConnectEx), &lpfnConnectEx, sizeof(lpfnConnectEx), (LPDWORD)&dwBytes, NULL, NULL); 
		if(dwErr==SOCKET_ERROR) 
		{
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::Connect WSAIoctl failed(%d)\n",ERRNO);
			return FALSE;
		}
	}

	Bind("0.0.0.0",0);
#endif

	//����
	unsigned int sendsize = 0;
#ifdef WIN32
	int result = lpfnConnectEx( m_Socket,(struct sockaddr *)&addr,sizeof(addr), NULL, NULL, NULL, (LPWSAOVERLAPPED)&m_ReadOverlappedPlus );
    if (result==0)
#else
	int result = connect(m_Socket,(struct sockaddr *)&addr,sizeof(addr));
    if (result!=0)
#endif
	
	{
#ifdef WIN32
		dwErr = ERRNO;
		if (dwErr==ERROR_IO_PENDING)
		{
			return true;
		}
#else
        if (errno==EINPROGRESS)
        {
            return true;
        }
#endif
		//
		return false;
	}
#ifndef WIN32
	// ConnectEvent();
#endif 
	return true;
}

//�ر�
void CIocpSocket::Close(void)
{
	CIocpItem::Close();
#ifndef WIN32
    CloseSocket();
#endif
}

//TCP
int CIocpSocket::Send(const char* buf, int len)
{
#ifdef WIN32
	int RetValue; //����ֵ
	WSABUF Buffer; //���ջ���
	unsigned int dwFlag=0; //���Ⱥͱ�־
	unsigned int dwRet = 0;

	Buffer.len = len; //�����С
	Buffer.buf = (char*)buf; //����

	//����
	RetValue = WSASend( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, dwFlag, NULL, NULL);
	//�жϷ���ֵ
	if ( RetValue == SOCKET_ERROR )
	{
		int last_err = ERRNO; //�õ�����
		switch(last_err) 
		{
		case WSA_IO_PENDING:
			break;
		case WSAEWOULDBLOCK:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::Send WSASend failed(%d)\n",ERRNO);
			//printf("SOCKET WSAEWOULDBLOCK:%d,len=%d",ERRNO,len);
			return -1;
			break;
		default:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::Send WSASend failed(%d)\n",ERRNO);
			//printf("SOCKET ERROR:%d,len=%d",ERRNO,len);
			return -2;
			break;
		}
	}
	return dwRet;
#else
	return CSocketEx::Send(buf,len);
#endif
}

int CIocpSocket::Recv(char* buf,int len)
{
#ifdef WIN32
	int RetValue; //����ֵ
	WSABUF Buffer; //���ջ���
	unsigned int dwFlag=0; //���Ⱥͱ�־
	unsigned int dwRet=0;

	Buffer.len = len; //�����С
	Buffer.buf = (char*)buf; //����

	//����TCP
	RetValue = WSARecv( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, (LPDWORD)&dwFlag, (LPWSAOVERLAPPED)&m_ReadOverlappedPlus, NULL );

	//�жϷ���ֵ
	if ( RetValue == SOCKET_ERROR )
	{
		int last_err = ERRNO; //�õ�����
		switch(last_err)
		{
		case WSA_IO_PENDING:
			break;
		case WSAEWOULDBLOCK:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::Recv WSARecv failed(%d)\n",ERRNO);
			return -1;
			break;
		default:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::Recv WSARecv failed(%d)\n",ERRNO);
			return -2;
			break;
		}
	}
	return dwRet;
#else //linux
	int b = 0;
#ifdef _KEVENT
	b = (m_Socket!=INVALID_SOCKET)?1:0;
#else
	CIocp* pIocp = CIocp::CreateInstance();
	if (pIocp)
	{
		if(pIocp->Associate(this,3))
		{
			b = 1;
		}
		pIocp->Release();
	}
#endif
	return b;
#endif
}

//UDP
int CIocpSocket::SendTo(const char* buf,int len,const struct sockaddr* to,int tolen)
{
#ifdef WIN32
	int RetValue; //����ֵ
	unsigned int dwFlag=0; //���Ⱥͱ�־
	WSABUF Buffer; //���ջ���
	unsigned int dwRet = 0;
  
	Buffer.len = len; //�����С
	Buffer.buf = (char*)buf; //����
  
  	//���� (LPWSAOVERLAPPED)&m_WriteOverlappedPlus
// 	if (pOverlapped)
// 	{
// 		((POVERLAPPEDPLUS)pOverlapped)->Opcode = _IOCP_ITEM_WRITE;
// 	}
  	RetValue = WSASendTo( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, dwFlag, to, tolen, 0, 0);
  	//�жϷ���ֵ
  	if ( RetValue == SOCKET_ERROR )
  	{
  		int last_err = ERRNO; //�õ�����
  		switch(last_err) 
  		{
  		case WSA_IO_PENDING:
  			break;
  		case WSAEWOULDBLOCK:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::SendTo WSASendTo failed(%d)\n",ERRNO);
  			//printf("SOCKET WSAEWOULDBLOCK:%d,len=%d",ERRNO,len);
  			return -1;
  			break;
  		default:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::SendTo WSASendTo failed(%d)\n",ERRNO);
  			//printf("SOCKET ERROR:%d,len=%d",ERRNO,len);
  			return -2;
  			break;
  		}
  	}

	return dwRet;
#else
	return SendTo(buf,len,to,tolen);
#endif
}

int CIocpSocket::RecvFrom(char* buf,int len,struct sockaddr* from,int* fromlen)
{
#ifdef WIN32
	int RetValue; //����ֵ
	WSABUF Buffer; //���ջ���
	unsigned int dwFlag=0; //���Ⱥͱ�־
	unsigned int dwRet=0;

	Buffer.len = len; //�����С
	Buffer.buf = (char*)buf; //����

	//����TCP
	RetValue = WSARecvFrom( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, (LPDWORD)&dwFlag, from, fromlen, (LPWSAOVERLAPPED)&m_ReadOverlappedPlus, NULL );

	//�жϷ���ֵ
	if ( RetValue == SOCKET_ERROR )
	{
		int last_err = ERRNO; //�õ�����
		switch(last_err)
		{
		case WSA_IO_PENDING:
			break;
		case WSAEWOULDBLOCK:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::RecvFrom WSARecvFrom failed(%d)\n",ERRNO);
			return -1;
			break;
		default:
			GetLogger()->log(_LOG_ERROR,"CIocpSocket::RecvFrom WSARecvFrom failed(%d)\n",ERRNO);
			return -2;
			break;
		}
	}
	return dwRet;
#else
	return RecvFrom(buf,len,from,fromlen);
#endif
}

// д����
int CIocpSocket::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite)
{
	return Send((char*)lpBuffer,nNumberOfBytesToWrite);
}

int CIocpSocket::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* to,int len)
{
	return SendTo((char*)lpBuffer,nNumberOfBytesToWrite,to,len);
}

// ������
int CIocpSocket::Read(void* lpBuffer, unsigned int nNumberOfBytesToRead)
{
    return Recv((char*)lpBuffer,nNumberOfBytesToRead);
}

const char* CIocpSocket::GetRemoteIP()
{
	return CSocketEx::GetRemoteIP();
}

unsigned short CIocpSocket::GetRemotePort()
{
	return CSocketEx::GetRemotePort();
}

