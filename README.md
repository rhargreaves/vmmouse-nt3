# VMMouse for NT 3.x

QEMU/VMware mouse integration driver for Windows NT 3.x

## Build

Using Docker:

```bash
make docker-build
```

## Installation

Mount `build/vmmouse.iso` on your VM:

1. Run `D:\INSTALL.BAT` (or `D:\INSTDRV.EXE`)
2. Reboot

To uninstall:
Run `D:\UNINSTALL.BAT` (or `D:\INSTDRV.EXE /u`).

## Testing

Only confirmed working on NT 3.51 SP5 (Intel)
