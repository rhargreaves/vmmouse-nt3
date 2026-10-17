#include "vmmouse.h"
#include "nt351_mouse.h"

typedef long NTSTATUS;
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL              ((NTSTATUS)0xC0000001L)
#define STATUS_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC000009AL)
#define STATUS_INVALID_DEVICE_REQUEST    ((NTSTATUS)0xC0000010L)

#define IRP_MJ_CREATE                    0x00
#define IRP_MJ_CLOSE                     0x02
#define IRP_MJ_INTERNAL_DEVICE_CONTROL   0x0f

#define DO_BUFFERED_IO                   0x00000004
#define IO_NO_INCREMENT                  0

typedef unsigned short wchar_t;

typedef union _LARGE_INTEGER {
    struct {
        unsigned long LowPart;
        long HighPart;
    };
    long long QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef struct _UNICODE_STRING {
    unsigned short Length;
    unsigned short MaximumLength;
    wchar_t *Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        void *Pointer;
    };
    unsigned long Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _DEVICE_OBJECT DEVICE_OBJECT, *PDEVICE_OBJECT;
typedef struct _DRIVER_OBJECT DRIVER_OBJECT, *PDRIVER_OBJECT;

typedef struct _IO_STACK_LOCATION {
    unsigned char MajorFunction;
    unsigned char MinorFunction;
    unsigned char Flags;
    unsigned char Control;
    union {
        struct {
            unsigned long OutputBufferLength;
            unsigned long InputBufferLength;
            unsigned long IoControlCode;
            void *Type3InputBuffer;
        } DeviceIoControl;
        struct {
            void *Argument1;
            void *Argument2;
            void *Argument3;
            void *Argument4;
        } Others;
    } Parameters;
    PDEVICE_OBJECT DeviceObject;
    void *FileObject;
    void *CompletionRoutine;
    void *Context;
} IO_STACK_LOCATION, *PIO_STACK_LOCATION;

typedef struct _IRP {
    short Type;
    unsigned short Size;
    void *MdlAddress;
    unsigned long Flags;
    union {
        struct _IRP *MasterIrp;
        long IrpCount;
        void *SystemBuffer;
    } AssociatedIrp;
    struct {
        void *Flink;
        void *Blink;
    } ThreadListEntry;
    IO_STATUS_BLOCK IoStatus;
    char RequestorMode;
    unsigned char PendingReturned;
    char StackCount;
    char CurrentLocation;
    unsigned char Cancel;
    unsigned char CancelIrql;
    char ApcEnvironment;
    unsigned char AllocationFlags;
    void *UserIosb;
    void *UserEvent;
    union {
        struct {
            void *UserApcRoutine;
            void *UserApcContext;
        } AsynchronousParameters;
        long long AllocationSize;
    } Overlay;
    void *CancelRoutine;
    void *UserBuffer;
    union {
        struct {
            union {
                void *DeviceQueueEntry[4];
                void *DriverContext[4];
            };
            void *Thread;
            char *AuxiliaryBuffer;
            struct {
                void *ListEntry[2];
                union {
                    struct _IO_STACK_LOCATION *CurrentStackLocation;
                    unsigned long PacketType;
                };
            };
            void *OriginalFileObject;
        } Overlay;
        char Apc[32];
        void *CompletionKey;
    } Tail;
} IRP, *PIRP;

#define IoGetCurrentIrpStackLocation(Irp) ((Irp)->Tail.Overlay.CurrentStackLocation)

struct _DEVICE_OBJECT {
    short Type;
    unsigned short Size;
    int ReferenceCount;
    struct _DRIVER_OBJECT *DriverObject;
    struct _DEVICE_OBJECT *NextDevice;
    struct _DEVICE_OBJECT *AttachedDevice;
    struct _IRP *CurrentIrp;
    void *Timer;
    unsigned long Flags;
    unsigned long Characteristics;
    void *Vpb;
    void *DeviceExtension;
    unsigned long DeviceType;
    char StackSize;
    union {
        struct _LIST_ENTRY *ListHead;
        void *WdiContext;
    } Queue;
    unsigned long AlignmentRequirement;
    void *DeviceQueue;
    void *Dpc;
    unsigned long ActiveThreadCount;
    void *SecurityDescriptor;
    void *DeviceLock;
    unsigned short SectorSize;
    unsigned short Spare1;
    void *Reserved;
};

struct _DRIVER_OBJECT {
    short Type;
    short Size;
    PDEVICE_OBJECT DeviceObject;
    unsigned long Flags;
    void *DriverStart;
    unsigned long DriverSize;
    void *DriverSection;
    void *DriverExtension;
    UNICODE_STRING DriverName;
    PUNICODE_STRING HardwareDatabase;
    void *FastIoDispatch;
    void *DriverInit;
    void *DriverStartIo;
    void *DriverUnload;
    void *MajorFunction[28];
};

typedef struct _KDPC {
    short Type;
    char Number;
    char Importance;
    void *DpcListEntry[2];
    void *DeferredRoutine;
    void *DeferredContext;
    void *SystemArgument1;
    void *SystemArgument2;
    void *DpcData;
} KDPC, *PKDPC;

typedef struct _KTIMER {
    void *Header[4];
    LARGE_INTEGER DueTime;
    void *TimerListEntry[2];
    PKDPC Dpc;
    long Period;
} KTIMER, *PKTIMER;

typedef struct _DEVICE_EXTENSION {
    PDEVICE_OBJECT DeviceObject;
    CONNECT_DATA ConnectData;
    KTIMER PollTimer;
    KDPC PollDpc;
    unsigned short PreviousButtons;
    int Enabled;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

extern void __stdcall RtlInitUnicodeString(PUNICODE_STRING DestinationString,
                                          const wchar_t *SourceString);
extern NTSTATUS __stdcall IoCreateDevice(PDRIVER_OBJECT DriverObject,
                                        unsigned long DeviceExtensionSize,
                                        PUNICODE_STRING DeviceName,
                                        unsigned long DeviceType,
                                        unsigned long DeviceCharacteristics,
                                        unsigned char Exclusive,
                                        PDEVICE_OBJECT *DeviceObject);
extern void __stdcall IoDeleteDevice(PDEVICE_OBJECT DeviceObject);
extern void __stdcall IoCompleteRequest(PIRP Irp, char PriorityBoost);
extern void __stdcall KeInitializeTimer(PKTIMER Timer);
extern unsigned char __stdcall KeSetTimer(PKTIMER Timer, LARGE_INTEGER DueTime, PKDPC Dpc);
extern unsigned char __stdcall KeCancelTimer(PKTIMER Timer);
extern void __stdcall KeInitializeDpc(PKDPC Dpc, void *DeferredRoutine, void *DeferredContext);

static int vmmouse_probe(void) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = 0;
    r.ecx = VMMOUSE_CMD_GETVERSION;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
    if (r.eax == 0xFFFFFFFF || r.ebx != VMWARE_MAGIC)
        return 0;

    r.eax = VMWARE_MAGIC;
    r.ebx = VMMOUSE_READ_ID;
    r.ecx = VMMOUSE_CMD_COMMAND;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);

    r.eax = VMWARE_MAGIC;
    r.ebx = 0;
    r.ecx = VMMOUSE_CMD_STATUS;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
    if ((r.eax & 0xFFFF) < 1)
        return 0;

    r.eax = VMWARE_MAGIC;
    r.ebx = 1;
    r.ecx = VMMOUSE_CMD_DATA;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);

    return (r.eax == VMMOUSE_VERSION);
}

