/** @file
  OMF Binary Format Library.

  This library provides support for parsing and creating OMF (Object Module Format)
  binaries and related DOS/Windows executable formats. It implements the unified
  BINFORMAT_API interface.

  Supported formats:
  - OMF (Object Module Format / Intel OMF-86/OMF-386)
  - MZ (DOS executable)
  - NE (Windows 16-bit New Executable)
  - LE (Linear Executable for OS/2 and Windows VxD)
  - LX (Linear Executable for OS/2)
  - Xenix x.out

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBOMF_H__
#define __LIBOMF_H__

#include "BinFormat.h"

///
/// MZ magic number
///
#define MZ_MAGIC           0x5A4D  ///< "MZ"
#define ZM_MAGIC           0x4D5A  ///< "ZM" (rare variant)

///
/// NE magic number
///
#define NE_MAGIC           0x454E  ///< "NE"

///
/// LE magic number
///
#define LE_MAGIC           0x454C  ///< "LE"

///
/// LX magic number
///
#define LX_MAGIC           0x584C  ///< "LX"

///
/// OMF record types
///
#define OMF_THEADR         0x80    ///< Translator Header
#define OMF_LHEADR         0x82    ///< Library Module Header
#define OMF_COMENT         0x88    ///< Comment
#define OMF_MODEND         0x8A    ///< Module End
#define OMF_EXTDEF         0x8C    ///< External Names
#define OMF_PUBDEF         0x90    ///< Public Names
#define OMF_LINNUM         0x94    ///< Line Numbers
#define OMF_LNAMES         0x96    ///< List of Names
#define OMF_SEGDEF         0x98    ///< Segment Definition
#define OMF_GRPDEF         0x9A    ///< Group Definition
#define OMF_FIXUPP         0x9C    ///< Fixup
#define OMF_LEDATA         0xA0    ///< Logical Enumerated Data
#define OMF_LIDATA         0xA2    ///< Logical Iterated Data

///
/// Xenix x.out magic
///
#define XOUT_MAGIC         0x206    ///< Xenix executable

///
/// Get the OMF library API table.
///
/// @return Pointer to OMF library API table.
///
EXTERN CONST BINFORMAT_API *
OmfGetApi (
  VOID
  );

#endif // __LIBOMF_H__
