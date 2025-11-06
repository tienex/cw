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
  - MMIX MMO (MMIX Object format)
  - NetWare NLM (NetWare Loadable Module)

  Note: Excludes formats already provided by other libraries (ELF, COFF,
  a.out, Mach-O, OMF variants).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __LIBORF_H__
#define __LIBORF_H__

#include "../../include/binformat/BinFormat.h"

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
/// MMIX MMO (MMIX Object format) magic
/// Used by the MMIX architecture designed by Donald Knuth
///
#define MMO_MAGIC             0x98080901  ///< MMO magic (MMIX object)
#define MMO_VERSION           1           ///< Current MMO version

///
/// MMIX MMO location (lop) codes
///
#define MMO_LOP_QUOTE         0x98000001  ///< Quote next tetrabyte literally
#define MMO_LOP_LOC           0x98000002  ///< Set location
#define MMO_LOP_SKIP          0x98000003  ///< Skip forward
#define MMO_LOP_FIXO          0x98000004  ///< Fixup forward
#define MMO_LOP_FIXR          0x98000005  ///< Fixup backward
#define MMO_LOP_FIXRX         0x98000006  ///< Fixup backward relaxed
#define MMO_LOP_FILE          0x98000007  ///< File name follows
#define MMO_LOP_LINE          0x98000008  ///< Line number follows
#define MMO_LOP_SPEC          0x98000009  ///< Special register
#define MMO_LOP_PRE           0x980A0000  ///< Preamble
#define MMO_LOP_POST          0x98100000  ///< Postamble
#define MMO_LOP_STAB          0x980B0000  ///< Symbol table

///
/// NetWare Loadable Module (NLM) magic
/// Used by Novell NetWare operating system
///
#define NLM_SIGNATURE         0x0000564C4D  ///< "VLM" signature (preceded by version)
#define NLM_VERSION_1         0x00        ///< Original NLM format
#define NLM_VERSION_ADVANCED  0x01        ///< Advanced NLM format

///
/// NLM module types
///
#define NLM_TYPE_LAN_DRIVER   0x00        ///< LAN driver
#define NLM_TYPE_DISK_DRIVER  0x01        ///< Disk driver
#define NLM_TYPE_NAME_SPACE   0x02        ///< Name space support
#define NLM_TYPE_UTILITY      0x03        ///< Utility module
#define NLM_TYPE_MSL          0x04        ///< Mirrored server link
#define NLM_TYPE_OS           0x05        ///< Operating system module
#define NLM_TYPE_PAGED        0x06        ///< Paged module
#define NLM_TYPE_HAM          0x07        ///< Hardware abstraction module
#define NLM_TYPE_CDM          0x08        ///< Custom device module
#define NLM_TYPE_FILE_SYSTEM  0x09        ///< File system module
#define NLM_TYPE_REAL_MODE    0x0A        ///< Real-mode module
#define NLM_TYPE_HIDDEN       0x0B        ///< Hidden module

///
/// NLM CPU types
///
#define NLM_CPU_I386          0x00        ///< Intel 386 and above
#define NLM_CPU_I486          0x01        ///< Intel 486 and above
#define NLM_CPU_PENTIUM       0x02        ///< Intel Pentium

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
