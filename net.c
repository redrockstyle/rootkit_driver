#include "net.h"


NTSTATUS InstallTCPDriverHook(WCHAR* wcTcpDeviceNameBuffer) {

    NTSTATUS status;
    UNICODE_STRING TcpDeviceName;

    RtlInitUnicodeString(&TcpDeviceName, wcTcpDeviceNameBuffer);
    status = IoGetDeviceObjectPointer(&TcpDeviceName, FILE_READ_DATA, &pTcpFile, &pTcpDevice);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    pTcpDriver = pTcpDevice->DriverObject;

    glRealIrpMjDeviceControl = pTcpDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL];
    pTcpDriver->MajorFunction[IRP_MJ_DEVICE_CONTROL] = HookTcpDeviceControl;

    return STATUS_SUCCESS;
}

NTSTATUS HookTcpDeviceControl(IN PDEVICE_OBJECT pDeviceObject, IN PIRP pIrp) {

    PIO_STACK_LOCATION pIrpStack;
    ULONG ioTransferType;
    TDIObjectID* inputBuffer;
    ULONG context;
    NTSTATUS status;

    pIrpStack = IoGetCurrentIrpStackLocation(pIrp);

    switch (pIrpStack->MajorFunction) {
    case IRP_MJ_DEVICE_CONTROL:

        //DbgPrint("NET HOOOOK\n");
        break;
    default:
        break;
    }



    return glRealIrpMjDeviceControl(pDeviceObject, pIrp);
}