# LibAout - a.out Binary Format Library

## Overview

LibAout provides comprehensive support for parsing and creating a.out (assembler output) binaries and their many variants. The a.out format was the original Unix executable format from the 1970s through the 1990s, preceding ELF on most Unix systems.

## Supported Formats

### Core a.out Variants
- **OMAGIC** (0407): Original impure format (text and data not separated, overlay support)
- **NMAGIC** (0410): Pure executable (read-only text, separate instruction and data space)
- **ZMAGIC** (0411/0413): Demand-paged executable (most common, memory-mapped)
- **QMAGIC** (0314): Compact demand-paged format (4KB aligned)

### 64-bit Extensions
- **aout64**: 64-bit a.out variants for MIPS, Alpha, and SPARC64

### Special Formats
- **bout** (Binary Output): Intel i960 and embedded systems format
  - **BOUT_MAGIC_I960** (0x10C): Intel i960 bout
  - **BOUT_MAGIC_B_OUT** (0x10D): Generic bout

## Operating System Variants (20+ systems)

### BSD Families

#### 4.3BSD and Derivatives
- **4.3BSD** (1986): Original BSD a.out
- **FreeBSD** (1993-present): FreeBSD-specific extensions and flags
- **OpenBSD** (1995-present): Security-focused variant
- **NetBSD** (1993-present): Portable variant across 50+ architectures
- **BSDI** (1991-1999): BSD/OS commercial variant

#### SunOS/Solaris
- **SunOS 4.x** (1987-1994): Sun Microsystems Unix
  - Big-endian (SPARC) and little-endian (x86) variants
  - Extended format with additional fields
  - Machine types: M_68010, M_68020, M_SPARC, M_386

### Commercial Unix Variants

#### AT&T System V Derivatives
- **SVR3 COFF-like a.out**: System V Release 3
- **SCO Unix**: Santa Cruz Operation Unix

#### Vendor-Specific Unix
- **HP-UX** (1986-present): Hewlett-Packard Unix
  - **HPUX_EXEC_MAGIC** (0x0107): Executable
  - **HPUX_RELOC_MAGIC** (0x0106): Relocatable
  - **HPUX_DEMAND_MAGIC** (0x010B): Demand paged
  - **HPUX_SHARE_MAGIC** (0x010C): Shared library
  - **HPUX_DL_MAGIC** (0x010D): Dynamic load
- **Ultrix** (1984-1995): DEC Unix for VAX and MIPS
  - VAX Ultrix: Standard OMAGIC/NMAGIC/ZMAGIC
  - MIPS Ultrix: Big-endian and little-endian variants
- **AIX**: IBM Unix (early versions, before XCOFF)
- **IRIX**: SGI Unix (early versions, before ELF)
- **Tru64/Digital UNIX**: Compaq/Digital Unix for Alpha
- **OSF/1** (1992-1999): Open Software Foundation Unix

### Historical Workstation Unix
- **Apollo Domain/OS** (1981-1989): Apollo Computer
  - **APOLLO_MAGIC** (0x0108): Apollo Domain/OS
  - **APOLLO_M68K_MAGIC** (0x010B): Apollo M68K
- **Pyramid Technology** (1980s): Pyramid 90x series
  - **PYRAMID_MAGIC** (0x010F)
- **Sequent Balance/Symmetry** (1980s-1990s): Multiprocessor systems
  - **SEQUENT_MAGIC** (0x010E)
- **Convex** (1985-1995): Supercomputers and minisupercomputers
- **Alliant FX** (1980s): Parallel processing systems
  - **ALLIANT_MAGIC** (0x010A)
- **Gould** (1970s-1980s): Gould PN and NP series
  - **GOULD_MAGIC** (0x0109)
- **Encore Multimax** (1985-1990): Parallel processing

### Minicomputer and Embedded Systems
- **PDP-11** (1970-1997): DEC PDP-11 Unix V6/V7
  - **PDP11_OMAGIC** (0407): Old impure
  - **PDP11_NMAGIC** (0410): Pure (separate I&D)
  - **PDP11_ZMAGIC** (0411): Demand paged (split I&D)
- **Atari TOS/MiNT** (1985-present): Atari ST/TT/Falcon
  - **ATARI_AOUT_MAGIC** (0x601A): TOS executable
  - **ATARI_GEMDOS_MAGIC** (0x601B): GEMDOS executable
- **BeOS** (1995-2001): BeBox and x86
  - **BEOS_MAGIC** (0xBEB): Legacy BeOS executables
