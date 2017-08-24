#include "./IocpSocket.h"
#include "../PublicLibrary/log.h"
#include "Iocp.h"

#define SOCKET_BUFF_SIZE	(1024*16) //Socket大小

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

//查询接口
bool CIocpSocket::QueryInterface( const GUID& guid, void **ppvObject)
{
    QUERYINTERFACE(CIocpSocket);
    IF_TRUE(QUERYINTERFACE_PARENT(CIocpItem));
    
    return false;
}

//获得句柄
HANDLE CIocpSocket::GetHandle(void)
{
    return (HANDLE)m_Socket;
}

//创建
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
	//经历TIME_WAIT,不强制关闭
	int flag = 0;
	setsockopt(m_Socket, SOL_SOCKET, SO_DONTLINGER, (char *)&flag, sizeof(flag));
#endif
    

	//发送数据再关闭
	struct linger lingerOpt;

	lingerOpt.l_onoff = 1;
	lingerOpt.l_linger = 0;

	setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (char *)&lingerOpt, sizeof(lingerOpt));

	int socklen;
	//设置接收缓存
	socklen = SOCKET_BUFF_SIZE*4;
	setsockopt( m_Socket, SOL_SOCKET, SO_RCVBUF, (char*)&socklen, sizeof(socklen));

	//设置发送缓存
	socklen = SOCKET_BUFF_SIZE*4;
	setsockopt( m_Socket, SOL_SOCKET, SO_SNDBUF, (char*)&socklen, sizeof(socklen));

	//设置心跳包
#ifdef WIN32
	unsigned int dwBytes = 0;
	struct tcp_keepalive keepin = { 0 };
	struct tcp_keepalive keepout = { 0 };

	keepin.keepaliveinterval = 1000;//10s 每10S发送1包探测报文，发5次没有回应，就断开
	keepin.keepalivetime = 5000;//60s 超过60S没有数据，就发送探测包
	keepin.onoff = 1;

	WSAIoctl(m_Socket, SIO_KEEPALIVE_VALS, &keepin, sizeof(keepin), &keepout, sizeof(keepout), (LPDWORD)&dwBytes, NULL, NULL);
#else
// 	int keepalive = 1; // 开启keepalive属性
// 	int keepidle = 5; // 如该连接在60秒内没有任何数据往来,则进行探测
// 	int keepinterval = 1; // 探测时发包的时间间隔为5 秒
// 	int keepcount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
// 	setsockopt(m_Socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive, sizeof(keepalive));
// 	setsockopt(m_Socket, SOL_TCP, TCP_KEEPIDLE, (void*)&keepidle, sizeof(keepidle));
// 	setsockopt(m_Socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval, sizeof(keepinterval));
// 	setsockopt(m_Socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount, sizeof(keepcount));
#endif


	return true;
}

//断开
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

//连接
int CIocpSocket::Connect(const char* IP,unsigned short Port)
{
	//初始化地址
	sockaddr_in addr; //地址结构
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

	//连接
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

//关闭
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
	int RetValue; //返回值
	WSABUF Buffer; //接收缓存
	unsigned int dwFlag=0; //长度和标志
	unsigned int dwRet = 0;

	Buffer.len = len; //缓存大小
	Buffer.buf = (char*)buf; //缓存

	//接收
	RetValue = WSASend( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, dwFlag, NULL, NULL);
	//判断返回值
	if ( RetValue == SOCKET_ERROR )
	{
		int last_err = ERRNO; //得到错误
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
	int RetValue; //返回值
	WSABUF Buffer; //接收缓存
	unsigned int dwFlag=0; //长度和标志
	unsigned int dwRet=0;

	Buffer.len = len; //缓存大小
	Buffer.buf = (char*)buf; //缓存

	//接收TCP
	RetValue = WSARecv( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, (LPDWORD)&dwFlag, (LPWSAOVERLAPPED)&m_ReadOverlappedPlus, NULL );

	//判断返回值
	if ( RetValue == SOCKET_ERROR )
	{
		int last_err = ERRNO; //得到错误
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
	int RetValue; //返回值
	unsigned int dwFlag=0; //长度和标志
	WSABUF Buffer; //接收缓存
	unsigned int dwRet = 0;
  
	Buffer.len = len; //缓存大小
	Buffer.buf = (char*)buf; //缓存
  
  	//接收 (LPWSAOVERLAPPED)&m_WriteOverlappedPlus
// 	if (pOverlapped)
// 	{
// 		((POVERLAPPEDPLUS)pOverlapped)->Opcode = _IOCP_ITEM_WRITE;
// 	}
  	RetValue = WSASendTo( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, dwFlag, to, tolen, 0, 0);
  	//判断返回值
  	if ( RetValue == SOCKET_ERROR )
  	{
  		int last_err = ERRNO; //得到错误
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
	int RetValue; //返回值
	WSABUF Buffer; //接收缓存
	unsigned int dwFlag=0; //长度和标志
	unsigned int dwRet=0;

	Buffer.len = len; //缓存大小
	Buffer.buf = (char*)buf; //缓存

	//接收TCP
	RetValue = WSARecvFrom( m_Socket, &Buffer, 1, (LPDWORD)&dwRet, (LPDWORD)&dwFlag, from, fromlen, (LPWSAOVERLAPPED)&m_ReadOverlappedPlus, NULL );

	//判断返回值
	if ( RetValue == SOCKET_ERROR )
	{
		int last_err = ERRNO; //得到错误
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

// 写缓存
int CIocpSocket::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite)
{
	return Send((char*)lpBuffer,nNumberOfBytesToWrite);
}

int CIocpSocket::Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* to,int len)
{
	return SendTo((char*)lpBuffer,nNumberOfBytesToWrite,to,len);
}

// 读缓存
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

