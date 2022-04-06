#ifndef _RENAME_PROC_H_
#define _RENAME_PROC_H_

#include <ntddk.h>
#define NUMBER_NT_QUERY_SYSTEM_INFORMATION 0xAD // 173

//for SYSTEM_INFORMATION_CLASS
//but wdk cannot open include file
//#include <Winternl.h>
//struct taked from Winternl.h
typedef enum _SYSTEM_INFORMATION_CLASS {
	SystemBasicInformation = 0,
	SystemPerformanceInformation = 2,
	SystemTimeOfDayInformation = 3,
	SystemProcessInformation = 5,
	SystemProcessorPerformanceInformation = 8,
	SystemInterruptInformation = 23,
	SystemExceptionInformation = 33,
	SystemRegistryQuotaInformation = 37,
	SystemLookasideInformation = 45
} SYSTEM_INFORMATION_CLASS;

typedef NTSTATUS(*NT_QUERY_SYSTEM_INFORMATION) (
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
	);

NTSTATUS HookNtQuerySystemInformation(
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
);

NT_QUERY_SYSTEM_INFORMATION glRealNtQuerySystemInformation;

#endif