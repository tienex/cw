/** @file
  MMO (MMIX Object) binary loader implementation.

  This file implements loading of MMO format binaries, which is
  Donald Knuth's object format for MMIX.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/MmixLoader.h"

//
// MMO constants
//

#define LOP_QUOTE    0x98000000
#define LOP_LOC      0x98010000
#define LOP_SKIP     0x98020000
#define LOP_FIXO     0x98030000
#define LOP_FIXR     0x98040000
#define LOP_FIXRX    0x98050000
#define LOP_FILE     0x98060000
#define LOP_LINE     0x98070000
#define LOP_SPEC     0x98080000
#define LOP_PRE      0x980A0000
#define LOP_POST     0x980B0000
#define LOP_STAB     0x980C0000
#define LOP_END      0x980D0000

/**
  Load MMO binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to MMO file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          MMO loaded successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadMmo (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  )
{
  FILE              *File;
  UINT32            Tetra;
  UINT64            CurrentAddr;
  MMIX_BINARY_INFO  LocalInfo;
  UINT8             *Buffer;
  size_t            FileSize;

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
  // Get file size
  //
  fseek (File, 0, SEEK_END);
  FileSize = ftell (File);
  fseek (File, 0, SEEK_SET);

  //
  // Read entire file
  //
  Buffer = (UINT8 *)malloc (FileSize);
  if (Buffer == NULL) {
    fclose (File);
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  if (fread (Buffer, 1, FileSize, File) != FileSize) {
    free (Buffer);
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  fclose (File);

  //
  // Initialize info
  //
  memset (&LocalInfo, 0, sizeof (MMIX_BINARY_INFO));
  LocalInfo.Format = MmixBinaryFormatMmo;
  LocalInfo.Endianness = MmixEndianBig;  // MMO is big-endian
  LocalInfo.Bits = 64;

  //
  // Parse MMO format
  // Simplified implementation: just load the tetras
  //
  CurrentAddr = 0;
  for (size_t i = 0; i < FileSize / 4; i++) {
    //
    // Read big-endian tetra
    //
    Tetra = (Buffer[i * 4] << 24) |
            (Buffer[i * 4 + 1] << 16) |
            (Buffer[i * 4 + 2] << 8) |
            Buffer[i * 4 + 3];

    //
    // Check for lop (loader operation)
    //
    if ((Tetra & 0xFF000000) == 0x98000000) {
      UINT32  LopType = Tetra & 0xFFFF0000;

      if (LopType == LOP_LOC) {
        //
        // Set location
        //
        CurrentAddr = (UINT64)(Tetra & 0xFFFF) << 32;
        if (i + 1 < FileSize / 4) {
          i++;
          Tetra = (Buffer[i * 4] << 24) |
                  (Buffer[i * 4 + 1] << 16) |
                  (Buffer[i * 4 + 2] << 8) |
                  Buffer[i * 4 + 3];
          CurrentAddr |= Tetra;
        }
      } else if (LopType == LOP_SKIP) {
        //
        // Skip bytes
        //
        CurrentAddr += (Tetra & 0xFFFF);
      } else if (LopType == LOP_PRE) {
        //
        // Preamble - get entry point
        //
        if ((Tetra & 0xFF) == 1 && i + 2 < FileSize / 4) {
          i++;
          UINT32  High = (Buffer[i * 4] << 24) |
                         (Buffer[i * 4 + 1] << 16) |
                         (Buffer[i * 4 + 2] << 8) |
                         Buffer[i * 4 + 3];
          i++;
          UINT32  Low = (Buffer[i * 4] << 24) |
                        (Buffer[i * 4 + 1] << 16) |
                        (Buffer[i * 4 + 2] << 8) |
                        Buffer[i * 4 + 3];
          LocalInfo.EntryPoint = ((UINT64)High << 32) | Low;
        }
      } else if (LopType == LOP_END) {
        //
        // End of file
        //
        break;
      }
    } else {
      //
      // Regular data tetra
      //
      if (CurrentAddr + 4 <= Context->MemoryState->PhysicalMemorySize) {
        Context->MemoryState->PhysicalMemory[CurrentAddr++] = (Tetra >> 24) & 0xFF;
        Context->MemoryState->PhysicalMemory[CurrentAddr++] = (Tetra >> 16) & 0xFF;
        Context->MemoryState->PhysicalMemory[CurrentAddr++] = (Tetra >> 8) & 0xFF;
        Context->MemoryState->PhysicalMemory[CurrentAddr++] = Tetra & 0xFF;
        LocalInfo.LoadedSize += 4;
      }
    }
  }

  free (Buffer);

  //
  // Set entry point
  //
  Context->CpuState->Pc = LocalInfo.EntryPoint;

  //
  // Copy info if requested
  //
  if (Info != NULL) {
    memcpy (Info, &LocalInfo, sizeof (MMIX_BINARY_INFO));
  }

  return MMIX_SUCCESS;
}
