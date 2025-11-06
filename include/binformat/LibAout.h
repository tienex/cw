/** @file
  a.out Binary Format Library.

  This library provides support for parsing and creating a.out (Assembler Output)
  binaries and its variants including 32-bit, 64-bit, and system-specific formats.
  It implements the unified BINFORMAT_API interface.

  Supported formats:
  - a.out (traditional Unix - OMAGIC, NMAGIC, ZMAGIC, QMAGIC)
  - aout64 (64-bit a.out for early 64-bit systems)
  - PDP-11 a.out (original Unix V6/V7)
  - b.out (early Unix binary output format)
  - BSD variants (FreeBSD, OpenBSD, NetBSD, BSDI)
  - SunOS a.out (with endianness variants)
  - Linux a.out
  - Plan 9 a.out
  - bout (Binary Output for Intel i960 and embedded systems)
  - HP-UX, Ultrix, VAX, MIPS variants
  - Encore, Apollo, Pyramid, Sequent systems
  - ARM, Atari TOS, BeOS, QNX variants

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
/// PDP-11 a.out Magic Numbers
///
/// The original Unix a.out format from Version 6/7 Unix on PDP-11.
/// These are 16-bit values in PDP-11 byte order.
///
#define PDP11_OMAGIC         0407       ///< Old impure format (overlay)
#define PDP11_NMAGIC         0410       ///< Pure executable (separate I&D)
#define PDP11_ZMAGIC         0411       ///< Demand paged (split I&D)
#define PDP11_IMAGIC         0405       ///< Separate I&D (no relocation)
#define PDP11_RMAGIC         0430       ///< Relocatable object
#define PDP11_AMAGIC         0407       ///< Absolute (non-relocatable)

///
/// PDP-11 Overlay Magic
///
#define PDP11_OVERLAY_MAGIC  0430       ///< Overlay structure

///
/// 64-bit a.out Magic Numbers
///
/// Used on early 64-bit Unix systems (pre-ELF).
///
#define AOUT64_OMAGIC        0407       ///< Old impure 64-bit format
#define AOUT64_NMAGIC        0410       ///< Pure 64-bit executable
#define AOUT64_ZMAGIC        0413       ///< Demand paged 64-bit
#define AOUT64_QMAGIC        0314       ///< Compact 64-bit

///
/// FreeBSD a.out Magic Numbers
///
#define FREEBSD_OMAGIC       0407       ///< Old impure format
#define FREEBSD_NMAGIC       0410       ///< Read-only text
#define FREEBSD_ZMAGIC       0413       ///< Demand paged
#define FREEBSD_QMAGIC       0314       ///< Compact (deprecated)

///
/// FreeBSD a.out Machine Types
///
#define FREEBSD_M_UNKNOWN    0          ///< Unknown
#define FREEBSD_M_68010      1          ///< Motorola 68010
#define FREEBSD_M_68020      2          ///< Motorola 68020
#define FREEBSD_M_SPARC      3          ///< SPARC
#define FREEBSD_M_386        100        ///< Intel 386
#define FREEBSD_M_MIPS       151        ///< MIPS
#define FREEBSD_M_ARM        103        ///< ARM

///
/// OpenBSD a.out Magic Numbers
///
#define OPENBSD_OMAGIC       0407       ///< Old impure format
#define OPENBSD_NMAGIC       0410       ///< Read-only text
#define OPENBSD_ZMAGIC       0413       ///< Demand paged

///
/// OpenBSD a.out Machine Types
///
#define OPENBSD_M_UNKNOWN    0          ///< Unknown
#define OPENBSD_M_68010      1          ///< Motorola 68010
#define OPENBSD_M_68020      2          ///< Motorola 68020
#define OPENBSD_M_SPARC      3          ///< SPARC
#define OPENBSD_M_386        100        ///< Intel 386
#define OPENBSD_M_SPARC64    18         ///< SPARC64
#define OPENBSD_M_ALPHA      17         ///< Alpha
#define OPENBSD_M_VAX        140        ///< VAX

///
/// BSDI (BSD/OS) a.out Magic Numbers
///
#define BSDI_OMAGIC          0407       ///< Old impure format
#define BSDI_NMAGIC          0410       ///< Read-only text
#define BSDI_ZMAGIC          0413       ///< Demand paged

///
/// SunOS a.out Extended Formats
///
#define SUNOS_OMAGIC_BE      0x0107     ///< Big-endian old impure
#define SUNOS_OMAGIC_LE      0x0701     ///< Little-endian old impure
#define SUNOS_NMAGIC_BE      0x0108     ///< Big-endian pure
#define SUNOS_NMAGIC_LE      0x0801     ///< Little-endian pure
#define SUNOS_ZMAGIC_BE      0x010B     ///< Big-endian demand paged
#define SUNOS_ZMAGIC_LE      0x0B01     ///< Little-endian demand paged

///
/// SunOS Machine Types
///
#define SUNOS_M_UNKNOWN      0          ///< Unknown
#define SUNOS_M_68010        1          ///< Sun 68010
#define SUNOS_M_68020        2          ///< Sun 68020
#define SUNOS_M_SPARC        3          ///< SPARC
#define SUNOS_M_386          100        ///< Intel 386

///
/// HP-UX a.out Magic Numbers
///
#define HPUX_EXEC_MAGIC      0x0107     ///< HP-UX executable
#define HPUX_RELOC_MAGIC     0x0106     ///< HP-UX relocatable
#define HPUX_DEMAND_MAGIC    0x010B     ///< HP-UX demand load
#define HPUX_SHARE_MAGIC     0x010C     ///< HP-UX shared library
#define HPUX_DL_MAGIC        0x010D     ///< HP-UX dynamic load

///
/// Ultrix a.out Magic Numbers
///
#define ULTRIX_OMAGIC        0407       ///< Old impure format
#define ULTRIX_NMAGIC        0410       ///< Pure executable
#define ULTRIX_ZMAGIC        0413       ///< Demand paged

///
/// Encore (Multimax) a.out Magic Numbers
///
#define ENCORE_OMAGIC        0407       ///< Encore old impure
#define ENCORE_NMAGIC        0410       ///< Encore pure
#define ENCORE_ZMAGIC        0413       ///< Encore demand paged

///
/// Apollo a.out Magic Numbers
///
#define APOLLO_MAGIC         0x0108     ///< Apollo Domain/OS
#define APOLLO_M68K_MAGIC    0x010B     ///< Apollo M68K

///
/// Pyramid a.out Magic Numbers
///
#define PYRAMID_MAGIC        0x010F     ///< Pyramid Technology

///
/// Sequent a.out Magic Numbers
///
#define SEQUENT_MAGIC        0x010E     ///< Sequent Balance/Symmetry

///
/// MIPS (Ultrix) a.out Magic Numbers
///
#define MIPS_OMAGIC_BE       0x0107     ///< MIPS old impure (big-endian)
#define MIPS_NMAGIC_BE       0x0108     ///< MIPS pure (big-endian)
#define MIPS_ZMAGIC_BE       0x010B     ///< MIPS demand paged (big-endian)
#define MIPS_OMAGIC_LE       0x0701     ///< MIPS old impure (little-endian)
#define MIPS_NMAGIC_LE       0x0801     ///< MIPS pure (little-endian)
#define MIPS_ZMAGIC_LE       0x0B01     ///< MIPS demand paged (little-endian)

///
/// VAX Ultrix a.out
///
#define VAX_OMAGIC           0407       ///< VAX old impure
#define VAX_NMAGIC           0410       ///< VAX pure
#define VAX_ZMAGIC           0413       ///< VAX demand paged

///
/// bout (Binary Output) Magic Numbers
///
/// Used on Intel 960 and some embedded systems. Similar to a.out
/// but with different segment organization.
///
#define BOUT_MAGIC_I960      0x10C      ///< Intel i960 bout
#define BOUT_MAGIC_B_OUT     0x10D      ///< Generic bout format
#define BOUT_VERSION_1       1          ///< bout version 1
#define BOUT_VERSION_2       2          ///< bout version 2

///
/// bout Flags
///
#define BOUT_FLAG_RELOC      0x01       ///< Contains relocation info
#define BOUT_FLAG_EXEC       0x02       ///< Executable
#define BOUT_FLAG_SYMS       0x04       ///< Contains symbols

///
/// ARM a.out Magic Numbers
///
#define ARM_OMAGIC           0407       ///< ARM old impure
#define ARM_NMAGIC           0410       ///< ARM pure
#define ARM_ZMAGIC           0413       ///< ARM demand paged

///
/// Atari TOS/MiNT a.out
///
#define ATARI_AOUT_MAGIC     0x601A     ///< Atari TOS executable
#define ATARI_GEMDOS_MAGIC   0x601B     ///< Atari GEMDOS executable

///
/// BeOS a.out Magic
///
#define BEOS_MAGIC           0xBEB      ///< BeOS executable (legacy)

///
/// QNX a.out Magic Numbers
///
#define QNX_OMAGIC           0407       ///< QNX old impure
#define QNX_NMAGIC           0410       ///< QNX pure
#define QNX_ZMAGIC           0413       ///< QNX demand paged

///
/// a.out Relocation Types
///
#define AOUT_RELOC_8         0          ///< 8-bit relocation
#define AOUT_RELOC_16        1          ///< 16-bit relocation
#define AOUT_RELOC_32        2          ///< 32-bit relocation
#define AOUT_RELOC_DISP8     3          ///< 8-bit PC-relative
#define AOUT_RELOC_DISP16    4          ///< 16-bit PC-relative
#define AOUT_RELOC_DISP32    5          ///< 32-bit PC-relative
#define AOUT_RELOC_WDISP30   6          ///< SPARC 30-bit word-disp
#define AOUT_RELOC_WDISP22   7          ///< SPARC 22-bit word-disp
#define AOUT_RELOC_HI22      8          ///< SPARC high 22 bits
#define AOUT_RELOC_22        9          ///< SPARC 22 bits
#define AOUT_RELOC_13        10         ///< SPARC 13 bits
#define AOUT_RELOC_LO10      11         ///< SPARC low 10 bits
#define AOUT_RELOC_SFA_BASE  12         ///< SPARC SFA base
#define AOUT_RELOC_SFA_OFF13 13         ///< SPARC SFA offset 13
#define AOUT_RELOC_BASE10    14         ///< Base 10 relocation
#define AOUT_RELOC_BASE13    15         ///< Base 13 relocation
#define AOUT_RELOC_BASE22    16         ///< Base 22 relocation
#define AOUT_RELOC_PC10      17         ///< PC relative 10 bit
#define AOUT_RELOC_PC22      18         ///< PC relative 22 bit
#define AOUT_RELOC_JMP_TBL   19         ///< Jump table relocation
#define AOUT_RELOC_SEGOFF16  20         ///< 16-bit segment offset
#define AOUT_RELOC_GLOB_DAT  21         ///< Global data relocation
#define AOUT_RELOC_JMP_SLOT  22         ///< Jump slot relocation
#define AOUT_RELOC_RELATIVE  23         ///< Relative relocation

///
/// a.out Symbol Types (n_type field)
///
#define AOUT_N_UNDF          0x00       ///< Undefined symbol
#define AOUT_N_ABS           0x02       ///< Absolute symbol
#define AOUT_N_TEXT          0x04       ///< Text segment symbol
#define AOUT_N_DATA          0x06       ///< Data segment symbol
#define AOUT_N_BSS           0x08       ///< BSS segment symbol
#define AOUT_N_COMM          0x12       ///< Common symbol
#define AOUT_N_FN            0x1E       ///< File name symbol
#define AOUT_N_EXT           0x01       ///< External symbol flag
#define AOUT_N_TYPE          0x1E       ///< Type mask
#define AOUT_N_STAB          0xE0       ///< Debug symbol flag

///
/// a.out Dynamic Linking Types
///
#define AOUT_DT_NULL         0          ///< End of dynamic section
#define AOUT_DT_NEEDED       1          ///< Needed library
#define AOUT_DT_PLTRELSZ     2          ///< PLT relocation size
#define AOUT_DT_PLTGOT       3          ///< PLT/GOT
#define AOUT_DT_HASH         4          ///< Symbol hash table
#define AOUT_DT_STRTAB       5          ///< String table
#define AOUT_DT_SYMTAB       6          ///< Symbol table
#define AOUT_DT_RELA         7          ///< Relocation table
#define AOUT_DT_RELASZ       8          ///< Relocation size
#define AOUT_DT_RELAENT      9          ///< Relocation entry size
#define AOUT_DT_STRSZ        10         ///< String table size
#define AOUT_DT_SYMENT       11         ///< Symbol entry size
#define AOUT_DT_INIT         12         ///< Init function address
#define AOUT_DT_FINI         13         ///< Finalization function

///
/// a.out Section Flags
///
#define AOUT_SF_ALLOC        0x01       ///< Section occupies memory
#define AOUT_SF_LOAD         0x02       ///< Section loaded at runtime
#define AOUT_SF_RELOC        0x04       ///< Section has relocations
#define AOUT_SF_EXEC         0x08       ///< Section is executable
#define AOUT_SF_WRITE        0x10       ///< Section is writable
#define AOUT_SF_SHARED       0x20       ///< Section is shared

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
