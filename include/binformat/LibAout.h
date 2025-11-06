/** @file
  a.out Binary Format Library.

  This library provides support for parsing and creating a.out (Assembler Output)
  binaries and its variants. It implements the unified BINFORMAT_API interface.

  Supported formats:
  - a.out (traditional Unix)
  - b.out (early Unix variant)
  - Plan 9 a.out

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBAOUT_H__
#define __LIBAOUT_H__

#include "BinFormat.h"

///
/// a.out magic numbers
///
#define AOUT_OMAGIC    0407  ///< Old impure format
#define AOUT_NMAGIC    0410  ///< Read-only text
#define AOUT_ZMAGIC    0413  ///< Demand load format
#define AOUT_QMAGIC    0314  ///< Compact format
#define AOUT_CMAGIC    0421  ///< Core file

///
/// b.out magic
///
#define BOUT_MAGIC     0407

///
/// Plan 9 a.out magic numbers (various architectures)
///
#define PLAN9_MAGIC_68020    0x80000107  ///< 68020
#define PLAN9_MAGIC_MIPS     0x80000108  ///< MIPS
#define PLAN9_MAGIC_386      0x800000EB  ///< Intel 386
#define PLAN9_MAGIC_SPARC    0x80000109  ///< SPARC
#define PLAN9_MAGIC_ARM      0x8000010A  ///< ARM
#define PLAN9_MAGIC_ALPHA    0x8000010B  ///< DEC Alpha
#define PLAN9_MAGIC_POWER    0x8000010C  ///< PowerPC

///
/// Get the a.out library API table.
///
/// @return Pointer to a.out library API table.
///
EXTERN CONST BINFORMAT_API *
AoutGetApi (
  VOID
  );

#endif // __LIBAOUT_H__
