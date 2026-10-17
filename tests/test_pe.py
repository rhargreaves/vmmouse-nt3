import os
import pefile


def get_version_info(pe: pefile.PE) -> dict[str, str]:
    for fileinfo in getattr(pe, "FileInfo", []):
        for entry in fileinfo:
            if getattr(entry, "name", None) == "StringFileInfo":
                for st in entry.StringTable:
                    return {k.decode(): v.decode() for k, v in st.entries.items()}
    return {}


def assert_common_pe_header(pe: pefile.PE, expected_image_version):
    assert pe.FILE_HEADER.Machine == pefile.MACHINE_TYPE["IMAGE_FILE_MACHINE_I386"]
    subsystem_ver = (
        pe.OPTIONAL_HEADER.MajorSubsystemVersion,
        pe.OPTIONAL_HEADER.MinorSubsystemVersion,
    )
    assert subsystem_ver == (3, 10), "Subsystem version must be 3.10 for NT 3.x"
    img_ver = (pe.OPTIONAL_HEADER.MajorImageVersion, pe.OPTIONAL_HEADER.MinorImageVersion)
    assert img_ver == expected_image_version, "Image version must match build"
    assert pe.OPTIONAL_HEADER.DllCharacteristics == 0, "No ASLR/NX/SafeSEH on NT 3.51"
    assert pe.OPTIONAL_HEADER.AddressOfEntryPoint > 0, "Entry point must be non-zero"


def assert_common_metadata(info: dict[str, str], filename: str, desc: str, version: str):
    assert info["CompanyName"] == "Robert Hargreaves"
    assert info["LegalCopyright"] == "Copyright (c) 2026 Robert Hargreaves"
    assert info["ProductName"] == "VMMouse for NT 3.x"
    assert info["OriginalFilename"] == filename
    assert info["FileDescription"] == desc
    assert info["FileVersion"] == version
    assert info["ProductVersion"] == version


def test_vmmouse_sys_header(build_dir, expected_image_version):
    pe = pefile.PE(os.path.join(build_dir, "vmmouse.sys"))
    assert_common_pe_header(pe, expected_image_version)
    assert pe.OPTIONAL_HEADER.ImageBase == 0x10000
    assert pe.OPTIONAL_HEADER.Subsystem == pefile.SUBSYSTEM_TYPE["IMAGE_SUBSYSTEM_NATIVE"]


def test_vmmouse_sys_metadata(build_dir, build_version):
    pe = pefile.PE(os.path.join(build_dir, "vmmouse.sys"))
    info = get_version_info(pe)
    assert_common_metadata(
        info, "vmmouse.sys", "VMware/QEMU Mouse Integration Driver", build_version
    )


def test_instdrv_exe_header(build_dir, expected_image_version):
    pe = pefile.PE(os.path.join(build_dir, "instdrv.exe"))
    assert_common_pe_header(pe, expected_image_version)
    assert pe.OPTIONAL_HEADER.ImageBase == 0x400000
    assert pe.OPTIONAL_HEADER.Subsystem == pefile.SUBSYSTEM_TYPE["IMAGE_SUBSYSTEM_WINDOWS_CUI"]


def test_instdrv_exe_metadata(build_dir, build_version):
    pe = pefile.PE(os.path.join(build_dir, "instdrv.exe"))
    info = get_version_info(pe)
    assert_common_metadata(info, "instdrv.exe", "VMMouse Driver Installer", build_version)
