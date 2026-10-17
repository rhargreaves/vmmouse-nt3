# VMMouse for NT 3.x

[![Build and Release](https://github.com/rhargreaves/vmmouse-nt3/actions/workflows/release.yml/badge.svg)](https://github.com/rhargreaves/vmmouse-nt3/actions/workflows/release.yml)

QEMU/VMware mouse integration driver for Windows NT 3.x

## Build

Using Docker:

```bash
make docker-build
```

## Install

Mount `build/vmmouse.iso`:

1. Run `D:\INSTALL.BAT` (or `D:\INSTDRV.EXE`)
2. Reboot

To uninstall:

1. Run `D:\UNINSTALL.BAT` (or `D:\INSTDRV.EXE /u`).
2. Reboot

## Testing

Only confirmed working on NT 3.51 SP5 (Intel)
