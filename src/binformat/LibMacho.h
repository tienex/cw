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

#include "../../include/binformat/BinFormat.h"

///
/// Mach-O magic numbers
/// Supported from NeXTSTEP 3.x (1993) through latest macOS (2025+)
///
#define MACHO_MAGIC_32        0xFEEDFACE  ///< 32-bit Mach-O
#define MACHO_MAGIC_64        0xFEEDFACF  ///< 64-bit Mach-O
#define MACHO_MAGIC_32_SWAP   0xCEFAEDFE  ///< 32-bit swapped (big-endian)
#define MACHO_MAGIC_64_SWAP   0xCFFAEDFE  ///< 64-bit swapped (big-endian)
#define MACHO_FAT_MAGIC       0xCAFEBABE  ///< Universal binary (fat)
#define MACHO_FAT_MAGIC_SWAP  0xBEBAFECA  ///< Universal swapped
#define MACHO_FAT_MAGIC_64    0xCAFEBABF  ///< Universal 64-bit (fat64)
#define MACHO_FAT_MAGIC_64_SWAP 0xBFBAFECA ///< Universal 64-bit swapped

///
/// Mach-O CPU types (from NeXTSTEP 3.x to latest macOS)
///
#define MACHO_CPU_TYPE_ANY     ((INT32)-1)
#define MACHO_CPU_TYPE_VAX     1
#define MACHO_CPU_TYPE_ROMP    2        ///< IBM RT (ROMP)
#define MACHO_CPU_TYPE_NS32032 4        ///< NeXTSTEP NS32032
#define MACHO_CPU_TYPE_NS32332 5        ///< NeXTSTEP NS32332
#define MACHO_CPU_TYPE_MC680x0 6        ///< Motorola 68K (NeXTSTEP 3.x)
#define MACHO_CPU_TYPE_X86     7        ///< Intel x86 (i386)
#define MACHO_CPU_TYPE_I386    MACHO_CPU_TYPE_X86
#define MACHO_CPU_TYPE_X86_64  (MACHO_CPU_TYPE_X86 | MACHO_CPU_ARCH_ABI64)
#define MACHO_CPU_TYPE_MIPS    8        ///< MIPS (never shipped)
#define MACHO_CPU_TYPE_NS32532 9        ///< NeXTSTEP NS32532
#define MACHO_CPU_TYPE_MC98000 10       ///< Motorola 98000 (never shipped)
#define MACHO_CPU_TYPE_HPPA    11       ///< HP PA-RISC (Rhapsody/DR1)
#define MACHO_CPU_TYPE_ARM     12       ///< ARM (iPhone OS 1.x - iOS 10.x)
#define MACHO_CPU_TYPE_ARM64   (MACHO_CPU_TYPE_ARM | MACHO_CPU_ARCH_ABI64)
#define MACHO_CPU_TYPE_ARM64_32 0x0200000C ///< ARM64_32 (watchOS)
#define MACHO_CPU_TYPE_MC88000 13       ///< Motorola 88000 (NeXTSTEP 3.x)
#define MACHO_CPU_TYPE_SPARC   14       ///< SPARC (never shipped)
#define MACHO_CPU_TYPE_I860    15       ///< Intel i860 (never shipped)
#define MACHO_CPU_TYPE_ALPHA   16       ///< DEC Alpha (Rhapsody/DR2)
#define MACHO_CPU_TYPE_RS6000  17       ///< IBM RS/6000 (never shipped)
#define MACHO_CPU_TYPE_POWERPC 18       ///< PowerPC (Mac OS X 10.0-10.5)
#define MACHO_CPU_TYPE_POWERPC64 (MACHO_CPU_TYPE_POWERPC | MACHO_CPU_ARCH_ABI64)

///
/// CPU architecture masks
///
#define MACHO_CPU_ARCH_MASK    0xFF000000
#define MACHO_CPU_ARCH_ABI64   0x01000000  ///< 64-bit ABI

///
/// Mach-O file types
///
#define MACHO_FILETYPE_OBJECT       0x1  ///< Relocatable object file
#define MACHO_FILETYPE_EXECUTE      0x2  ///< Executable
#define MACHO_FILETYPE_FVMLIB       0x3  ///< Fixed VM library
#define MACHO_FILETYPE_CORE         0x4  ///< Core dump
#define MACHO_FILETYPE_PRELOAD      0x5  ///< Preloaded executable
#define MACHO_FILETYPE_DYLIB        0x6  ///< Dynamic library
#define MACHO_FILETYPE_DYLINKER     0x7  ///< Dynamic linker
#define MACHO_FILETYPE_BUNDLE       0x8  ///< Bundle
#define MACHO_FILETYPE_DYLIB_STUB   0x9  ///< Stub library
#define MACHO_FILETYPE_DSYM         0xA  ///< Debug symbols (dSYM)
#define MACHO_FILETYPE_KEXT_BUNDLE  0xB  ///< Kernel extension

