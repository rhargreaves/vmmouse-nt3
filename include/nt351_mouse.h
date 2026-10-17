#ifndef _NT351_MOUSE_H_
#define _NT351_MOUSE_H_

#define MOUSE_MOVE_RELATIVE  0x0000
#define MOUSE_MOVE_ABSOLUTE  0x0001

#define MOUSE_LEFT_BUTTON_DOWN   0x0001
#define MOUSE_LEFT_BUTTON_UP     0x0002
#define MOUSE_RIGHT_BUTTON_DOWN  0x0004
#define MOUSE_RIGHT_BUTTON_UP    0x0008
#define MOUSE_MIDDLE_BUTTON_DOWN 0x0010
#define MOUSE_MIDDLE_BUTTON_UP   0x0020

#define FILE_DEVICE_MOUSE    0x0000000F
#define METHOD_NEITHER       3
#define FILE_ANY_ACCESS      0

#define CTL_CODE(DeviceType, Function, Method, Access) ( \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
)

#define IOCTL_INTERNAL_MOUSE_CONNECT \
    CTL_CODE(FILE_DEVICE_MOUSE, 0x0080, METHOD_NEITHER, FILE_ANY_ACCESS)

#pragma pack(push, 4)
typedef struct _MOUSE_INPUT_DATA {
    unsigned short UnitId;
    unsigned short Flags;
    union {
        unsigned long Buttons;
        struct {
            unsigned short ButtonFlags;
            unsigned short ButtonData;
        };
    };
    unsigned long RawButtons;
    long LastX;
    long LastY;
    unsigned long ExtraInformation;
} MOUSE_INPUT_DATA, *PMOUSE_INPUT_DATA;

typedef void (__stdcall *PMOUSE_SERVICE_CALLBACK)(
    void *DeviceObject,
    PMOUSE_INPUT_DATA InputDataStart,
    PMOUSE_INPUT_DATA InputDataEnd,
    unsigned long *InputDataConsumed
);

typedef struct _CONNECT_DATA {
    void *ClassDeviceObject;
    PMOUSE_SERVICE_CALLBACK ClassService;
} CONNECT_DATA, *PCONNECT_DATA;
#pragma pack(pop)

#endif
