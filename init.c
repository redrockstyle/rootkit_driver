#include <ntddk.h>

typedef struct _KSERVICE_TABLE_DESCRIPTOR {
    PULONG_PTR Base;        // массив адресов системных вызовов(сервисов)
    PULONG Count;           // массив счётчиков вызовов сервисов
    ULONG Limit;            // количество вызовов в таблице
    PUCHAR Number;          // массив количества параметров вызовов(в байтах)
} KSERVICE_TABLE_DESCRIPTOR, * PKSERVICE_TABLE_DESCRIPTOR;


typedef NTSTATUS
(*NT_ACCESS_CHECK_AND_AUDIT_ALARM)(
    IN  PUNICODE_STRING SubsystemName,
    IN  PVOID HandleId,
    IN  PUNICODE_STRING ObjectTypeName,
    IN  PUNICODE_STRING ObjectName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping,
    IN  BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
);

#define NUMBER_NT_ACCESS_CHECK_AND_AUDIT_ALARM  0x26 //38

NTSTATUS DriverEntry(IN PDRIVER_OBJECT dob, IN PUNICODE_STRING rgp);
VOID DriverUnload(IN PDRIVER_OBJECT dob);

NTSTATUS HookNtAccessCheckAndAuditAlarm(
    IN  PUNICODE_STRING SubsystemName,
    IN  PVOID HandleId,
    IN  PUNICODE_STRING ObjectTypeName,
    IN  PUNICODE_STRING ObjectName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping,
    IN  BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
);

NT_ACCESS_CHECK_AND_AUDIT_ALARM glRealNtAccessCheckAndAuditAlarm;
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

    // запоминаем адрес реального обработчика вызова NtCreateFile
    glRealNtAccessCheckAndAuditAlarm = (NT_ACCESS_CHECK_AND_AUDIT_ALARM)KeServiceDescriptorTable->Base[NUMBER_NT_ACCESS_CHECK_AND_AUDIT_ALARM];

    reg = ClearWP();
    // подставляем адрес нового обработчика
    KeServiceDescriptorTable->Base[NUMBER_NT_ACCESS_CHECK_AND_AUDIT_ALARM] = (ULONG)HookNtAccessCheckAndAuditAlarm;
    WriteCR0(reg);

    dob->DriverUnload = DriverUnload;


    return STATUS_SUCCESS;
}

VOID DriverUnload(IN PDRIVER_OBJECT dob) {

    ULONG reg;

#if DBG
    DbgPrint("Driver unload");
#endif

    reg = ClearWP();
    KeServiceDescriptorTable->Base[NUMBER_NT_ACCESS_CHECK_AND_AUDIT_ALARM] = (ULONG)glRealNtAccessCheckAndAuditAlarm;
    WriteCR0(reg);

    return;
}



NTSTATUS HookNtAccessCheckAndAuditAlarm(
    IN  PUNICODE_STRING SubsystemName,
    IN  PVOID HandleId,
    IN  PUNICODE_STRING ObjectTypeName,
    IN  PUNICODE_STRING ObjectName,
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN  ACCESS_MASK DesiredAccess,
    IN  PGENERIC_MAPPING GenericMapping,
    IN  BOOLEAN ObjectCreation,
    OUT PACCESS_MASK GrantedAccess,
    OUT PNTSTATUS AccessStatus,
    OUT PBOOLEAN GenerateOnClose
) {

    KPROCESSOR_MODE mode;
    NTSTATUS retStatus;

    mode = ExGetPreviousMode();
    if (mode == KernelMode)
        mode = 'K';
    else
        mode = 'U';

    DbgPrint("HOOOOOOOOOOOOOOOOOOK\n");

    retStatus = glRealNtAccessCheckAndAuditAlarm(
        SubsystemName,
        HandleId,
        ObjectTypeName,
        ObjectName,
        SecurityDescriptor,
        DesiredAccess,
        GenericMapping,
        ObjectCreation,
        GrantedAccess,
        AccessStatus,
        GenerateOnClose
    );

    return retStatus;
}