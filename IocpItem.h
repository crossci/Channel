#pragma once
#include "../PublicLibrary/UnknownEx.h"
#include "../PublicLibrary/VerifyID.h"
#include "../PublicLibrary/Lock.h"
#include "overlappedplus.h"
#include "../PublicLibrary/Macro.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#endif // WIN32


#define IOCP_ITEM_BUFF_SIZE	(1024*4) //Socket��С

namespace _IOCP_ITEM
{
	enum 
	{
		UNKNOWN = 0,
		CLOSE,
		CONNECT,
		WRITE,
		RECEIVE,
	};
}

// {45CE4566-1E11-4BC5-B28E-4921AC59DACC}
static const GUID GUID_OF(IIocpItem) = { 0x45ce4566, 0x1e11, 0x4bc5, { 0xb2, 0x8e, 0x49, 0x21, 0xac, 0x59, 0xda, 0xcc } };
class IIocpItem : public IUnknownEx
{
public:
	// д����
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite) = 0;
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen) = 0;

	//�ر�
	virtual void Close(void) = 0;
};

class CIocp;
// {B68F17E7-64C2-483C-8B94-53AEB1820148}
static const GUID GUID_OF(CIocpItem) = { 0xb68f17e7, 0x64c2, 0x483c, { 0x8b, 0x94, 0x53, 0xae, 0xb1, 0x82, 0x1, 0x48 } };
class CIocpItem : public CUnknownEx, public IIocpItem, public CRandomKey
{
protected:
	//��������
	_OVERLAPPEDPLUS m_ReadOverlappedPlus;
	//����
	char m_Buffer[IOCP_ITEM_BUFF_SIZE];
	//
	CLock m_Lock;
public:
	CIocpItem(void);
	virtual ~CIocpItem(void);

	UNKNOWNEX_INTERFACE(CUnknownEx);
public:
	//�����¼�
	virtual void ConnectEvent(void);
	//�����ֽ��¼�
	virtual void WriteEvent(unsigned int size,void* Param);
	//�����ֽ��¼�
	virtual void ReceiveEvent(unsigned int size);
	//�ر��¼�
	virtual void CloseEvent(void);

	//������Ϣ
	virtual void OnConnect(const char* IP,unsigned short Port){};
	//����������Ϣ
	virtual void OnWrite(unsigned int size,void* Param){};
	//���յ�������Ϣ
	virtual void OnRead(void* lpBuffer, unsigned int nNumberOfBytesToRead){};
	//�ر���Ϣ
	virtual void OnClose(void){};
public:
	//�ر�
	virtual void Close(void);
	//����
	virtual bool Receive(void);
public:
	//��þ��
	virtual HANDLE GetHandle(void) = 0;
    //
    virtual int CheckConnectState() = 0;
	//
	virtual const char* GetRemoteIP() = 0;
	virtual unsigned short GetRemotePort() = 0;
public:
	//��ʼ��
	virtual bool Initailize(const char* Name,unsigned short Port) = 0;
	// д����
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite)=0;
	virtual int Write(const void* lpBuffer, unsigned int nNumberOfBytesToWrite,sockaddr* addr,int addrlen) = 0;
	// ������
	virtual int Read(void* lpBuffer, unsigned int nNumberOfBytesToRead)=0;
};
