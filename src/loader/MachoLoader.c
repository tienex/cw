/** @file
  Mach-O binary loader implementation.

  This file implements loading of Mach-O (Mach Object) binaries
  used by macOS and iOS, supporting both 32-bit and 64-bit variants.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/MmixLoader.h"

//
// Mach-O magic numbers
//

#define MH_MAGIC      0xFEEDFACE  // 32-bit big-endian
#define MH_CIGAM      0xCEFAEDFE  // 32-bit little-endian
#define MH_MAGIC_64   0xFEEDFACF  // 64-bit big-endian
#define MH_CIGAM_64   0xCFFAEDFE  // 64-bit little-endian

//
// Mach-O load commands
//

#define LC_SEGMENT    0x1   // 32-bit segment
#define LC_SEGMENT_64 0x19  // 64-bit segment

//
// Mach-O structures
//

#pragma pack(push, 1)

typedef struct {
  UINT32  magic;
  UINT32  cputype;
  UINT32  cpusubtype;
  UINT32  filetype;
  UINT32  ncmds;
  UINT32  sizeofcmds;
  UINT32  flags;
} mach_header;

typedef struct {
  UINT32  magic;
  UINT32  cputype;
  UINT32  cpusubtype;
  UINT32  filetype;
  UINT32  ncmds;
  UINT32  sizeofcmds;
  UINT32  flags;
  UINT32  reserved;
} mach_header_64;

typedef struct {
  UINT32  cmd;
  UINT32  cmdsize;
} load_command;

typedef struct {
  UINT32  cmd;
  UINT32  cmdsize;
  CHAR8   segname[16];
  UINT32  vmaddr;
  UINT32  vmsize;
  UINT32  fileoff;
  UINT32  filesize;
  UINT32  maxprot;
  UINT32  initprot;
  UINT32  nsects;
  UINT32  flags;
} segment_command;

typedef struct {
  UINT32  cmd;
  UINT32  cmdsize;
  CHAR8   segname[16];
  UINT64  vmaddr;
  UINT64  vmsize;
  UINT64  fileoff;
  UINT64  filesize;
  UINT32  maxprot;
  UINT32  initprot;
  UINT32  nsects;
  UINT32  flags;
} segment_command_64;

#pragma pack(pop)

/**
  Swap 32-bit value for endianness.

  @param[in]  Value             Value to swap.

  @return Swapped value.

**/
STATIC
UINT32
Swap32 (
  IN UINT32  Value
  )
{
  return ((Value & 0xFF) << 24) |
         (((Value >> 8) & 0xFF) << 16) |
         (((Value >> 16) & 0xFF) << 8) |
         ((Value >> 24) & 0xFF);
}

/**
  Swap 64-bit value for endianness.

  @param[in]  Value             Value to swap.

  @return Swapped value.

**/
STATIC
UINT64
Swap64 (
  IN UINT64  Value
  )
{
  return ((Value & 0xFF) << 56) |
         (((Value >> 8) & 0xFF) << 48) |
         (((Value >> 16) & 0xFF) << 40) |
         (((Value >> 24) & 0xFF) << 32) |
         (((Value >> 32) & 0xFF) << 24) |
         (((Value >> 40) & 0xFF) << 16) |
         (((Value >> 48) & 0xFF) << 8) |
         ((Value >> 56) & 0xFF);
}

