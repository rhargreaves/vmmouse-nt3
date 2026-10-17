#!/usr/bin/env python3
import sys
from pathlib import Path


def patch_pe_checksum(filepath: Path) -> int:
    data = bytearray(filepath.read_bytes())
    pe_offset = int.from_bytes(data[0x3c:0x40], "little")

    # Clear DllCharacteristics (offset 0x5E from PE header in OptionalHeader32)
    data[pe_offset + 0x5e:pe_offset + 0x60] = b"\x00\x00"

    checksum_offset = pe_offset + 0x58
    data[checksum_offset:checksum_offset + 4] = b"\x00\x00\x00\x00"

    total = 0
    for i in range(0, len(data), 2):
        if i + 1 < len(data):
            word = int.from_bytes(data[i:i+2], "little")
        else:
            word = data[i]
        total += word
        total = (total & 0xffff) + (total >> 16)

    total = (total & 0xffff) + (total >> 16)
    checksum = (total + len(data)) & 0xffffffff
    data[checksum_offset:checksum_offset + 4] = checksum.to_bytes(4, "little")
    filepath.write_bytes(data)
    return checksum


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <pe-file>")
        sys.exit(1)
    for arg in sys.argv[1:]:
        cs = patch_pe_checksum(Path(arg))
        print(f"Patched {arg} checksum: 0x{cs:08x}")
