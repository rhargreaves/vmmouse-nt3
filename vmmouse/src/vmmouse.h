#ifndef _VMMOUSE_H_
#define _VMMOUSE_H_

#define VMWARE_MAGIC 0x564D5868
#define VMWARE_PORT 0x5658

#define VMMOUSE_READ_ID 0x45414552
#define VMMOUSE_DISABLE 0x000000F5
#define VMMOUSE_REQUEST_RELATIVE 0x4C455252
#define VMMOUSE_REQUEST_ABSOLUTE 0x53424152

#define VMMOUSE_CMD_GETVERSION 10
#define VMMOUSE_CMD_DATA 39
#define VMMOUSE_CMD_STATUS 40
#define VMMOUSE_CMD_COMMAND 41

#define VMMOUSE_VERSION 0x3442554A

#define VMMOUSE_LEFT_BUTTON 0x20
#define VMMOUSE_RIGHT_BUTTON 0x10
#define VMMOUSE_MIDDLE_BUTTON 0x08

typedef struct _VMMOUSE_REGS {
    unsigned long eax;
    unsigned long ebx;
    unsigned long ecx;
    unsigned long edx;
    unsigned long esi;
    unsigned long edi;
} VMMOUSE_REGS, *PVMMOUSE_REGS;

static inline void vmmouse_cmd(VMMOUSE_REGS *regs) {
    __asm__ __volatile__("inl %%dx, %%eax"
                         : "+a"(regs->eax), "+b"(regs->ebx), "+c"(regs->ecx), "+d"(regs->edx),
                           "+S"(regs->esi), "+D"(regs->edi)
                         :
                         : "memory");
}

#endif
