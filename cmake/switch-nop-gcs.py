#!/usr/bin/env python3
# Neutralise AArch64 GCS (FEAT_GCS) + SME (FEAT_SME) instructions in the linked ELF.
#
# devkitPro's GCC 15 libgcc C++ exception unwinder (+ its SME lazy-save cleanup,
# __arm_za_disable / __arm_tpidr2_save) contains GCS and SME instructions, on code
# paths that are feature-gated and never execute on a GCS/SME-less Cortex-A57
# Switch (guarded by CHKFEAT for GCS and a runtime SME-available flag).
#
# The problem is Ryujinx's JIT (ARMeilleure) translates a function's ENTIRE static
# CFG ahead of execution, so it faults *compiling* these instructions
# ("Unknown MRS 0xD53B2521", etc.) the moment the unwinder runs — i.e. on the
# first thrown C++ exception — even though they're unreachable at runtime.
#
# Replacing each with NOP makes them translate cleanly and is behaviourally
# correct: on real hardware these paths are gated off (no GCS/SME on A57), so a
# NOP is exactly what should execute if the guard were ever (wrongly) taken.

import sys

NOP = bytes.fromhex("1f2003d5")   # nop (0xD503201F), little-endian

# name -> little-endian 4-byte encoding
BAD = {
    "chkfeat x16":         "1f2503d5",   # 0xD503251F
    "gcspopm":             "3f772bd5",   # 0xD52B773F
    "mrs x, gcspr_el0":    "21253bd5",   # 0xD53B2521
    "mrs x, tpidr2_el0":   "aed03bd5",   # 0xD53BD0AE
    "msr tpidr2_el0, xzr": "bfd01bd5",   # 0xD51BD0BF
    "smstop za":           "7f4403d5",   # 0xD503447F
    "smstop sm":           "7f4603d5",   # 0xD503467F
}

path = sys.argv[1]
data = open(path, "rb").read()
total = 0
for name, hx in BAD.items():
    pat = bytes.fromhex(hx)
    n = data.count(pat)
    if n:
        data = data.replace(pat, NOP)
        print(f"[switch-nop-gcs]   {name:<20} x{n}")
    total += n
open(path, "wb").write(data)
print(f"[switch-nop-gcs] neutralised {total} GCS/SME instruction(s) in {path}")
