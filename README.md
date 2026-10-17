# vmmouse-nt3

Mouse integration driver for Windows NT 3.51 (x86) running in QEMU / VMware.

## Build

Using Docker:

```bash
make docker-build
```

Outputs in `build/`:

- `vmmouse.sys`: The driver
- `instdrv.exe`: Installer / uninstaller
- `vmmouse.iso`: CD-ROM image

## Installation

Mount `build/vmmouse.iso` on your VM:

1. Run `D:\INSTALL.BAT` (or `D:\INSTDRV.EXE`)
2. Reboot

To uninstall:
Run `D:\UNINSTALL.BAT` (or `D:\INSTDRV.EXE /u`).