static void vmmouse_enable_absolute(void) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = VMMOUSE_REQUEST_ABSOLUTE;
    r.ecx = VMMOUSE_CMD_COMMAND;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
}

static void vmmouse_disable(void) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = VMMOUSE_DISABLE;
    r.ecx = VMMOUSE_CMD_COMMAND;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
}

static void __stdcall vmmouse_poll_dpc(PKDPC Dpc, void *DeferredContext,
                                      void *SysArg1, void *SysArg2) {
    PDEVICE_EXTENSION dx = (PDEVICE_EXTENSION)DeferredContext;
    VMMOUSE_REGS r;
    MOUSE_INPUT_DATA mid;
    unsigned long consumed;
    unsigned short btn_flags;
    unsigned short cur_btns;
    LARGE_INTEGER due;

    if (!dx->Enabled || !dx->ConnectData.ClassService)
        return;

    while (1) {
        r.eax = VMWARE_MAGIC;
        r.ebx = 0;
        r.ecx = VMMOUSE_CMD_STATUS;
        r.edx = VMWARE_PORT;
        r.esi = 0;
        r.edi = 0;
        vmmouse_cmd(&r);

        if ((r.eax & 0xFFFF) < 4)
            break;

        r.eax = VMWARE_MAGIC;
        r.ebx = 4;
        r.ecx = VMMOUSE_CMD_DATA;
        r.edx = VMWARE_PORT;
        r.esi = 0;
        r.edi = 0;
        vmmouse_cmd(&r);

        cur_btns = 0;
        if (r.eax & VMMOUSE_LEFT_BUTTON)   cur_btns |= 0x01;
        if (r.eax & VMMOUSE_RIGHT_BUTTON)  cur_btns |= 0x02;
        if (r.eax & VMMOUSE_MIDDLE_BUTTON) cur_btns |= 0x04;

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
        mid.LastX = (long)r.ebx;
        mid.LastY = (long)r.ecx;
        mid.ExtraInformation = 0;

        dx->ConnectData.ClassService(
            dx->ConnectData.ClassDeviceObject,
            &mid,
            &mid + 1,
            &consumed
        );
    }

    if (dx->Enabled) {
        due.QuadPart = -100000LL;
        KeSetTimer(&dx->PollTimer, due, &dx->PollDpc);
    }
}

