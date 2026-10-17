#ifndef _VMMOUSE_H_
#define _VMMOUSE_H_

#define VMWARE_MAGIC             0x564D5868
#define VMWARE_PORT              0x5658

#define VMMOUSE_READ_ID          0x45414552
#define VMMOUSE_DISABLE          0x000000F5
#define VMMOUSE_REQUEST_RELATIVE 0x4C455252
#define VMMOUSE_REQUEST_ABSOLUTE 0x53424152

#define VMMOUSE_CMD_GETVERSION   10
#define VMMOUSE_CMD_DATA         39
#define VMMOUSE_CMD_STATUS       40
#define VMMOUSE_CMD_COMMAND      41

#define VMMOUSE_VERSION          0x3442554A

#define VMMOUSE_LEFT_BUTTON      0x20
#define VMMOUSE_RIGHT_BUTTON     0x10
#define VMMOUSE_MIDDLE_BUTTON    0x08

typedef struct _VMMOUSE_REGS {
    unsigned long eax;
    unsigned long ebx;
    unsigned long ecx;
    unsigned long edx;
    unsigned long esi;
    unsigned long edi;
} VMMOUSE_REGS, *PVMMOUSE_REGS;

static inline void vmmouse_cmd(VMMOUSE_REGS *regs) {
    __asm__ __volatile__(
        "pushl %%ebp\n\t"
        "pushl %%edi\n\t"
        "pushl %%esi\n\t"
        "movl 16(%0), %%esi\n\t"
        "movl 20(%0), %%edi\n\t"
        "movl 0(%0), %%eax\n\t"
        "movl 4(%0), %%ebx\n\t"
        "movl 8(%0), %%ecx\n\t"
        "movl 12(%0), %%edx\n\t"
        "inl %%dx, %%eax\n\t"
        "movl %%eax, 0(%0)\n\t"
        "movl %%ebx, 4(%0)\n\t"
        "movl %%ecx, 8(%0)\n\t"
        "movl %%edx, 12(%0)\n\t"
        "movl %%esi, 16(%0)\n\t"
        "movl %%edi, 20(%0)\n\t"
        "popl %%esi\n\t"
        "popl %%edi\n\t"
        "popl %%ebp\n\t"
        :
        : "a"(regs)
        : "ebx", "ecx", "edx", "memory"
    );
}

#endif
