#include "device.h"
#include "vmmouse.h"

static const wchar_t g_pointer_port[] = L"PointerPort";

void __stdcall vmmouse_poll_dpc(PKDPC Dpc, void *DeferredContext, void *SysArg1, void *SysArg2) {
    PDEVICE_EXTENSION dx = (PDEVICE_EXTENSION)DeferredContext;
    MOUSE_INPUT_DATA mid;
    unsigned long consumed;
    unsigned short cur_btns;
    unsigned short btn_flags;
    long x;
    long y;
    LARGE_INTEGER due;

    (void)Dpc;
    (void)SysArg1;
    (void)SysArg2;

    if (!dx->Enabled || !dx->ConnectData.ClassService)
        return;

    while (vmmouse_read_data(&cur_btns, &x, &y)) {
        btn_flags = 0;
        if ((cur_btns & 0x01) && !(dx->PreviousButtons & 0x01))
            btn_flags |= MOUSE_LEFT_BUTTON_DOWN;
        if (!(cur_btns & 0x01) && (dx->PreviousButtons & 0x01))
            btn_flags |= MOUSE_LEFT_BUTTON_UP;
        if ((cur_btns & 0x02) && !(dx->PreviousButtons & 0x02))
            btn_flags |= MOUSE_RIGHT_BUTTON_DOWN;
        if (!(cur_btns & 0x02) && (dx->PreviousButtons & 0x02))
            btn_flags |= MOUSE_RIGHT_BUTTON_UP;
        if ((cur_btns & 0x04) && !(dx->PreviousButtons & 0x04))
            btn_flags |= MOUSE_MIDDLE_BUTTON_DOWN;
        if (!(cur_btns & 0x04) && (dx->PreviousButtons & 0x04))
            btn_flags |= MOUSE_MIDDLE_BUTTON_UP;

        dx->PreviousButtons = cur_btns;

        mid.UnitId = 0;
        mid.Flags = MOUSE_MOVE_ABSOLUTE;
        mid.ButtonFlags = btn_flags;
        mid.ButtonData = 0;
        mid.RawButtons = cur_btns;
        mid.LastX = x;
        mid.LastY = y;
        mid.ExtraInformation = 0;

        dx->ConnectData.ClassService(dx->ConnectData.ClassDeviceObject, &mid, &mid + 1, &consumed);
    }

    if (dx->Enabled) {
        due.QuadPart = -100000LL;
        KeSetTimer(&dx->PollTimer, due, &dx->PollDpc);
    }
}

NTSTATUS __stdcall vmmouse_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    (void)DeviceObject;
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

NTSTATUS __stdcall vmmouse_internal_ioctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PDEVICE_EXTENSION dx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    LARGE_INTEGER due;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_INTERNAL_MOUSE_CONNECT:
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(CONNECT_DATA)) {
            PCONNECT_DATA cd = (PCONNECT_DATA)irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            if (!cd)
                cd = (PCONNECT_DATA)Irp->AssociatedIrp.SystemBuffer;
            if (cd && cd->ClassService) {
                dx->ConnectData = *cd;
                dx->Enabled = 1;
                vmmouse_enable_absolute();
                due.QuadPart = -100000LL;
                KeSetTimer(&dx->PollTimer, due, &dx->PollDpc);
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        break;

    case IOCTL_INTERNAL_MOUSE_ENABLE:
        dx->Enabled = 1;
        due.QuadPart = -100000LL;
        KeSetTimer(&dx->PollTimer, due, &dx->PollDpc);
        status = STATUS_SUCCESS;
        break;

    case IOCTL_INTERNAL_MOUSE_DISCONNECT:
    case IOCTL_INTERNAL_MOUSE_DISABLE:
        dx->Enabled = 0;
        KeCancelTimer(&dx->PollTimer);
        vmmouse_disable();
        status = STATUS_SUCCESS;
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

void __stdcall vmmouse_unload(PDRIVER_OBJECT DriverObject) {
    PDEVICE_OBJECT dev_obj = DriverObject->DeviceObject;
    if (dev_obj) {
        PDEVICE_EXTENSION dx = (PDEVICE_EXTENSION)dev_obj->DeviceExtension;
        dx->Enabled = 0;
        KeCancelTimer(&dx->PollTimer);
        vmmouse_disable();
        RtlDeleteRegistryValue(4, g_pointer_port, dx->DeviceName.Buffer);
        IoDeleteDevice(dev_obj);
    }
}
