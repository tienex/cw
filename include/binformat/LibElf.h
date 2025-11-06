/** @file
  ELF Binary Format Library.

  This library provides support for parsing and creating ELF (Executable and
  Linkable Format) binaries including ELF32, ELF64, and FatELF (multi-architecture)
  variants. It implements the unified BINFORMAT_API interface.

  Supported formats:
  - ELF32 (32-bit ELF)
  - ELF64 (64-bit ELF)
  - FatELF (multi-architecture ELF containers)

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBELF_H__
#define __LIBELF_H__

#include "BinFormat.h"

///
/// ELF magic number
///
#define ELF_MAGIC_0  0x7F
#define ELF_MAGIC_1  'E'
#define ELF_MAGIC_2  'L'
#define ELF_MAGIC_3  'F'

///
/// FatELF magic number
///
#define FATELF_MAGIC  0x0FAEF1FA

///
/// Get the ELF library API table.
///
/// @return Pointer to ELF library API table.
///
EXTERN CONST BINFORMAT_API *
ElfGetApi (
  VOID
  );

#endif // __LIBELF_H__
