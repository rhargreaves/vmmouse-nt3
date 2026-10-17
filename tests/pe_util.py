from dataclasses import dataclass
import os
import struct
from typing import Tuple

IMAGE_FILE_MACHINE_I386 = 0x14C
PE32_MAGIC = 0x10B
SUBSYSTEM_NATIVE = 1
SUBSYSTEM_WINDOWS_CUI = 3


@dataclass(frozen=True)
class PeInfo:
    data: bytes
    machine: int
    entry_point: int
    image_base: int
    subsystem: int
    subsystem_version: Tuple[int, int]
    image_version: Tuple[int, int]
    dll_characteristics: int

    def has_resource_string(self, text: str) -> bool:
        return text.encode("utf-16le") in self.data


def parse_pe(path: str) -> PeInfo:
    assert os.path.isfile(path), f"File not found: {path}"
    with open(path, "rb") as f:
        data = f.read()

    assert data[:2] == b"MZ", "Missing 'MZ' DOS header magic"
    e_lfanew = struct.unpack_from("<I", data, 0x3C)[0]
    assert data[e_lfanew : e_lfanew + 4] == b"PE\x00\x00", "Missing 'PE' signature"

    coff_offset = e_lfanew + 4
    machine, _, _, _, _, _, _ = struct.unpack_from("<HHIIIHH", data, coff_offset)

    opt_offset = coff_offset + 20
    magic = struct.unpack_from("<H", data, opt_offset)[0]
    assert magic == PE32_MAGIC, f"Expected PE32 magic {PE32_MAGIC:#x}, got {magic:#x}"

    entry_point = struct.unpack_from("<I", data, opt_offset + 16)[0]
    image_base = struct.unpack_from("<I", data, opt_offset + 28)[0]
    maj_img, min_img = struct.unpack_from("<HH", data, opt_offset + 44)
    maj_sub, min_sub = struct.unpack_from("<HH", data, opt_offset + 48)
    subsystem = struct.unpack_from("<H", data, opt_offset + 68)[0]
    dll_char = struct.unpack_from("<H", data, opt_offset + 70)[0]

    return PeInfo(
        data=data,
        machine=machine,
        entry_point=entry_point,
        image_base=image_base,
        subsystem=subsystem,
        subsystem_version=(maj_sub, min_sub),
        image_version=(maj_img, min_img),
        dll_characteristics=dll_char,
    )
