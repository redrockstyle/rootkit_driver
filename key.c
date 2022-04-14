#include "key.h"

NTSTATUS NTAPI HookNtEnumerateKey(
	HANDLE                KeyHandle,
	ULONG                 Index,
	KEY_INFORMATION_CLASS KeyInformationClass,
	PVOID                 KeyInformation,
	ULONG                 Length,
	PULONG                ResultLength
) {
	NTSTATUS retStatus;


	retStatus = glRealNtEnumerateKey(KeyHandle,
		Index,
		KeyInformationClass,
		KeyInformation,
		Length,
		ResultLength);

	if (KeyInformationClass == KeyBasicInformation) {
		PKEY_BASIC_INFORMATION pbi = (PKEY_BASIC_INFORMATION)KeyInformation;
		if (pbi) {
			if (pbi->NameLength != 0 && pbi->Name != NULL) {
				PLIST_ENTRY pLink;
				for (pLink = glTaskQueueKey.Flink; pLink != &glTaskQueueKey; pLink = pLink->Flink) {
					PTASK_QUEUE_KEY task = CONTAINING_RECORD(pLink, TASK_QUEUE_KEY, link);
					if (!wcsncmp(pbi->Name, task->target, pbi->NameLength / sizeof(WCHAR))) {
						DbgPrint("BASIC\n");
						break;
					}
				}
			}
		}
	}
	else if (KeyInformationClass == KeyNodeInformation) {
		PKEY_NODE_INFORMATION pni = (PKEY_NODE_INFORMATION)KeyInformation;
		if (pni) {
			if (pni->NameLength != 0 && pni->Name != NULL) {
				PLIST_ENTRY pLink;
				for (pLink = glTaskQueueKey.Flink; pLink != &glTaskQueueKey; pLink = pLink->Flink) {
					PTASK_QUEUE_KEY task = CONTAINING_RECORD(pLink, TASK_QUEUE_KEY, link);
					if (!wcsncmp(pni->Name, task->target, pni->NameLength / sizeof(WCHAR))) {
						DbgPrint("NODE\n");
						break;
					}
				}
			}
		}
	}
	else if (KeyInformationClass == KeyNameInformation) {
		PKEY_NAME_INFORMATION pni = (PKEY_NAME_INFORMATION)KeyInformation;
		UNICODE_STRING unName;
		PWCHAR name = ExAllocatePool(NonPagedPool, pni->NameLength + sizeof(WCHAR));
		RtlZeroMemory(name, pni->NameLength + 2);
		RtlCopyMemory(name, pni->Name, pni->NameLength);
		RtlInitUnicodeString(&unName, name);
		KdPrint(("KeyNameInformation %ws\n", unName.Buffer));
	}


	return retStatus;
}

PWCH AnsiToUnicodeKey(char* str) {

	ANSI_STRING ansiStr;
	UNICODE_STRING uniStr;
	USHORT length;

	RtlInitAnsiString(&ansiStr, str);
	length = (USHORT)RtlAnsiStringToUnicodeSize(&ansiStr);
	uniStr.Buffer = (PWCH)ExAllocatePoolWithTag(PagedPool, length, 'oneN');
	uniStr.MaximumLength = length;
	RtlAnsiStringToUnicodeString(&uniStr, &ansiStr, FALSE);
	
	return uniStr.Buffer;
}

VOID TaskQueueByKey(PCHAR target, PCHAR add) {

	if (target != NULL && add != NULL) {
		PTASK_QUEUE_KEY task = (PTASK_QUEUE_KEY)ExAllocateFromPagedLookasideList(&glPagedTaskQueueKey);

		task->target = AnsiToUnicodeKey(target);
		task->change = AnsiToUnicodeKey(add);

		InsertTailList(&glTaskQueueKey, &task->link);
	}

}

VOID FreeTaskQueueKeyList() {

	while (!IsListEmpty(&glTaskQueueKey)) {
		PLIST_ENTRY pLink = RemoveHeadList(&glTaskQueueKey);
		PTASK_QUEUE_KEY task = CONTAINING_RECORD(pLink, TASK_QUEUE_KEY, link);
		if (task->target) ExFreePool(task->target);
		if (task->change) ExFreePool(task->change);
		ExFreeToPagedLookasideList(&glPagedTaskQueueKey, task);
	}

}


VOID PrintTaskQueueKeyList() {
	PLIST_ENTRY pLink;
	DbgPrint("TASK QUEUE FOR ADD KEY\n");
	for (pLink = glTaskQueueKey.Flink; pLink != &glTaskQueueKey; pLink = pLink->Flink) {
		PTASK_QUEUE_KEY task = CONTAINING_RECORD(pLink, TASK_QUEUE_KEY, link);

		if (task->target) {
			DbgPrint("TARGET:%ws\t", (PCHAR)task->target);
		}
		if (task->change) {
			DbgPrint("CHANGE:%ws\n", (PCHAR)task->change);
		}
	}
}