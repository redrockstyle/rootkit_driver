#include "rename_proc.h"

NTSTATUS HookNtQuerySystemInformation(
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
) {
	NTSTATUS retStatus;

	DbgPrint("a\n");

	retStatus = glRealNtQuerySystemInformation(
		SystemInformationClass,
		SystemInformation,
		SystemInformationLength,
		ReturnLength
	);

	return retStatus;
}