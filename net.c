#include "net.h"

NTSTATUS InitHookNet(PWCHAR DriverName) {

	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uniDriverName;

	RtlInitUnicodeString(&uniDriverName, DriverName);

	status = ObReferenceObjectByName(&uniDriverName,
		OBJ_CASE_INSENSITIVE,
		NULL,
		GENERIC_READ,
		*IoDriverObjectType,
		KernelMode,
		NULL,
		&glNsiDriverObject);
	if (!NT_SUCCESS(status)) {
		DbgPrint("Error ObReferenceObjectByName: %08X\n", status);
		return status;
	}

	return status;
}


NTSTATUS HookTcpDeviceControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp) {

}