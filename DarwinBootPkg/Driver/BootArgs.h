// SPDX-License-Identifier: BSD-2-Clause-Patent
#ifndef DARWIN_BOOT_ARGS_H_
#define DARWIN_BOOT_ARGS_H_

#include <Base.h>

//
// boot_args, as pexpert/pexpert/arm64/boot.h defines it for this kernel.
//
// The layout is pinned to what the kernel was actually compiled against. For
// xnu-8796 (iOS 16 / macOS 13) that header says, literally:
//
//   #define BOOT_LINE_LENGTH        608
//   struct Boot_Video {
//       unsigned long v_baseAddr;   /* unsigned long: 8 bytes on arm64 */
//       unsigned long v_display;
//       unsigned long v_rowBytes;
//       unsigned long v_width;
//       unsigned long v_height;
//       unsigned long v_depth;
//   };                              /* 48 bytes -- NOT six uint32 */
//
// so the fields land on:
//
//   Revision            0x00        VirtualBase        0x08
//   PhysicalBase        0x10        MemorySize         0x18
//   TopOfKernelData     0x20        Video              0x28  (48 bytes)
//   machineType         0x58        deviceTreeP        0x60
//   deviceTreeLength    0x68        CommandLine        0x6c  (608 bytes)
//   bootFlags           0x2d0       memSizeActual      0x2d8
//   sizeof              0x2e0
//
// ios-sweet's loader.c asserts the same three numbers against its own copy of
// the struct (Video at 40, CommandLine at 108, sizeof 736) and boots this very
// kernel to the iOS home screen, so this is the layout to match.
//
// The only thing that was ever wrong here was the CommandLine length: the
// original header used 1024 (the Darwin 23 width), which made sizeof 0x480.
// Shrinking it to the kernel's own BOOT_LINE_LENGTH of 608 is the fix. Video
// must stay six 64-bit words -- treating it as six uint32 moves deviceTreeP
// from 0x60 down to 0x48, and the kernel then reads a device tree pointer out
// of the middle of the video fields and dies before it can say anything.
//
#define BOOT_LINE_LENGTH  608   /* xnu-8796 pexpert/pexpert/arm64/boot.h */

typedef struct {
  UINT16  Revision;          /* 0x00 */
  UINT16  Version;           /* 0x02 */
  UINT32  Padding;           /* 0x04 */
  UINT64  VirtualBase;       /* 0x08 */
  UINT64  PhysicalBase;      /* 0x10 */
  UINT64  MemorySize;        /* 0x18 */
  UINT64  TopOfKernelData;   /* 0x20 */
  UINT64  Video[6];          /* 0x28: Boot_Video, six unsigned long = 48 bytes */
  UINT32  MachineType;       /* 0x58 */
  UINT32  TreePadding;       /* 0x5c */
  UINT64  DeviceTree;        /* 0x60 */
  UINT32  DeviceTreeLength;  /* 0x68 */
  CHAR8   CommandLine[BOOT_LINE_LENGTH];  /* 0x6c, 608 bytes -> ends 0x2cc */
  UINT64  BootFlags;         /* 0x2d0, aligned up from 0x2cc */
  UINT64  MemorySizeActual;  /* 0x2d8 */
} DARWIN_BOOT_ARGS;

STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, Video) == 0x28, "video ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, MachineType) == 0x58, "machine type ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, DeviceTree) == 0x60, "DT ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, DeviceTreeLength) == 0x68, "DT length ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, CommandLine) == 0x6c, "command line ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, BootFlags) == 0x2d0, "boot flags ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, MemorySizeActual) == 0x2d8, "memory ABI");
STATIC_ASSERT (sizeof (DARWIN_BOOT_ARGS) == 0x2e0, "boot_args ABI");

#endif