/**
  Load Mach-O binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to Mach-O file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          Mach-O loaded successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadMacho (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  )
{
  FILE            *File;
  UINT32          Magic;
  BOOLEAN         Is64Bit;
  BOOLEAN         SwapEndian;
  mach_header     Header32;
  mach_header_64  Header64;
  UINT32          NumCommands;
  UINT32          i;
  UINT32          Offset;
  MMIX_BINARY_INFO  LocalInfo;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  File = fopen (FilePath, "rb");
  if (File == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Read magic
  //
  if (fread (&Magic, 1, sizeof (UINT32), File) != sizeof (UINT32)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Determine architecture and endianness
  //
  Is64Bit = (Magic == MH_MAGIC_64 || Magic == MH_CIGAM_64);
  SwapEndian = (Magic == MH_CIGAM || Magic == MH_CIGAM_64);

  //
  // Initialize info
  //
  memset (&LocalInfo, 0, sizeof (MMIX_BINARY_INFO));
  LocalInfo.Format = Is64Bit ? MmixBinaryFormatMacho64 : MmixBinaryFormatMacho32;
  LocalInfo.Endianness = SwapEndian ? MmixEndianLittle : MmixEndianBig;
  LocalInfo.Bits = Is64Bit ? 64 : 32;

  //
  // Read header
  //
  fseek (File, 0, SEEK_SET);
  if (Is64Bit) {
    if (fread (&Header64, 1, sizeof (mach_header_64), File) != sizeof (mach_header_64)) {
      fclose (File);
      return MMIX_ERROR_DEVICE_ERROR;
    }
    NumCommands = SwapEndian ? Swap32 (Header64.ncmds) : Header64.ncmds;
    Offset = sizeof (mach_header_64);
  } else {
    if (fread (&Header32, 1, sizeof (mach_header), File) != sizeof (mach_header)) {
      fclose (File);
      return MMIX_ERROR_DEVICE_ERROR;
    }
    NumCommands = SwapEndian ? Swap32 (Header32.ncmds) : Header32.ncmds;
    Offset = sizeof (mach_header);
  }

  //
  // Process load commands
  //
  for (i = 0; i < NumCommands; i++) {
    load_command  Cmd;
    UINT32        CmdType;
    UINT32        CmdSize;

    fseek (File, Offset, SEEK_SET);
    if (fread (&Cmd, 1, sizeof (load_command), File) != sizeof (load_command)) {
      break;
    }

    CmdType = SwapEndian ? Swap32 (Cmd.cmd) : Cmd.cmd;
    CmdSize = SwapEndian ? Swap32 (Cmd.cmdsize) : Cmd.cmdsize;

    //
    // Handle segment load commands
    //
    if (CmdType == LC_SEGMENT_64 && Is64Bit) {
      segment_command_64  Seg;
      UINT8               *SegmentData;

      fseek (File, Offset, SEEK_SET);
      if (fread (&Seg, 1, sizeof (segment_command_64), File) == sizeof (segment_command_64)) {
        UINT64  FileOff = SwapEndian ? Swap64 (Seg.fileoff) : Seg.fileoff;
        UINT64  FileSize = SwapEndian ? Swap64 (Seg.filesize) : Seg.filesize;
        UINT64  VmAddr = SwapEndian ? Swap64 (Seg.vmaddr) : Seg.vmaddr;

        if (FileSize > 0 && VmAddr < Context->MemoryState->PhysicalMemorySize) {
          SegmentData = (UINT8 *)malloc (FileSize);
          if (SegmentData != NULL) {
            fseek (File, FileOff, SEEK_SET);
            if (fread (SegmentData, 1, FileSize, File) == FileSize) {
              memcpy (&Context->MemoryState->PhysicalMemory[VmAddr], SegmentData, FileSize);
              LocalInfo.LoadedSize += FileSize;
            }
            free (SegmentData);
          }
        }
      }
    } else if (CmdType == LC_SEGMENT && !Is64Bit) {
      segment_command  Seg;
      UINT8            *SegmentData;

      fseek (File, Offset, SEEK_SET);
      if (fread (&Seg, 1, sizeof (segment_command), File) == sizeof (segment_command)) {
        UINT32  FileOff = SwapEndian ? Swap32 (Seg.fileoff) : Seg.fileoff;
        UINT32  FileSize = SwapEndian ? Swap32 (Seg.filesize) : Seg.filesize;
        UINT32  VmAddr = SwapEndian ? Swap32 (Seg.vmaddr) : Seg.vmaddr;

        if (FileSize > 0 && VmAddr < Context->MemoryState->PhysicalMemorySize) {
          SegmentData = (UINT8 *)malloc (FileSize);
          if (SegmentData != NULL) {
            fseek (File, FileOff, SEEK_SET);
            if (fread (SegmentData, 1, FileSize, File) == FileSize) {
              memcpy (&Context->MemoryState->PhysicalMemory[VmAddr], SegmentData, FileSize);
              LocalInfo.LoadedSize += FileSize;
            }
            free (SegmentData);
          }
        }
      }
    }

    Offset += CmdSize;
  }

  fclose (File);

  //
  // Set entry point (simplified - use base address)
  //
  LocalInfo.EntryPoint = 0;
  Context->CpuState->Pc = LocalInfo.EntryPoint;

  //
  // Copy info if requested
  //
  if (Info != NULL) {
    memcpy (Info, &LocalInfo, sizeof (MMIX_BINARY_INFO));
  }

  return MMIX_SUCCESS;
}