///
/// Mach-O CPU subtypes - x86
///
#define MACHO_CPU_SUBTYPE_X86_ALL    3
#define MACHO_CPU_SUBTYPE_386        3
#define MACHO_CPU_SUBTYPE_486        4
#define MACHO_CPU_SUBTYPE_486SX      0x84
#define MACHO_CPU_SUBTYPE_586        5
#define MACHO_CPU_SUBTYPE_PENT       MACHO_CPU_SUBTYPE_586
#define MACHO_CPU_SUBTYPE_PENTPRO    0x16
#define MACHO_CPU_SUBTYPE_PENTII_M3  0x36
#define MACHO_CPU_SUBTYPE_PENTII_M5  0x56
#define MACHO_CPU_SUBTYPE_CELERON    0x67
#define MACHO_CPU_SUBTYPE_CELERON_MOBILE 0x77
#define MACHO_CPU_SUBTYPE_PENTIUM_3  0x08
#define MACHO_CPU_SUBTYPE_PENTIUM_3_M 0x18
#define MACHO_CPU_SUBTYPE_PENTIUM_3_XEON 0x28
#define MACHO_CPU_SUBTYPE_PENTIUM_M  0x09
#define MACHO_CPU_SUBTYPE_PENTIUM_4  0x0A
#define MACHO_CPU_SUBTYPE_PENTIUM_4_M 0x1A
#define MACHO_CPU_SUBTYPE_ITANIUM    0x0B
#define MACHO_CPU_SUBTYPE_ITANIUM_2  0x1B
#define MACHO_CPU_SUBTYPE_XEON       0x0C
#define MACHO_CPU_SUBTYPE_XEON_MP    0x1C

///
/// Mach-O CPU subtypes - x86_64
///
#define MACHO_CPU_SUBTYPE_X86_64_ALL 3
#define MACHO_CPU_SUBTYPE_X86_64_H   8   ///< Haswell and later

///
/// Mach-O CPU subtypes - ARM
///
#define MACHO_CPU_SUBTYPE_ARM_ALL    0
#define MACHO_CPU_SUBTYPE_ARM_V4T    5
#define MACHO_CPU_SUBTYPE_ARM_V6     6
#define MACHO_CPU_SUBTYPE_ARM_V5TEJ  7
#define MACHO_CPU_SUBTYPE_ARM_XSCALE 8
#define MACHO_CPU_SUBTYPE_ARM_V7     9
#define MACHO_CPU_SUBTYPE_ARM_V7F    10  ///< Cortex A9 (Swift)
#define MACHO_CPU_SUBTYPE_ARM_V7S    11  ///< Swift
#define MACHO_CPU_SUBTYPE_ARM_V7K    12  ///< Cortex A7 (Apple Watch)
#define MACHO_CPU_SUBTYPE_ARM_V6M    14  ///< Cortex-M0
#define MACHO_CPU_SUBTYPE_ARM_V7M    15  ///< Cortex-M3
#define MACHO_CPU_SUBTYPE_ARM_V7EM   16  ///< Cortex-M4

///
/// Mach-O CPU subtypes - ARM64
///
#define MACHO_CPU_SUBTYPE_ARM64_ALL  0
#define MACHO_CPU_SUBTYPE_ARM64_V8   1
#define MACHO_CPU_SUBTYPE_ARM64E     2   ///< Pointer authentication

///
/// Mach-O CPU subtypes - ARM64_32 (watchOS)
///
#define MACHO_CPU_SUBTYPE_ARM64_32_V8 1

///
/// Mach-O CPU subtypes - PowerPC (NeXTSTEP/Rhapsody/Mac OS X 10.0-10.5)
///
#define MACHO_CPU_SUBTYPE_POWERPC_ALL 0
#define MACHO_CPU_SUBTYPE_POWERPC_601 1
#define MACHO_CPU_SUBTYPE_POWERPC_602 2
#define MACHO_CPU_SUBTYPE_POWERPC_603 3
#define MACHO_CPU_SUBTYPE_POWERPC_603e 4
#define MACHO_CPU_SUBTYPE_POWERPC_603ev 5
#define MACHO_CPU_SUBTYPE_POWERPC_604 6
#define MACHO_CPU_SUBTYPE_POWERPC_604e 7
#define MACHO_CPU_SUBTYPE_POWERPC_620 8
#define MACHO_CPU_SUBTYPE_POWERPC_750 9
#define MACHO_CPU_SUBTYPE_POWERPC_7400 10
#define MACHO_CPU_SUBTYPE_POWERPC_7450 11
#define MACHO_CPU_SUBTYPE_POWERPC_970  100

///
/// Mach-O CPU subtypes - 68K (NeXTSTEP 3.x)
///
#define MACHO_CPU_SUBTYPE_MC680x0_ALL 1
#define MACHO_CPU_SUBTYPE_MC68030     1
#define MACHO_CPU_SUBTYPE_MC68040     2
#define MACHO_CPU_SUBTYPE_MC68030_ONLY 3

///
/// Mach-O CPU subtypes - 88K (NeXTSTEP 3.x)
///
#define MACHO_CPU_SUBTYPE_MC88000_ALL 0
#define MACHO_CPU_SUBTYPE_MC88100     1
#define MACHO_CPU_SUBTYPE_MC88110     2

///
/// Mach-O CPU subtypes - HPPA (Rhapsody DR1)
///
#define MACHO_CPU_SUBTYPE_HPPA_ALL   0
#define MACHO_CPU_SUBTYPE_HPPA_7100LC 1

///
/// OSF/1 ROSE (Relocatable Object Specific Extensions) for Alpha
/// This is a separate Mach-O variant used on DEC Alpha systems running OSF/1
/// (later Digital UNIX/Tru64 UNIX). Not compatible with NeXT/Apple Mach-O.
///
#define MACHO_ROSE_MAGIC          0xFACE0FF1  ///< OSF/1 ROSE object file
#define MACHO_ROSE_MAGIC_SWAP     0xF10FFACE  ///< OSF/1 ROSE swapped
#define MACHO_CPU_TYPE_ALPHA_OSF1 16          ///< Alpha for OSF/1 ROSE

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
