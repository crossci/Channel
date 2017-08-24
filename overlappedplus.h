#pragma once

#ifdef WIN32
#include "windows.h"
#else
typedef struct _OVERLAPPED
{
}OVERLAPPED,*POVERLAPPED,*LPOVERLAPPED;
#endif // WIN32

typedef struct _OVERLAPPEDPLUS : public OVERLAPPED
{
	unsigned char Opcode;
	void* Item;
	void* Param;
}*POVERLAPPEDPLUS;