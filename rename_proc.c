#include "rename_proc.h"

NTSTATUS HookNtQuerySystemInformation(
	IN				SYSTEM_INFORMATION_CLASS SystemInformationClass,
	IN OUT			PVOID                    SystemInformation,
	IN				ULONG                    SystemInformationLength,
	OUT OPTIONAL	PULONG                   ReturnLength
) {
	NTSTATUS retStatus;

	//DbgPrint("a\n");

	retStatus = glRealNtQuerySystemInformation(
		SystemInformationClass,
		SystemInformation,
		SystemInformationLength,
		ReturnLength
	);

	if (SystemInformationClass == SystemProcessInformation && NT_SUCCESS(retStatus)) {

		__try {
			DbgPrint("PROCESS ID %d NAME %wZ\n", ((SYSTEM_PROCESS*)SystemInformation)->ProcessId, &((SYSTEM_PROCESS*)SystemInformation)->ProcessName);
			//FILTER_PROCESS(SYSTEM_PROCESS, SystemInformation, SystemInformationLength, &ReturnLength);
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {

		}

	}
	

	return retStatus;
}

VOID ChangeProcessName() {


}


ULONG StrLenght(PCHAR str) {
	ULONG i = 0;
	while (str[i] != '\0') { i++; };
	return i;
}

VOID TaskQueueProcess(ULONG pid, PCHAR name) {
	
	PTASK_QUEUE task = (PTASK_QUEUE)ExAllocateFromPagedLookasideList(&glPagedTaskQueue);

	if (pid != 0) {
		task->pid = pid;
		task->name = NULL;
	}
	else if (name != NULL) {
		ULONG len = StrLenght(name);
		task->name = (PCHAR)ExAllocatePoolWithTag(PagedPool, len, 'oneN');
		RtlCopyMemory(task->name, name, len);
		task->pid = 0;
	}
	else {
		ExFreeToPagedLookasideList(&glPagedTaskQueue, task);
		return;
	}

	task->done = 0;
	InsertTailList(&glTaskQueue, &task->link);
}

VOID FreeListQueue() {

	while (!IsListEmpty(&glTaskQueue)) {
		PLIST_ENTRY pLink = RemoveHeadList(&glTaskQueue);
		PTASK_QUEUE task = CONTAINING_RECORD(pLink, TASK_QUEUE, link);
		if (task->name) ExFreePool(&task->name);
		ExFreeToPagedLookasideList(&glPagedTaskQueue, task);
	}

}