#pragma once
#include "../PublicLibrary/ThreadManage.h"
#include "../PublicLibrary/UnknownEx.h"
#include "../PublicLibrary/Macro.h"
#include "overlappedplus.h"

class CIocpItem;
class IIocpItem;
class IIocpEvent
{
public:
	//分配项目
	virtual CIocpItem* OnAllocItem(void) = 0;
	//连接
	virtual void OnConnect(IIocpItem* pIocpItem, const char* IP,unsigned short Port) = 0;
	//接收数据
	virtual void OnRead(IIocpItem* pIocpItem, char* Buffer,int Length) = 0;
	//关闭
	virtual void OnClose(IIocpItem* pIocpItem) = 0;
};

// {397BC077-D0E9-4705-BABE-2B61C6DB07F6}
static const GUID GUID_OF(CIocp) = { 0x397bc077, 0xd0e9, 0x4705, { 0xba, 0xbe, 0x2b, 0x61, 0xc6, 0xdb, 0x7, 0xf6 } };
class CIocp : public CThreadManage
{
private:
	// 完成端口句柄
	HANDLE m_hHandle;
	//循环标志
	bool m_Loop;
	//退出事件
	OVERLAPPED m_Overlapped;
public:
	CIocp(void);
	virtual ~CIocp(void);

	__QueryInterface;
public:
	// 初始化
	virtual bool Initailize(unsigned int WorkerNum=1);
	//关闭
	virtual void Close(void);
	//处理函数
	virtual void DoWorkThread(void);
public:
	//完成端口和ID绑定
	bool Associate(CIocpItem* pIocpItem,int op=1);
	// 发送消息
	virtual bool PostQueuedCompletionStatus(unsigned int dwTransferred,unsigned long long dwCompletionKey,LPOVERLAPPED lpOverlapped);
public:
	STATIC_CREATE(CIocp);
};
