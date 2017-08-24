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
	//������Ŀ
	virtual CIocpItem* OnAllocItem(void) = 0;
	//����
	virtual void OnConnect(IIocpItem* pIocpItem, const char* IP,unsigned short Port) = 0;
	//��������
	virtual void OnRead(IIocpItem* pIocpItem, char* Buffer,int Length) = 0;
	//�ر�
	virtual void OnClose(IIocpItem* pIocpItem) = 0;
};

// {397BC077-D0E9-4705-BABE-2B61C6DB07F6}
static const GUID GUID_OF(CIocp) = { 0x397bc077, 0xd0e9, 0x4705, { 0xba, 0xbe, 0x2b, 0x61, 0xc6, 0xdb, 0x7, 0xf6 } };
class CIocp : public CThreadManage
{
private:
	// ��ɶ˿ھ��
	HANDLE m_hHandle;
	//ѭ����־
	bool m_Loop;
	//�˳��¼�
	OVERLAPPED m_Overlapped;
public:
	CIocp(void);
	virtual ~CIocp(void);

	__QueryInterface;
public:
	// ��ʼ��
	virtual bool Initailize(unsigned int WorkerNum=1);
	//�ر�
	virtual void Close(void);
	//������
	virtual void DoWorkThread(void);
public:
	//��ɶ˿ں�ID��
	bool Associate(CIocpItem* pIocpItem,int op=1);
	// ������Ϣ
	virtual bool PostQueuedCompletionStatus(unsigned int dwTransferred,unsigned long long dwCompletionKey,LPOVERLAPPED lpOverlapped);
public:
	STATIC_CREATE(CIocp);
};
