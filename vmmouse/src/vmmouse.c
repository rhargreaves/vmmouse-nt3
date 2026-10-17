#include "vmmouse.h"

int vmmouse_probe(void) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = 0;
    r.ecx = VMMOUSE_CMD_GETVERSION;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
    /* On hardware without backdoor support, reading the unassigned I/O port
     * returns floating bus state 0xFFFFFFFF, safely failing the probe. */
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

void vmmouse_enable_absolute(void) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = VMMOUSE_REQUEST_ABSOLUTE;
    r.ecx = VMMOUSE_CMD_COMMAND;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
}

void vmmouse_disable(void) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = VMMOUSE_DISABLE;
    r.ecx = VMMOUSE_CMD_COMMAND;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);
}

int vmmouse_read_data(unsigned short *buttons, long *x, long *y) {
    VMMOUSE_REGS r = {0};

    r.eax = VMWARE_MAGIC;
    r.ebx = 0;
    r.ecx = VMMOUSE_CMD_STATUS;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);

    if ((r.eax & 0xFFFF) < VMMOUSE_PACKET_WORDS)
        return 0;

    r.eax = VMWARE_MAGIC;
    r.ebx = VMMOUSE_PACKET_WORDS;
    r.ecx = VMMOUSE_CMD_DATA;
    r.edx = VMWARE_PORT;
    vmmouse_cmd(&r);

    *buttons = 0;
    if (r.eax & VMMOUSE_LEFT_BUTTON)
        *buttons |= VMMOUSE_BTN_LEFT;
    if (r.eax & VMMOUSE_RIGHT_BUTTON)
        *buttons |= VMMOUSE_BTN_RIGHT;
    if (r.eax & VMMOUSE_MIDDLE_BUTTON)
        *buttons |= VMMOUSE_BTN_MIDDLE;

    *x = (long)r.ebx;
    *y = (long)r.ecx;

    return 1;
}
