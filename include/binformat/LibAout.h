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
/// a.out magic numbers (octal notation)
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
/// BSD a.out Magic Numbers
///
#define OMAGIC_BSD      0407    ///< Old impure format
#define NMAGIC_BSD      0410    ///< Read-only text
#define ZMAGIC_BSD      0413    ///< Demand paged
#define QMAGIC_BSD      0314    ///< Compact QMAGIC (NetBSD)

///
/// NetBSD a.out Machine Types
///
#define MID_ZERO        0       ///< Unknown
#define MID_SUN010      1       ///< Sun 68010
#define MID_SUN020      2       ///< Sun 68020
#define MID_PC386       100     ///< Intel 386
#define MID_M68K        1       ///< Motorola 68000
#define MID_M68K4K      2       ///< M68K 4K pages
#define MID_NS32532     137     ///< NS32532
#define MID_SPARC       138     ///< SPARC
#define MID_PMAX        139     ///< PMAX (MIPS)
#define MID_VAX         140     ///< VAX
#define MID_ALPHA       141     ///< Alpha
#define MID_MIPS        142     ///< MIPS
#define MID_ARM6        143     ///< ARM6
#define MID_HP200       200     ///< HP 200
#define MID_HP300       300     ///< HP 300
#define MID_HPUX        0x20C   ///< HP-UX
#define MID_HPUX800     0x20B   ///< HP-UX 800

///
/// Linux a.out Machine Types
///
#define M_OLDSUN2       0       ///< Old Sun2
#define M_68010         1       ///< Motorola 68010
#define M_68020         2       ///< Motorola 68020
#define M_SPARC         3       ///< SPARC
#define M_386           100     ///< Intel 386
#define M_MIPS1         151     ///< MIPS R2000/R3000
#define M_MIPS2         152     ///< MIPS R4000/R6000
#define M_ARM           103     ///< ARM
#define M_SPARCLET      131     ///< SPARClet
#define M_29K           101     ///< AMD 29000
#define M_PPC           102     ///< PowerPC

///
/// Linux a.out Flags
///
#define A_FLAG_QMAGIC   0x0001  ///< QMAGIC format

///
/// NS32K a.out Magic
///
#define NS32K_MAGIC_MACH    0532    ///< pc532mach
#define NS32K_MAGIC_NBSD    0532    ///< NetBSD NS32K

///
/// Sony NEWS a.out (newsos3)
///
#define NEWSOS3_MAGIC   0x010B

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
