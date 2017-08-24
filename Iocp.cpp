#include "Iocp.h"
#include <iostream>
#include "IocpItem.h"
#include "../PublicLibrary/log.h"
#include "../PublicLibrary/Macro.h"



#ifdef WIN32
#include "winsock2.h"
#pragma comment(lib,"ws2_32.lib")
#else
#include <fcntl.h>
#if !defined(ANDROID)
#define _KEVENT
#endif
#ifdef _KEVENT
#include <sys/types.h>
#include <sys/event.h>
#include <unistd.h>
#include <stdio.h>
#else
#include <sys/epoll.h>
#endif  //_KEVENT
#endif

#define MAX_EVENT 20

CIocp::CIocp(void)
{
#ifdef WIN32
	//初始化socket版本
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested = MAKEWORD( 2, 2 );
	WSAStartup( wVersionRequested, &wsaData );
#endif
	m_hHandle = INVALID_HANDLE_VALUE;
	m_Loop = false;
	ZERO_MEMORYQ(m_Overlapped);
}

CIocp::~CIocp(void)
{
	//关闭socket
	Close();
#ifdef WIN32
	WSACleanup();
#endif
}

//查询接口
bool CIocp::QueryInterface( const GUID& guid, void **ppvObject)
{
	QUERYINTERFACE(CIocp);
	IF_TRUE(QUERYINTERFACE_PARENT(CThreadManage));

	return false;
}

// 初始化
bool CIocp::Initailize(unsigned int WorkerNum)
{
	Close();

#ifdef WIN32
	m_hHandle = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0 );
#else
#ifdef _KEVENT
	m_hHandle = kqueue();
#else
	m_hHandle = epoll_create(WorkerNum);
#endif
#endif // WIN32
	//是否自动设置使用资源
	if( WorkerNum==0 )
	{
		GetLogger()->log(_LOG_ERROR,"CIocp::Initailize WorkerNum=0.");
		return false;
	}

	m_Loop = true;

	if(CreateThread(WorkerNum)!=WorkerNum)
	{
		GetLogger()->log(_LOG_ERROR,"CIocp::Initailize CreateThread failed(%d).",ERRNO);
		return false;
	}

	return true;
}

// 发送消息
bool CIocp::PostQueuedCompletionStatus(unsigned int dwTransferred,unsigned long long dwCompletionKey,LPOVERLAPPED lpOverlapped)
{
#ifdef WIN32
	return ::PostQueuedCompletionStatus(m_hHandle,dwTransferred,(ULONG_PTR)dwCompletionKey,lpOverlapped);
#else
	return false;
#endif
}

//关闭
void CIocp::Close(void)
{
	//释放
	if(m_hHandle!=INVALID_HANDLE_VALUE)
	{
#ifdef WIN32
		CloseHandle(m_hHandle);
#else
		close(m_hHandle);
#endif
		m_hHandle = INVALID_HANDLE_VALUE;
	}

	m_Loop = false;
	while(!IsEmptyThread())
	{
#ifdef WIN32
		Sleep(100);
#else
		sleep(1);
#endif
		
	}
}

//完成端口和ID绑定
bool CIocp::Associate(CIocpItem* pIocpItem,int op)
{
	bool b = false;
	if (pIocpItem)
	{
		CIocpItem* p;
		IF_QUERYINTERFACE(pIocpItem,p,CIocpItem)
		{
#ifdef WIN32
			HANDLE h = pIocpItem->GetHandle();
			int key = pIocpItem->GetLowKey();
			HANDLE hTemp = CreateIoCompletionPort(h,  m_hHandle, (ULONG_PTR)key, 0 );
			b = (hTemp==m_hHandle);
			if (b)
            {
                p->AddRef();
            }
            else
			{
				GetLogger()->log(_LOG_ERROR,"CIocp::Associate QueryInterface CreateIoCompletionPort failed(%d).",ERRNO);
				pIocpItem->RandKey();
			}
#else
 
#ifdef _KEVENT
            {
                struct kevent ev;
                EV_SET(&ev, p->GetHandle(), EVFILT_READ, EV_ADD, 0, 0, p);
                if(kevent(m_hHandle, &ev, 1, 0, 0, 0)==0)
                {
                    b = true;
                    p->AddRef();
                }
            }
#else
#define EPOLLONESHOT (1 << 30)
			epoll_event ev = {0};
			ev.events = EPOLLIN|EPOLLET|EPOLLONESHOT;
			ev.data.ptr = pIocpItem;
			if(epoll_ctl(m_hHandle,op,p->GetHandle(),&ev)==0)
			{
				b = true;
				if (op==1)
				{
					p->AddRef();
				}
			}
#endif

#endif
            p->Release();
		}
	}
	return b;
}