- **QNX** (1982-present): Real-time OS
  - OMAGIC/NMAGIC/ZMAGIC variants
- **ARM platforms** (various): Early ARM Unix systems
- **Clipper** (1986-1989): Fairchild/Intergraph Clipper
  - **CLIPPER_MAGIC** (0x0111)

## Architecture Support (25+ architectures)

### Production Architectures
- **x86 (i386)**: Intel 386, 486, Pentium series
  - Machine type: **AOUT_M_386** (100)
- **68K family**: Motorola 68010, 68020, 68030, 68040
  - Machine types: **AOUT_M_68010** (1), **AOUT_M_68020** (2)
  - Extended magics: M68K_MAGIC_010 through M68K_MAGIC_040
- **SPARC**: SPARC v7, v8, v9 (32-bit and 64-bit)
  - Machine type: **AOUT_M_SPARC** (3), **AOUT_M_SPARC64** (200)
- **MIPS**: R2000, R3000, R4000, R6000, R8000
  - Machine types: **AOUT_M_MIPS1** (151), **AOUT_M_MIPS2** (152)
  - Big-endian: MIPS_OMAGIC_BE, MIPS_NMAGIC_BE, MIPS_ZMAGIC_BE
  - Little-endian: MIPS_OMAGIC_LE, MIPS_NMAGIC_LE, MIPS_ZMAGIC_LE
- **ARM**: ARM2 through ARM11 (32-bit)
  - Machine type: **AOUT_M_ARM** (165)
- **PowerPC**: PowerPC 601-970 (32-bit and 64-bit)
  - Machine type: **AOUT_M_PPC** (175)
- **Alpha**: DEC Alpha AXP (64-bit)
  - Machine type: **AOUT_M_ALPHA** (180)
  - NetBSD/Alpha a.out support
- **SuperH**: SH3, SH4 (Hitachi/Renesas)
  - Machine type: **AOUT_M_SH3** (166)
  - Magics: SH3_MAGIC (0x0103), SH4_MAGIC (0x0104)

### Historical Architectures
- **VAX**: DEC VAX (1977-1997)
  - Machine type: **AOUT_M_VAX** (185)
  - VAX_OMAGIC, VAX_NMAGIC, VAX_ZMAGIC
- **NS32K**: National Semiconductor 32016/32032/32532
  - Machine type: **AOUT_M_NS32K** (190)
  - Magics: NS32K_MAGIC_032 (0x010A), NS32K_MAGIC_532 (0x0185)
- **WE32100**: AT&T WE 32100 (1984-1990s)
  - Machine type: **AOUT_M_WE32K** (195)
  - Magic: WE32K_MAGIC (0x0170)
- **AMD 29000**: AMD 29K RISC processor (1988-1995)
  - Machine type: **AOUT_M_29K** (160)
- **PDP-11**: DEC PDP-11 (1970-1997)
  - 16-bit words, separate I&D space support

### Embedded and Special Purpose
- **Intel i960**: RISC embedded processor
  - Used with bout format
- **68HC11/68HC12**: Motorola/Freescale microcontrollers
- **M88K**: Motorola 88000 RISC processor

## Relocation Types (50+ types)

### Generic Relocations
- **AOUT_RELOC_8** (0): 8-bit absolute
- **AOUT_RELOC_16** (1): 16-bit absolute
- **AOUT_RELOC_32** (2): 32-bit absolute
- **AOUT_RELOC_DISP8** (3): 8-bit PC-relative
- **AOUT_RELOC_DISP16** (4): 16-bit PC-relative
- **AOUT_RELOC_DISP32** (5): 32-bit PC-relative

### SPARC Relocations
- **AOUT_RELOC_WDISP30** (6): 30-bit word displacement
- **AOUT_RELOC_WDISP22** (7): 22-bit word displacement
- **AOUT_RELOC_HI22** (8): High 22 bits
- **AOUT_RELOC_22** (9): Direct 22 bits
- **AOUT_RELOC_13** (10): Direct 13 bits
- **AOUT_RELOC_LO10** (11): Low 10 bits
- **AOUT_RELOC_SFA_BASE** (12): SFA base
- **AOUT_RELOC_SFA_OFF13** (13): SFA offset 13 bits
- **AOUT_SPARC_RELOC_GOT10** (28): GOT 10-bit
- **AOUT_SPARC_RELOC_GOT13** (29): GOT 13-bit
- **AOUT_SPARC_RELOC_GOT22** (30): GOT 22-bit
- **AOUT_SPARC_RELOC_PC10** (31): PC-relative 10-bit
- **AOUT_SPARC_RELOC_PC22** (32): PC-relative 22-bit
- **AOUT_SPARC_RELOC_WPLT30** (33): WPLT 30-bit

