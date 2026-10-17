# vmmouse-nt3

Seamless absolute mouse integration driver for **Windows NT 3.51** running in QEMU / VMware.

## Overview

Windows NT 3.51 only natively supports relative PS/2 or serial mice. When virtualized
in QEMU, this requires capturing / grabbing the cursor (`Ctrl+Alt`).

This project implements an NT kernel-mode mouse port driver (`vmmouse.sys`) that
communicates with QEMU / VMware's paravirtualized backdoor I/O port (`0x5658`),
reading normalized absolute cursor coordinates `[0, 65535]` and dispatching
`MOUSE_MOVE_ABSOLUTE` packets to `mouser.sys` for seamless cursor navigation.

## Architecture

1. **Hardware Protocol**:
   - Queries the VMware backdoor port `0x5658` with magic signature `0x564D5868`.
   - Switches the hypervisor pointer interface into absolute mode (`VMMOUSE_REQUEST_ABSOLUTE`).
   - Polls or intercepts pointer queue events (`VMMOUSE_CMD_DATA`), reading absolute
     coordinates and button transition states.

2. **NT 3.51 Driver Model**:
   - Creates `\Device\PointerPort0`.
   - Handles `IOCTL_INTERNAL_MOUSE_CONNECT` from `mouser.sys` (Mouse Class Driver).
   - Injects normalized `MOUSE_INPUT_DATA` packets with `Flags = MOUSE_MOVE_ABSOLUTE`
     directly into the class service callback.

## Building

Requires LLVM/Clang with `lld-link` and `llvm-dlltool`:

```bash
# macOS
brew install llvm
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# Build driver
make
```

Output binary: `build/vmmouse.sys`