//处理函数
void CIocp::DoWorkThread(void)
{
	//项目
	CIocpItem *pIocpItem = 0,*p=0;
	//返回大小
    int dwSize = 0;
#ifdef WIN32
	//返回值
	BOOL bRet = false;
	//返回结构
	POVERLAPPEDPLUS pOverlappedPlus = NULL;

	DWORD err = 0;
	int key;
	
	//循环
	while(m_Loop)
	{
		//监听完成端口,有数据才会往下运行
		key = 0;
		bRet = GetQueuedCompletionStatus(m_hHandle, (LPDWORD)&dwSize, (PULONG_PTR)&key, (LPOVERLAPPED*)&pOverlappedPlus, INFINITE);
		//有信息判断是否正确
		if(pOverlappedPlus)
		{
			pIocpItem = (CIocpItem*)pOverlappedPlus->Item;
			if (pIocpItem&&pIocpItem->CheckLowKey(key))
			{
				IF_QUERYINTERFACE(pIocpItem,p,CIocpItem)
				{
					if (bRet)
					{
						//引用
						switch( pOverlappedPlus->Opcode )
						{
						case _IOCP_ITEM::CONNECT:
							p->ConnectEvent();
							if(!p->Receive())
							{
								//关闭
								p->Close();
							}
							break;
						case _IOCP_ITEM::WRITE:
							p->WriteEvent(dwSize,pOverlappedPlus->Param);
							break;
						case _IOCP_ITEM::RECEIVE:
							if(dwSize==0)
							{
								//关闭
								p->Close();
								break;
							}
							p->ReceiveEvent(dwSize);
							if(!p->Receive())
							{
								//关闭
								p->Close();
							}
							break;
						case _IOCP_ITEM::CLOSE:
							//关闭
							p->CloseEvent();
							p->Release();
							break;
						}
					}
					else
					{
						//关闭
						p->CloseEvent();
						p->Release();
					}					
					p->Release();
				}
			}
		}		
	}
#else
#ifdef _KEVENT
	struct kevent events[MAX_EVENT];
#else
	epoll_event events[MAX_EVENT];
#endif
	//循环
	while(m_Loop)
	{
#ifdef _KEVENT
		dwSize = kevent(m_hHandle, NULL, 0, events, MAX_EVENT, NULL);
#else
		dwSize = epoll_wait(m_hHandle,events,MAX_EVENT,-1);
#endif // _KEVENT
		for(int i=0;i<dwSize;++i)
		{
#ifdef _KEVENT
			pIocpItem = (CIocpItem*)events[i].udata;
#else
			pIocpItem = (CIocpItem*)events[i].data.ptr;
#endif
			if (pIocpItem)
			{
				IF_QUERYINTERFACE(pIocpItem,p,CIocpItem)
				{
#ifdef _KEVENT
					//if (events[i].fflags & NOTE_DELETE)
#else
					if (events[i].events&EPOLLIN)
#endif
					{
						p->ReceiveEvent(0);
						if(!p->Receive())
						{
							//关闭
                            p->CloseEvent();
                            p->Release();
						}
					}
					p->Release();
				}
			}
		}
	}
#endif
}