### Dynamic Linking Relocations
- **AOUT_RELOC_JMP_TBL** (19): Jump table (PLT)
- **AOUT_RELOC_SEGOFF16** (20): 16-bit segment offset
- **AOUT_RELOC_GLOB_DAT** (21): Global data (GOT)
- **AOUT_RELOC_JMP_SLOT** (22): Jump slot (PLT entry)
- **AOUT_RELOC_RELATIVE** (23): Relative to load address

### MIPS Relocations
- **AOUT_MIPS_RELOC_LITERAL** (24): Literal pool entry
- **AOUT_MIPS_RELOC_LITUSE** (25): Literal use
- **AOUT_MIPS_RELOC_GPDISP** (26): GP displacement
- **AOUT_MIPS_RELOC_GPVALUE** (27): GP value

### 68K Relocations
- **AOUT_68K_RELOC_GOTOFF16** (34): GOT offset 16-bit
- **AOUT_68K_RELOC_GOTOFF32** (35): GOT offset 32-bit
- **AOUT_68K_RELOC_PLT16** (36): PLT 16-bit
- **AOUT_68K_RELOC_PLT32** (37): PLT 32-bit

### x86 Relocations
- **AOUT_386_RELOC_GOTPC** (38): GOT PC-relative
- **AOUT_386_RELOC_GOTOFF** (39): GOT offset
- **AOUT_386_RELOC_PLT32** (40): PLT 32-bit

### ARM Relocations
- **AOUT_ARM_RELOC_PC24** (41): PC-relative 24-bit
- **AOUT_ARM_RELOC_GOT32** (42): GOT 32-bit
- **AOUT_ARM_RELOC_PLT32** (43): PLT 32-bit
- **AOUT_ARM_RELOC_GOTOFF** (44): GOT offset

## Symbol Types

- **AOUT_N_UNDF** (0x00): Undefined symbol
- **AOUT_N_ABS** (0x02): Absolute symbol
- **AOUT_N_TEXT** (0x04): Text segment symbol
- **AOUT_N_DATA** (0x06): Data segment symbol
- **AOUT_N_BSS** (0x08): BSS segment symbol
- **AOUT_N_COMM** (0x12): Common symbol (uninitialized global)
- **AOUT_N_FN** (0x1E): File name symbol
- **AOUT_N_EXT** (0x01): External symbol flag
- **AOUT_N_TYPE** (0x1E): Type mask
- **AOUT_N_STAB** (0xE0): Debug symbol (stabs) flag

## Dynamic Linking

### Dynamic Section Entries
- **AOUT_DT_NULL** (0): End of dynamic section
- **AOUT_DT_NEEDED** (1): Needed shared library
- **AOUT_DT_PLTRELSZ** (2): PLT relocation size
- **AOUT_DT_PLTGOT** (3): PLT/GOT address
- **AOUT_DT_HASH** (4): Symbol hash table
- **AOUT_DT_STRTAB** (5): String table
- **AOUT_DT_SYMTAB** (6): Symbol table
- **AOUT_DT_RELA** (7): Relocation table (with addend)
- **AOUT_DT_RELASZ** (8): Relocation size
- **AOUT_DT_RELAENT** (9): Relocation entry size
- **AOUT_DT_STRSZ** (10): String table size
- **AOUT_DT_SYMENT** (11): Symbol entry size
- **AOUT_DT_INIT** (12): Initialization function
- **AOUT_DT_FINI** (13): Termination function

### Section Flags
- **AOUT_SF_ALLOC** (0x01): Section occupies memory
- **AOUT_SF_LOAD** (0x02): Section loaded at runtime
- **AOUT_SF_RELOC** (0x04): Section has relocations
- **AOUT_SF_EXEC** (0x08): Section is executable
- **AOUT_SF_WRITE** (0x10): Section is writable
- **AOUT_SF_SHARED** (0x20): Section is shared

## bout Format Details

### bout Magic Numbers
- **BOUT_MAGIC_I960** (0x10C): Intel i960
- **BOUT_MAGIC_B_OUT** (0x10D): Generic bout

### bout Versions
- **BOUT_VERSION_1** (1): Original bout
- **BOUT_VERSION_2** (2): Enhanced bout

