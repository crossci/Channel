#include "IocpItem.h"
#include "Iocp.h"
#include "../PublicLibrary/UnknownEx.h"
#include "../PublicLibrary/Macro.h"

CIocpItem::CIocpItem(void)
{
	ZERO_MEMORYQ(m_ReadOverlappedPlus);
	m_ReadOverlappedPlus.Item = this;
}

CIocpItem::~CIocpItem(void)
{
}

//查询接口
bool CIocpItem::QueryInterface( const GUID& guid, void **ppvObject)
{
	QUERYINTERFACE(CIocpItem);
	QUERYINTERFACE(IIocpItem);
	IF_TRUE(QUERYINTERFACE_PARENT(CUnknownEx));

	return false;
}


void CIocpItem::ConnectEvent( void )
{
	OnConnect(0,0);
}

void CIocpItem::WriteEvent( unsigned int size,void* Param )
{
	OnWrite(size,Param);
}

void CIocpItem::ReceiveEvent( unsigned int size )
{
	OnRead(m_Buffer,size);
}

void CIocpItem::CloseEvent( void )
{
	m_ReadOverlappedPlus.Opcode = _IOCP_ITEM::UNKNOWN;
	RandKey();
	OnClose();
}

//关闭
void CIocpItem::Close(void)
{
	m_Lock.Lock();
	if (m_ReadOverlappedPlus.Opcode>_IOCP_ITEM::CLOSE)
	{
		m_ReadOverlappedPlus.Opcode = _IOCP_ITEM::CLOSE;
		CIocp* pIocp = CIocp::CreateInstance();
		if (pIocp)
		{
			pIocp->PostQueuedCompletionStatus(0,GetLowKey(),(LPOVERLAPPED)&m_ReadOverlappedPlus);
			pIocp->Release();
		}
	}
	m_Lock.Unlock();
}

bool CIocpItem::Receive(void)
{
	bool b = false;
	if(Read( m_Buffer, sizeof(m_Buffer) )>0)
	{
		b = true;
	}
	else
	{
#ifdef WIN32
		if(ERRNO==ERROR_IO_PENDING)
		{
			b = true;
		}
#endif
	}
	return b;
}
