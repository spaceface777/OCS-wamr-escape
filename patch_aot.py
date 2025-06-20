#!/usr/bin/env python3
"""Patch WAMR AOT output: replace guard opcodes with Thumb NOP (0xBF00)."""
import sys, struct, pathlib

PATTERNS = [
    b"\x14\xd2",
    b"\x0f\xd8",
]
NOP = b"\x00\xBF"

in_path  = pathlib.Path(sys.argv[1])
out_path = in_path.with_suffix('.patched.aot')
blob     = bytearray(in_path.read_bytes())

replacements = 0
for pat in PATTERNS:
    idx = 0
    while True:
        idx = blob.find(pat, idx)
        if idx == -1:
            break
        # overwrite each 16‑bit chunk inside the pattern with a NOP
        for off in range(0, len(pat), 2):
            print(f"[+] patching {in_path.name} at offset {idx + off:#x} with NOP")
            blob[idx + off : idx + off + 2] = NOP
            replacements += len(pat) // 2
            idx += len(pat)

print(f"[+] patched {replacements} instructions → {out_path.name}")
out_path.write_bytes(blob)
