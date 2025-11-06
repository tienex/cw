/** @file
  MMIX Binary loader implementation.

  This file implements the main binary loader that detects and loads
  various executable formats.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/MmixLoader.h"
#include "../../include/MmixEmulator.h"

//
// Magic number definitions
//

#define ELF_MAGIC           0x7F454C46  // "\x7FELF"
#define MACHO_MAGIC_32      0xFEEDFACE  // Mach-O 32-bit
#define MACHO_MAGIC_64      0xFEEDFACF  // Mach-O 64-bit
#define MACHO_CIGAM_32      0xCEFAEDFE  // Mach-O 32-bit (byte-swapped)
#define MACHO_CIGAM_64      0xCFFAEDFE  // Mach-O 64-bit (byte-swapped)
#define PE_MAGIC            0x5A4D      // "MZ"
#define MMO_MAGIC           0x98090100  // MMO magic

/**
  Detect binary format from file.

  Reads the file header to determine the binary format.

  @param[in]   FilePath         Path to binary file.
  @param[out]  Format           Pointer to receive format.

  @retval MMIX_SUCCESS          Format detected successfully.
  @retval MMIX_ERROR_NOT_FOUND  File not found.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderDetectFormat (
  IN  CONST CHAR8         *FilePath,
  OUT MMIX_BINARY_FORMAT  *Format
  )
{
  FILE    *File;
  UINT8   Header[64];
  UINT32  Magic32;
  UINT16  Magic16;
  size_t  BytesRead;

  if (FilePath == NULL || Format == NULL) {
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
  // Read header
  //
  BytesRead = fread (Header, 1, sizeof (Header), File);
  fclose (File);

  if (BytesRead < 4) {
    *Format = MmixBinaryFormatRaw;
    return MMIX_SUCCESS;
  }

  //
  // Check ELF magic
  //
  Magic32 = (Header[0] << 24) | (Header[1] << 16) | (Header[2] << 8) | Header[3];
  if (Magic32 == ELF_MAGIC) {
    //
    // Check for 32-bit or 64-bit
    //
    if (BytesRead >= 5) {
      if (Header[4] == 1) {
        *Format = MmixBinaryFormatElf32;
      } else if (Header[4] == 2) {
        *Format = MmixBinaryFormatElf64;
      } else {
        *Format = MmixBinaryFormatUnknown;
      }
      return MMIX_SUCCESS;
    }
  }

  //
  // Check Mach-O magic
  //
  Magic32 = (Header[0] << 24) | (Header[1] << 16) | (Header[2] << 8) | Header[3];
  if (Magic32 == MACHO_MAGIC_32 || Magic32 == MACHO_CIGAM_32) {
    *Format = MmixBinaryFormatMacho32;
    return MMIX_SUCCESS;
  }
  if (Magic32 == MACHO_MAGIC_64 || Magic32 == MACHO_CIGAM_64) {
    *Format = MmixBinaryFormatMacho64;
    return MMIX_SUCCESS;
  }

  //
  // Check PE magic (MZ)
  //
  Magic16 = (Header[0] << 8) | Header[1];
  if (Magic16 == PE_MAGIC) {
    //
    // Need to check PE signature for 32/64-bit
    // This is a simplified check
    //
    *Format = MmixBinaryFormatPe32;  // Default to PE32
    return MMIX_SUCCESS;
  }

  //
  // Check MMO magic
  //
  Magic32 = (Header[0] << 24) | (Header[1] << 16) | (Header[2] << 8) | Header[3];
  if (Magic32 == MMO_MAGIC) {
    *Format = MmixBinaryFormatMmo;
    return MMIX_SUCCESS;
  }

  //
  // Unknown format, treat as raw
  //
  *Format = MmixBinaryFormatRaw;
  return MMIX_SUCCESS;
}

/**
  Load binary into emulator memory.

  Loads a binary file in any supported format into emulator memory,
  parsing headers and setting up the environment appropriately.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to binary file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          Binary loaded successfully.
  @retval MMIX_ERROR_NOT_FOUND  File not found.
  @retval MMIX_ERROR_UNSUPPORTED  Unsupported format.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadBinary (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  )
{
  MMIX_STATUS         Status;
  MMIX_BINARY_FORMAT  Format;
  MMIX_BINARY_INFO    LocalInfo;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Detect format
  //
  Status = MmixLoaderDetectFormat (FilePath, &Format);
  if (MMIX_IS_ERROR (Status)) {
    return Status;
  }

  //
  // Initialize info
  //
  memset (&LocalInfo, 0, sizeof (MMIX_BINARY_INFO));
  LocalInfo.Format = Format;

  //
  // Load based on format
  //
  switch (Format) {
    case MmixBinaryFormatElf32:
    case MmixBinaryFormatElf64:
      Status = MmixLoaderLoadElf (Context, FilePath, &LocalInfo);
      break;

    case MmixBinaryFormatMacho32:
    case MmixBinaryFormatMacho64:
      Status = MmixLoaderLoadMacho (Context, FilePath, &LocalInfo);
      break;

    case MmixBinaryFormatPe32:
    case MmixBinaryFormatPe64:
      Status = MmixLoaderLoadPe (Context, FilePath, &LocalInfo);
      break;

    case MmixBinaryFormatMmo:
      Status = MmixLoaderLoadMmo (Context, FilePath, &LocalInfo);
      break;

    case MmixBinaryFormatRaw:
      //
      // Load as raw binary at address 0
      //
      Status = MmixEmulatorLoadImage (Context, FilePath, 0);
      LocalInfo.EntryPoint = 0;
      LocalInfo.Endianness = MmixEndianBig;
      LocalInfo.Bits = 64;
      break;

    default:
      return MMIX_ERROR_UNSUPPORTED;
  }

  //
  // Copy info if requested
  //
  if (Info != NULL && !MMIX_IS_ERROR (Status)) {
    memcpy (Info, &LocalInfo, sizeof (MMIX_BINARY_INFO));
  }

  return Status;
}

/**
  Get format name string.

  @param[in]  Format            Binary format.

  @return String name of format.

**/
CONST CHAR8 *
MmixLoaderGetFormatName (
  IN MMIX_BINARY_FORMAT  Format
  )
{
  switch (Format) {
    case MmixBinaryFormatRaw:
      return "Raw Binary";
    case MmixBinaryFormatElf32:
      return "ELF 32-bit";
    case MmixBinaryFormatElf64:
      return "ELF 64-bit";
    case MmixBinaryFormatMacho32:
      return "Mach-O 32-bit";
    case MmixBinaryFormatMacho64:
      return "Mach-O 64-bit";
    case MmixBinaryFormatPe32:
      return "PE32";
    case MmixBinaryFormatPe64:
      return "PE32+";
    case MmixBinaryFormatMmo:
      return "MMIX Object";
    default:
      return "Unknown";
  }
}
