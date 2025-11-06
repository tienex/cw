/** @file
  Obscure and Rare Binary Format Library.

  This library provides support for parsing and creating obscure and rare
  binary formats from historical and specialized systems. It implements the
  unified BINFORMAT_API interface.

  Supported formats:
  - Classic macOS PEF (Preferred Executable Format)
  - Classic macOS CODE resources
  - Amiga HUNK and OBJ formats
  - Atari TOS PRG and IBJ formats
  - OpenVMS/VMS EXE and OBJ formats
  - PDP-10 SAV format
  - HP SOM32/64 (System Object Module)
  - Acorn RISC OS executable and object formats
  - EPOC32/Symbian OBJ and EXE formats

  Note: Excludes formats already provided by other libraries (ELF, COFF,
  a.out, Mach-O, OMF variants).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBORF_H__
#define __LIBORF_H__

#include "BinFormat.h"

///
/// PEF (Preferred Executable Format) magic
///
#define PEF_MAGIC_JOY         0x4A6F7921  ///< "Joy!" - PEF container
#define PEF_MAGIC_CFM         0x43464D31  ///< CFM-68K format

///
/// Classic Mac CODE resource
///
#define MAC_CODE_RESOURCE    0x434F4445  ///< "CODE"

///
/// Amiga HUNK magic
///
#define HUNK_MAGIC            0x000003F3  ///< Amiga HUNK format
#define HUNK_UNIT             0x000003E7  ///< Unit hunk
#define HUNK_NAME             0x000003E8  ///< Name hunk
#define HUNK_CODE             0x000003E9  ///< Code hunk
#define HUNK_DATA             0x000003EA  ///< Data hunk
#define HUNK_BSS              0x000003EB  ///< BSS hunk

///
/// Atari TOS PRG magic
///
#define TOS_PRG_MAGIC         0x601A      ///< Atari TOS program

///
/// OpenVMS EXE magic
///
#define VMS_EXE_MAGIC         0x0102      ///< VMS executable
#define VMS_OBJ_MAGIC         0x0002      ///< VMS object module

///
/// PDP-10 SAV magic
///
#define PDP10_SAV_MAGIC       0137        ///< PDP-10 save file

///
/// HP SOM (System Object Module) magic
///
#define SOM_MAGIC_EXEC        0x0107      ///< SOM executable
#define SOM_MAGIC_RELOC       0x0106      ///< SOM relocatable
#define SOM_MAGIC_DEMAND      0x010B      ///< SOM demand load
#define SOM_MAGIC_SHARED      0x010D      ///< SOM shared library
#define SOM_MAGIC_DL          0x010E      ///< SOM dynamic load

///
/// Acorn RISC OS magic numbers
///
#define ACORN_AIF_MAGIC       0xE1A00000  ///< AIF executable
#define ACORN_AOF_MAGIC       0xC5E2D080  ///< AOF object format

///
/// EPOC32/Symbian magic
///
#define EPOC_UID1             0x10000079  ///< EPOC executable UID1
#define EPOC_UID_E32          0x1000007A  ///< E32 image UID

///
/// Get the ORF (Obscure/Rare Formats) library API table.
///
/// @return Pointer to ORF library API table.
///
EXTERN CONST BINFORMAT_API *
OrfGetApi (
  VOID
  );

#endif // __LIBORF_H__
