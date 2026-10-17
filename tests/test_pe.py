import os
from pe_util import (
    IMAGE_FILE_MACHINE_I386,
    SUBSYSTEM_NATIVE,
    SUBSYSTEM_WINDOWS_CUI,
    PeInfo,
    parse_pe,
)


def test_vmmouse_sys_header(build_dir, expected_image_version):
    path = os.path.join(build_dir, "vmmouse.sys")
    pe: PeInfo = parse_pe(path)

    assert pe.machine == IMAGE_FILE_MACHINE_I386, "Must be i386 machine type"
    assert pe.image_base == 0x10000, "Kernel driver base must be 0x10000"
    assert pe.subsystem == SUBSYSTEM_NATIVE, "Must use native subsystem"
    assert pe.subsystem_version == (3, 10), "Subsystem version must be 3.10 for NT 3.x"
    assert pe.image_version == expected_image_version, "Image version must match build"
    assert pe.dll_characteristics == 0, "No ASLR/NX/SafeSEH on NT 3.51"
    assert pe.entry_point > 0, "DriverEntry point must be non-zero"


def test_vmmouse_sys_metadata(build_dir, build_version):
    path = os.path.join(build_dir, "vmmouse.sys")
    pe: PeInfo = parse_pe(path)

    for needle in [
        "Robert Hargreaves",
        "vmmouse.sys",
        "VMware/QEMU Mouse Integration Driver",
        "VMMouse for NT 3.x",
        build_version,
    ]:
        assert pe.has_resource_string(needle), f"Missing UTF-16LE '{needle}' in resource section"


def test_instdrv_exe_header(build_dir, expected_image_version):
    path = os.path.join(build_dir, "instdrv.exe")
    pe: PeInfo = parse_pe(path)

    assert pe.machine == IMAGE_FILE_MACHINE_I386, "Must be i386 machine type"
    assert pe.image_base == 0x400000, "Console EXE base must be 0x400000"
    assert pe.subsystem == SUBSYSTEM_WINDOWS_CUI, "Must use console subsystem"
    assert pe.subsystem_version == (3, 10), "Subsystem version must be 3.10 for NT 3.x"
    assert pe.image_version == expected_image_version, "Image version must match build"
    assert pe.dll_characteristics == 0, "No ASLR/NX/SafeSEH on NT 3.51"
    assert pe.entry_point > 0, "mainCRTStartup entry point must be non-zero"


def test_instdrv_exe_metadata(build_dir, build_version):
    path = os.path.join(build_dir, "instdrv.exe")
    pe: PeInfo = parse_pe(path)

    for needle in [
        "Robert Hargreaves",
        "instdrv.exe",
        "VMMouse Driver Installer",
        "VMMouse for NT 3.x",
        build_version,
    ]:
        assert pe.has_resource_string(needle), f"Missing UTF-16LE '{needle}' in resource section"