static NTSTATUS __stdcall vmmouse_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return STATUS_SUCCESS;
}

static NTSTATUS __stdcall vmmouse_internal_ioctl(PDEVICE_OBJECT DeviceObject, PIRP Irp) {
    PDEVICE_EXTENSION dx = (PDEVICE_EXTENSION)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_INVALID_DEVICE_REQUEST;
    LARGE_INTEGER due;

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    case IOCTL_INTERNAL_MOUSE_CONNECT:
        if (irpSp->Parameters.DeviceIoControl.InputBufferLength >= sizeof(CONNECT_DATA)) {
            PCONNECT_DATA cd = (PCONNECT_DATA)Irp->AssociatedIrp.SystemBuffer;
            dx->ConnectData = *cd;
            dx->Enabled = 1;
            vmmouse_enable_absolute();
            due.QuadPart = -100000LL;
            KeSetTimer(&dx->PollTimer, due, &dx->PollDpc);
            status = STATUS_SUCCESS;
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

static void __stdcall vmmouse_unload(PDRIVER_OBJECT DriverObject) {
    PDEVICE_OBJECT dev_obj = DriverObject->DeviceObject;
    if (dev_obj) {
        PDEVICE_EXTENSION dx = (PDEVICE_EXTENSION)dev_obj->DeviceExtension;
        dx->Enabled = 0;
        KeCancelTimer(&dx->PollTimer);
        vmmouse_disable();
        IoDeleteDevice(dev_obj);
    }
}

static const wchar_t *g_port_names[] = {
    L"\\Device\\PointerPort0",
    L"\\Device\\PointerPort1",
    L"\\Device\\PointerPort2",
    L"\\Device\\PointerPort3"
};

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

    KeInitializeTimer(&dx->PollTimer);
    KeInitializeDpc(&dx->PollDpc, vmmouse_poll_dpc, dx);

    DriverObject->MajorFunction[IRP_MJ_CREATE] = vmmouse_create_close;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = vmmouse_create_close;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = vmmouse_internal_ioctl;
    DriverObject->DriverUnload = vmmouse_unload;

    return STATUS_SUCCESS;
}
