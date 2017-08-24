#pragma once
#include "IocpSocket.h"
#include "../PublicLibrary/MemoryPool.h"
#include "ChannelItem.h"

// {7D349A55-BF5A-423E-A43C-24AB5E8E463E}
static const GUID GUID_OF(CIocpTcpServer) = { 0x7d349a55, 0xbf5a, 0x423e, { 0xa4, 0x3c, 0x24, 0xab, 0x5e, 0x8e, 0x46, 0x3e } };
class CIocpTcpServer : public CCircularMemory, public CIocpSocket, public CChannelItem
{
private:
	CIocpItem* m_AcceptItem;
public:
	CIocpTcpServer(void);
	virtual ~CIocpTcpServer(void);

	MEMORY_INTERFACE;
public:
	//��ʼ��
	virtual bool Initailize(const char* Name,unsigned short Port);
public:
	//
	virtual const char* GetRemoteIP();
	//
	virtual unsigned short GetRemotePort();
public:
	// ������
	virtual int Read(void* lpBuffer, unsigned int nNumberOfBytesToRead);
	//������Ŀ
	virtual CIocpItem* AllocItem(SOCKET s = INVALID_SOCKET);
public:
	//������Ϣ
	virtual void OnConnect(const char* IP,unsigned short Port);
	//�ر�
	virtual void OnClose(void);
    //���յ�������Ϣ
	virtual void OnRead(void* lpBuffer, unsigned int nNumberOfBytesToRead);
	//�ر��¼�
	virtual void CloseEvent(void);
	//�ر�
	virtual void Close(void);
	// д����
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite);
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen);
	POOL_CREATE(CIocpTcpServer);
};
