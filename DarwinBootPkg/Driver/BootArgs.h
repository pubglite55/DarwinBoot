// SPDX-License-Identifier: BSD-2-Clause-Patent
#ifndef DARWIN_BOOT_ARGS_H_
#define DARWIN_BOOT_ARGS_H_

#include <Base.h>

//
// boot_args, as pexpert/pexpert/arm64/boot.h defines it for the kernel we boot.
//
// Two layouts exist and the kernel reads whichever it was built against, so every
// field after Video must line up exactly -- an off-by-24 here makes the kernel read
// garbage for machineType/deviceTreeP/CommandLine and die before it can print:
//
//   xnu-8796 and older (iOS 16 / macOS 13) -- what this device boots:
//     Boot_Video Video        6 x uint32                     @0x28  (24 bytes)
//     uint32_t   machineType                                 @0x40
//     void      *deviceTreeP                                 @0x48
//     uint32_t   deviceTreeLength                            @0x50
//     char       CommandLine[608]                            @0x54
//     uint64_t   bootFlags                                   @0x2b8
//     uint64_t   memSizeActual                               @0x2c0
//     sizeof                                                 @0x2c8
//
//   Darwin 23 (macOS 14+) widened Video to uint64[6] (48 bytes) and CommandLine
//   to 1024, which shifts everything after it. That is the ABI this header used
//   to describe; it is not the one xnu-8796 was built with.
//
// BOOT_LINE_LENGTH is 608 in xnu-8796's boot.h (and in ios-sweet's mkpayload.py).
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
  UINT32  Video[6];          /* 0x28: Boot_Video, six uint32 */
  UINT32  MachineType;       /* 0x40 */
  UINT32  TreePadding;       /* 0x44 */
  UINT64  DeviceTree;        /* 0x48 */
  UINT32  DeviceTreeLength;  /* 0x50 */
  CHAR8   CommandLine[BOOT_LINE_LENGTH];  /* 0x54 */
  UINT64  BootFlags;         /* 0x2b8 (4 bytes of alignment padding after 0x2b4) */
  UINT64  MemorySizeActual;  /* 0x2c0 */
} DARWIN_BOOT_ARGS;

STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, Video) == 0x28, "video ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, MachineType) == 0x40, "machine type ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, DeviceTree) == 0x48, "DT ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, DeviceTreeLength) == 0x50, "DT length ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, CommandLine) == 0x54, "command line ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, BootFlags) == 0x2b8, "boot flags ABI");
STATIC_ASSERT (OFFSET_OF (DARWIN_BOOT_ARGS, MemorySizeActual) == 0x2c0, "memory ABI");
STATIC_ASSERT (sizeof (DARWIN_BOOT_ARGS) == 0x2c8, "boot_args ABI");

#endif
