// SPDX-License-Identifier: BSD-2-Clause-Patent
#include <AsmMacroLib.h>

ASM_FUNC(DarwinCurrentEl)
  mrs x0, CurrentEL
  ret

// x0=boot_args PA, x1=entry PA, x2=owned PA, x3=owned size, x4=frame buffer
// PA (0 for none), x5=its size in bytes. EL1 only.
ASM_FUNC(DarwinEnter)
  msr daifset, #15
  // Breadcrumb stage 1: we are inside DarwinEnter with interrupts masked.
  movz x9, #0x0008, lsl #32
  movk x9, #0xd300, lsl #16
  movk x9, #0x3000
  mov w10, #1
  str w10, [x9, #40]
  dsb sy
  // Paint the top 1/16 of the framebuffer green (BGRA 0xFF00FF00)
  // before cache maintenance, MMU shutdown and the branch to the kernel.
  cbz x4, 2f
  lsr x9, x5, #4
  bic x9, x9, #7
  mov w10, #0xFF00
  movk w10, #0xFF00, lsl #16
  mov x11, x4
3:
  stp w10, w10, [x11], #8
  subs x9, x9, #8
  b.hi 3b
  dsb sy
2:
  add x5, x2, x3
  mrs x6, ctr_el0
  ubfx x6, x6, #16, #4
  mov x7, #4
  lsl x6, x7, x6
  sub x7, x6, #1
  bic x4, x2, x7
  dsb sy
1:
  dc civac, x4
  add x4, x4, x6
  cmp x4, x5
  b.lo 1b
  dsb sy
  ic iallu
  dsb sy
  isb
  // Breadcrumb stage 2: caches cleaned, about to read SCTLR.
  movz x9, #0x0008, lsl #32
  movk x9, #0xd300, lsl #16
  movk x9, #0x3000
  mov w10, #2
  str w10, [x9, #40]
  dsb sy
  mrs x4, sctlr_el1
  bic x4, x4, #1
  bic x4, x4, #4
  bic x4, x4, #0x1000
  msr sctlr_el1, x4
  isb
  // Breadcrumb stage 3: SCTLR written (MMU off) and it did not fault.
  movz x9, #0x0008, lsl #32
  movk x9, #0xd300, lsl #16
  movk x9, #0x3000
  mov w10, #3
  str w10, [x9, #40]
  dsb sy
  tlbi vmalle1
  dsb sy
  isb

  // Record what we hand the kernel, so the next boot can tell "the loader never
  // jumped" apart from "the kernel never ran its first instruction". The kernel
  // writes the same magic at 0x8d3003000 before its own breadcrumbs; note[1] is
  // its arm_init stage mask, note[2] its start.s checkpoint.
  //   note[3] = the SCTLR_EL1 the kernel starts with (M/C/I cleared above)
  //   note[4] = the physical entry point we branch to
  // The window DarwinBoot allocates starts at 0x8c0004000 and is at least
  // 256 MiB, so the last page of a 304 MiB window is ordinary DRAM.
  movz x9, #0x0008, lsl #32
  movk x9, #0xd300, lsl #16
  movk x9, #0x3000
  movz x10, #0x4e4f, lsl #16
  movk x10, #0x5445
  str x10, [x9]
  str x4, [x9, #24]
  str x1, [x9, #32]
  dsb sy
  br x1
