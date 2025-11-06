/** @file
  COFF Binary Format Library.

  This library provides support for parsing and creating COFF (Common Object
  File Format) binaries and its many variants. It implements the unified
  BINFORMAT_API interface.

  Supported formats:
  - PE/COFF (Portable Executable / Windows)
  - BigObj (Extended COFF for large object files)
  - SVR3 COFF (System V Release 3 COFF)
  - XCOFF32 (Extended COFF 32-bit for AIX)
  - XCOFF64 (Extended COFF 64-bit for AIX)
  - ECOFF32 (Extended COFF 32-bit for MIPS)
  - ECOFF64 (Extended COFF 64-bit for MIPS/Alpha)
  - TE (Terse Executable for UEFI)

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBCOFF_H__
#define __LIBCOFF_H__

#include "../../include/binformat/BinFormat.h"

///
/// COFF magic numbers
///
#define COFF_MAGIC_I386     0x014C  ///< Intel 386
#define COFF_MAGIC_AMD64    0x8664  ///< AMD64/x86-64
#define COFF_MAGIC_ARM      0x01C0  ///< ARM little-endian
#define COFF_MAGIC_ARMNT    0x01C4  ///< ARM Thumb-2
#define COFF_MAGIC_ARM64    0xAA64  ///< ARM64
#define COFF_MAGIC_IA64     0x0200  ///< Intel Itanium

///
/// PE signature
///
#define PE_SIGNATURE        0x00004550  ///< "PE\0\0"

///
/// BigObj magic
///
#define BIGOBJ_MAGIC_ANON   0xFFFF
#define BIGOBJ_VERSION      2

///
/// TE magic
///
#define TE_MAGIC            0x5A56  ///< "VZ"

///
/// XCOFF magic numbers
///
#define XCOFF32_MAGIC       0x01DF
#define XCOFF64_MAGIC       0x01F7

///
/// ECOFF magic numbers
///
#define ECOFF_MAGIC_MIPS    0x0162
#define ECOFF_MAGIC_ALPHA   0x0183

///
/// Get the COFF library API table.
///
/// @return Pointer to COFF library API table.
///
EXTERN CONST BINFORMAT_API *
CoffGetApi (
  VOID
  );

#endif // __LIBCOFF_H__
