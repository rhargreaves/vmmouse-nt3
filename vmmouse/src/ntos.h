#ifndef _NTOS_H_
#define _NTOS_H_

typedef long NTSTATUS;
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L)
#define STATUS_UNSUCCESSFUL ((NTSTATUS)0xC0000001L)
#define STATUS_INVALID_PARAMETER ((NTSTATUS)0xC000000DL)
#define STATUS_INSUFFICIENT_RESOURCES ((NTSTATUS)0xC000009AL)
#define STATUS_INVALID_DEVICE_REQUEST ((NTSTATUS)0xC0000010L)

#define IRP_MJ_CREATE 0x00
#define IRP_MJ_CLOSE 0x02
#define IRP_MJ_INTERNAL_DEVICE_CONTROL 0x0f

#define DO_BUFFERED_IO 0x00000004
#define IO_NO_INCREMENT 0

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
typedef struct _IRP IRP, *PIRP;
typedef struct _KDPC KDPC, *PKDPC;

typedef NTSTATUS(__stdcall *PDRIVER_DISPATCH)(PDEVICE_OBJECT DeviceObject, PIRP Irp);
typedef void(__stdcall *PDRIVER_UNLOAD)(PDRIVER_OBJECT DriverObject);
typedef void(__stdcall *PKDEFERRED_ROUTINE)(PKDPC Dpc, void *DeferredContext, void *SystemArgument1,
                                            void *SystemArgument2);

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
    PDRIVER_UNLOAD DriverUnload;
    PDRIVER_DISPATCH MajorFunction[28];
};

typedef struct _KDPC {
    short Type;
    char Number;
    char Importance;
    void *DpcListEntry[2];
    PKDEFERRED_ROUTINE DeferredRoutine;
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

void __stdcall RtlInitUnicodeString(PUNICODE_STRING DestinationString, const wchar_t *SourceString);
NTSTATUS __stdcall RtlWriteRegistryValue(unsigned long RelativeTo, const wchar_t *Path,
                                         const wchar_t *ValueName, unsigned long ValueType,
                                         void *ValueData, unsigned long ValueLength);
NTSTATUS __stdcall RtlDeleteRegistryValue(unsigned long RelativeTo, const wchar_t *Path,
                                          const wchar_t *ValueName);
NTSTATUS __stdcall IoCreateDevice(PDRIVER_OBJECT DriverObject, unsigned long DeviceExtensionSize,
                                  PUNICODE_STRING DeviceName, unsigned long DeviceType,
                                  unsigned long DeviceCharacteristics, unsigned char Exclusive,
                                  PDEVICE_OBJECT *DeviceObject);
void __stdcall IoDeleteDevice(PDEVICE_OBJECT DeviceObject);
void __stdcall IoCompleteRequest(PIRP Irp, char PriorityBoost);
void __stdcall KeInitializeTimer(PKTIMER Timer);
unsigned char __stdcall KeSetTimer(PKTIMER Timer, LARGE_INTEGER DueTime, PKDPC Dpc);
unsigned char __stdcall KeCancelTimer(PKTIMER Timer);
void __stdcall KeInitializeDpc(PKDPC Dpc, PKDEFERRED_ROUTINE DeferredRoutine,
                               void *DeferredContext);

#endif
