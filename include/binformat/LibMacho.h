/** @file
  Mach-O Binary Format Library.

  This library provides support for parsing and creating Mach-O (Mach Object)
  binaries including universal binaries. It implements the unified BINFORMAT_API
  interface.

  Supported formats:
  - Mach-O 32-bit
  - Mach-O 64-bit
  - Universal binaries (Fat Mach-O)

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBMACHO_H__
#define __LIBMACHO_H__

#include "BinFormat.h"

///
/// Mach-O magic numbers
///
#define MACHO_MAGIC_32        0xFEEDFACE  ///< 32-bit Mach-O
#define MACHO_MAGIC_64        0xFEEDFACF  ///< 64-bit Mach-O
#define MACHO_MAGIC_32_SWAP   0xCEFAEDFE  ///< 32-bit swapped
#define MACHO_MAGIC_64_SWAP   0xCFFAEDFE  ///< 64-bit swapped
#define MACHO_FAT_MAGIC       0xCAFEBABE  ///< Universal binary
#define MACHO_FAT_MAGIC_SWAP  0xBEBAFECA  ///< Universal swapped

///
/// Get the Mach-O library API table.
///
/// @return Pointer to Mach-O library API table.
///
EXTERN CONST BINFORMAT_API *
MachoGetApi (
  VOID
  );

#endif // __LIBMACHO_H__
