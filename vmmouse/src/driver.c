#include "device.h"
#include "vmmouse.h"

static const wchar_t g_pointer_port[] = L"PointerPort";

static const wchar_t *g_port_names[] = {L"\\Device\\PointerPort0", L"\\Device\\PointerPort1",
                                        L"\\Device\\PointerPort2", L"\\Device\\PointerPort3"};

NTSTATUS __stdcall DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    PDEVICE_OBJECT dev_obj = 0;
    PDEVICE_EXTENSION dx;
    UNICODE_STRING dev_name;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    int i;

    if (!vmmouse_probe())
        return STATUS_UNSUCCESSFUL;

    for (i = 0; i < 4; i++) {
        RtlInitUnicodeString(&dev_name, g_port_names[i]);
        status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION), &dev_name,
                                FILE_DEVICE_MOUSE, 0, 0, &dev_obj);
        if (status == STATUS_SUCCESS)
            break;
    }

    if (status != STATUS_SUCCESS)
        return status;

    dev_obj->Flags |= DO_BUFFERED_IO;
    dx = (PDEVICE_EXTENSION)dev_obj->DeviceExtension;
    dx->DeviceObject = dev_obj;
    dx->Enabled = 0;
    dx->PreviousButtons = 0;
    dx->DeviceName = dev_name;

    RtlWriteRegistryValue(4, g_pointer_port, dev_name.Buffer, 1, RegistryPath->Buffer,
                          RegistryPath->Length + sizeof(wchar_t));

    KeInitializeTimer(&dx->PollTimer);
    KeInitializeDpc(&dx->PollDpc, vmmouse_poll_dpc, dx);

    DriverObject->MajorFunction[IRP_MJ_CREATE] = vmmouse_create_close;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = vmmouse_create_close;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = vmmouse_internal_ioctl;
    DriverObject->DriverUnload = vmmouse_unload;

    return STATUS_SUCCESS;
}
