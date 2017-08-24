#include "./SocketEx.h"
#include "../PublicLibrary/log.h"
#include "../PublicLibrary/Macro.h"

#ifndef WIN32
#include <fcntl.h>
#else
//#if !defined(ANDROID)
//#include 襰ocket.h"
//#endif
#endif

CSocketEx::CSocketEx(void)
{
	m_Socket = INVALID_SOCKET;
}

CSocketEx::~CSocketEx(void)
{
	CloseSocket();
}

//创建
bool CSocketEx::Create(int nSocketType,SOCKET s)
{
	CloseSocket();

    m_Socket = s;
    if (m_Socket==INVALID_SOCKET)
    {
        m_Socket = socket( AF_INET, nSocketType, 0 );

    }
    if( m_Socket == INVALID_SOCKET )
	{
		GetLogger()->log(_LOG_ERROR,"socket failed(%d)\n",ERRNO);
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

	return true;
}

//绑定
bool CSocketEx::Bind(const char* IP, unsigned short Port)
{
	//bing地址结构
	sockaddr_in addr;
    ZERO_MEMORYQ(addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Port);
  
	if (IP&&(strlen(IP)>0))
	{
		addr.sin_addr.s_addr = inet_addr(IP);
	}
	//bing端口
	if ( bind(m_Socket, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
	{
		GetLogger()->log(_LOG_ERROR,"bind failed(%d)\n",ERRNO);
		return false; //返回错误
	}
	return true;
}

//断开
void CSocketEx::CloseSocket(void)
{
	if(m_Socket!=INVALID_SOCKET)
	{
		shutdown(m_Socket,SD_BOTH);
#ifdef WIN32
		closesocket(m_Socket);
#else
		close(m_Socket);
#endif
		m_Socket = INVALID_SOCKET;
	}
}

int CSocketEx::CheckConnectState()
{
    int error = -1;
    if (m_Socket!=INVALID_SOCKET) {
        //socklen_t size = sizeof(error);
        //getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, &error, &size);
        
// 		struct timeval timeout = { 0, 0 };
// 		fd_set readSocketSet;
// 		FD_ZERO(&readSocketSet);
// 		FD_SET(m_Socket, &readSocketSet);
// 		int iRet = ::select(0, &readSocketSet, NULL, NULL, &timeout);
// 		bool bOK = (iRet > 0);
// 
// 		if (bOK)
// 		{
// 			bOK = FD_ISSET(m_Socket, &readSocketSet);
// 		}
        error = (recv(m_Socket,0,0,MSG_PEEK)>=0)?0:-1;
    }
    return error;
}

//监听
bool CSocketEx::Listen(unsigned short Port)
{
	if((listen(m_Socket,SOMAXCONN)==SOCKET_ERROR))
	{
		GetLogger()->log(_LOG_ERROR,"Listen failed(%d)\n",ERRNO);
		return false;
	}
    return true;
}

//接受连接
SOCKET CSocketEx::Accept(struct sockaddr* addr,socklen_t* addrlen)
{
    return accept(m_Socket,addr,addrlen);
}

//连接
int CSocketEx::Connect(const char* IP,unsigned short Port)
{
	//初始化地址
	sockaddr_in addr; //地址结构
	memset((void *)&addr, 0, sizeof(addr)); 
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(IP);
	addr.sin_port = htons((u_short)Port);

	int ret = connect(m_Socket,(struct sockaddr *)&addr,sizeof(addr));
	if (ret!=SOCKET_ERROR)
	{
	}
	return ret;
}

//TCP
int CSocketEx::Send(const char* buf, int len)
{
	return (int)::send(m_Socket,buf,len,0);
}

int CSocketEx::Recv(char* buf,int len)
{
	return (int)::recv(m_Socket,buf,len,0);
}

//UDP
int CSocketEx::SendTo(const char* buf,int len,const struct sockaddr* to,socklen_t tolen)
{
    return (int)::sendto(m_Socket,buf,len,0,to,tolen);
}

int CSocketEx::RecvFrom(char* buf,int len,struct sockaddr* from,socklen_t* fromlen)
{
    return (int)::recvfrom(m_Socket,buf,len,0,from,fromlen);
}

const char* CSocketEx::GetRemoteIP()
{
	if(m_Socket != INVALID_SOCKET)
	{
		sockaddr_in addr;
		socklen_t len = sizeof(addr);
		ZERO_MEMORYQ(addr);
		if(getpeername(m_Socket,(sockaddr*)&addr,&len)==0)
		{
			return (const char*)inet_ntoa(addr.sin_addr);
		}
		else
		{
			GetLogger()->log(_LOG_ERROR,"CSocketEx::GetIP failed(%d)\n", ERRNO);
		}
	}
	return "127.0.0.1";
}

unsigned short CSocketEx::GetRemotePort()
{
	if(m_Socket != INVALID_SOCKET)
	{
		sockaddr_in addr;
		socklen_t len = sizeof(addr);
		ZERO_MEMORYQ(addr);
		if(getpeername(m_Socket,(sockaddr*)&addr,&len)==0)
		{
			return ntohs(addr.sin_port);
		}
		else
		{
			GetLogger()->log(_LOG_ERROR,"CSocketEx::GetPort failed(%d)\n", ERRNO);
		}
	}
	return 0;
}
