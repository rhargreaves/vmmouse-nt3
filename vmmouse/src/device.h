#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "ntddmou.h"
#include "ntos.h"

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
    CONNECT_DATA ConnectData;
    KTIMER PollTimer;
    KDPC PollDpc;
    unsigned short PreviousButtons;
    int Enabled;
    UNICODE_STRING DeviceName;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

void __stdcall device_poll_dpc(PKDPC Dpc, void *DeferredContext, void *SysArg1, void *SysArg2);
NTSTATUS __stdcall device_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp);
NTSTATUS __stdcall device_internal_ioctl(PDEVICE_OBJECT DeviceObject, PIRP Irp);
void __stdcall device_unload(PDRIVER_OBJECT DriverObject);

#endif