### bout Flags
- **BOUT_FLAG_RELOC** (0x01): Contains relocations
- **BOUT_FLAG_EXEC** (0x02): Executable
- **BOUT_FLAG_SYMS** (0x04): Contains symbol table

## Sources and References

### Historical Unix Documentation
- **Unix Programmer's Manual** (1971): First Edition Unix
- **Unix V6 documentation** (1975): PDP-11 a.out format
- **Unix V7 documentation** (1979): Standardized a.out
- **4.3BSD documentation** (1986): BSD a.out extensions
- **SunOS 4.x documentation** (1987-1994): Sun a.out extensions

### Operating System Sources
- **FreeBSD** (sys/exec/imgact_aout.c, sys/sys/imgact_aout.h)
  - Repository: https://github.com/freebsd/freebsd-src
- **NetBSD** (sys/sys/exec_aout.h, sys/kern/exec_aout.c)
  - Repository: https://github.com/NetBSD/src
- **OpenBSD** (sys/sys/exec_aout.h)
  - Repository: https://github.com/openbsd/src
- **Linux kernel** (include/linux/a.out.h, binfmt_aout.c) - deprecated
  - Last supported: Linux 5.1 (2019)
  - Repository: https://kernel.org/

### Toolchain Sources
- **GNU binutils** (include/aout/*.h, bfd/aoutx.h)
  - a.out64.h: 64-bit a.out variants
  - sun4.h: SunOS 4.x format
  - hp300hpux.h: HP-UX 68K format
  - Repository: https://sourceware.org/git/binutils-gdb.git
- **gdb** (gdb/*/tm-*.h): Target machine definitions
  - Historical a.out debugging support
- **NetBSD toolchain** (src/sys/sys/exec_aout.h)
  - Most comprehensive modern a.out implementation

### Historical Vendor Documentation
- **Sun Microsystems**: SunOS 4.x linker and libraries guide
- **DEC**: Ultrix documentation for VAX and MIPS
- **HP**: HP-UX linker and libraries reference
- **IBM**: AIX early versions (pre-XCOFF)
- **SGI**: IRIX 3.x and 4.x documentation
- **Apollo**: Domain/OS technical reference
- **Motorola**: 68K Unix documentation

### Academic Sources
- **Lions' Commentary on UNIX 6th Edition** (1977): PDP-11 a.out internals
- **The Design of the UNIX Operating System** by Maurice Bach (1986)
- **Advanced Programming in the UNIX Environment** by W. Richard Stevens (1992)

### Books
- *Linkers and Loaders* by John R. Levine (1999): Chapter on a.out format
- *The Magic Garden Explained* by Berny Goodheart and James Cox (1994): System V internals

## Feature Parity with LibElf

LibAout provides equivalent functionality to LibElf:

| Feature | LibElf | LibAout |
|---------|--------|---------|
| Multiple format variants | ELF32, ELF64, x32, FatELF | OMAGIC, NMAGIC, ZMAGIC, QMAGIC, aout64, bout |
| OS variants | 18 OS/ABI types | 20+ Unix systems |
| Architecture support | 100+ machine types | 25+ architectures |
| Relocation types | 800+ across 18 architectures | 50+ generic and arch-specific |
| Dynamic linking | Full DT_* support (60+ tags) | Full AOUT_DT_* support (13 tags) |
| Symbol types | Complete | Complete (8 types + flags) |
| Machine-specific extensions | Yes (per-architecture flags) | Yes (per-system magic numbers) |

## Version History

- **v1.0** (2025): Initial comprehensive implementation
  - Support for 20+ operating systems
  - 25+ architectures
  - Complete relocation support
  - Dynamic linking support
  - Historical variants (PDP-11, VAX, Apollo, etc.)
  - bout format support

## Migration Notes

### From a.out to ELF
Most systems migrated from a.out to ELF between 1995-2005:
- **Linux**: 1995 (kernel 1.2) - ELF became default
- **FreeBSD**: 1998 (3.0) - switched to ELF
- **NetBSD**: 1999 (1.5) - ELF became standard
- **Solaris**: Never used a.out (used COFF, then ELF)
- **IRIX**: 1994 (5.3) - switched to ELF

### Why a.out was replaced
- No shared library versioning
- Limited section types
- Fixed segment layout
- No TLS (thread-local storage) support
- Difficult to extend

### When to use LibAout
- Analyzing historical Unix binaries
- Retro computing and emulation
- Forensics and archaeology
- Supporting legacy systems
- Understanding Unix evolution

## License

MIT License - See SPDX-License-Identifier in source files.
