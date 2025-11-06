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
/// FatELF magic number (multi-architecture ELF container)
///
#define FATELF_MAGIC  0x0FAEF1FA

///
/// ELF class (e_ident[EI_CLASS])
///
#define ELFCLASSNONE  0  ///< Invalid class
#define ELFCLASS32    1  ///< 32-bit objects
#define ELFCLASS64    2  ///< 64-bit objects

///
/// ELF data encoding (e_ident[EI_DATA])
///
#define ELFDATANONE   0  ///< Invalid data encoding
#define ELFDATA2LSB   1  ///< Little-endian
#define ELFDATA2MSB   2  ///< Big-endian

///
/// ELF version (e_ident[EI_VERSION])
///
#define EV_NONE       0  ///< Invalid version
#define EV_CURRENT    1  ///< Current version

///
/// ELF OS/ABI identification (e_ident[EI_OSABI])
///
#define ELFOSABI_NONE       0    ///< System V / generic
#define ELFOSABI_SYSV       0    ///< System V (same as NONE)
#define ELFOSABI_HPUX       1    ///< HP-UX
#define ELFOSABI_NETBSD     2    ///< NetBSD
#define ELFOSABI_GNU        3    ///< GNU/Linux
#define ELFOSABI_LINUX      3    ///< Linux (same as GNU)
#define ELFOSABI_SOLARIS    6    ///< Solaris
#define ELFOSABI_AIX        7    ///< AIX
#define ELFOSABI_IRIX       8    ///< IRIX
#define ELFOSABI_FREEBSD    9    ///< FreeBSD
#define ELFOSABI_TRU64      10   ///< Tru64 UNIX
#define ELFOSABI_MODESTO    11   ///< Novell Modesto
#define ELFOSABI_OPENBSD    12   ///< OpenBSD
#define ELFOSABI_OPENVMS    13   ///< OpenVMS
#define ELFOSABI_NSK        14   ///< HP Non-Stop Kernel
#define ELFOSABI_AROS       15   ///< AROS
#define ELFOSABI_FENIXOS    16   ///< FenixOS
#define ELFOSABI_CLOUDABI   17   ///< CloudABI
#define ELFOSABI_ARM        97   ///< ARM
#define ELFOSABI_STANDALONE 255  ///< Standalone (embedded)

///
/// x32 ABI flag (e_flags for EM_X86_64)
/// x32 uses ILP32 data model (32-bit int, long, pointer) on x86-64
///
#define EF_X86_64_X32     0x00000001  ///< x32 ABI (ILP32 on x86-64)

///
/// ELF object file types (e_type)
///
#define ET_NONE    0       ///< No file type
#define ET_REL     1       ///< Relocatable object
#define ET_EXEC    2       ///< Executable
#define ET_DYN     3       ///< Shared object
#define ET_CORE    4       ///< Core dump
#define ET_LOOS    0xFE00  ///< OS-specific range start
#define ET_HIOS    0xFEFF  ///< OS-specific range end
#define ET_LOPROC  0xFF00  ///< Processor-specific range start
#define ET_HIPROC  0xFFFF  ///< Processor-specific range end

///
/// ELF machine types (e_machine) - comprehensive list
///
#define EM_NONE         0    ///< No machine
#define EM_M32          1    ///< AT&T WE 32100
#define EM_SPARC        2    ///< SPARC
#define EM_386          3    ///< Intel 80386
#define EM_68K          4    ///< Motorola 68000
#define EM_88K          5    ///< Motorola 88000
#define EM_IAMCU        6    ///< Intel MCU
#define EM_860          7    ///< Intel 80860
#define EM_MIPS         8    ///< MIPS RS3000
#define EM_S370         9    ///< IBM System/370
#define EM_MIPS_RS3_LE  10   ///< MIPS RS3000 little-endian
#define EM_PARISC       15   ///< HP PA-RISC
#define EM_VPP500       17   ///< Fujitsu VPP500
#define EM_SPARC32PLUS  18   ///< SPARC v8+
#define EM_960          19   ///< Intel 80960
#define EM_PPC          20   ///< PowerPC 32-bit
#define EM_PPC64        21   ///< PowerPC 64-bit
#define EM_S390         22   ///< IBM S/390
#define EM_SPU          23   ///< Sony/Toshiba/IBM SPU
#define EM_V800         36   ///< NEC V800
#define EM_FR20         37   ///< Fujitsu FR20
#define EM_RH32         38   ///< TRW RH-32
#define EM_RCE          39   ///< Motorola RCE
#define EM_ARM          40   ///< ARM 32-bit
#define EM_ALPHA        41   ///< DEC Alpha
#define EM_SH           42   ///< Hitachi SuperH
#define EM_SPARCV9      43   ///< SPARC v9 64-bit
#define EM_TRICORE      44   ///< Siemens TriCore
#define EM_ARC          45   ///< Argonaut RISC Core
#define EM_H8_300       46   ///< Renesas H8/300
#define EM_H8_300H      47   ///< Renesas H8/300H
#define EM_H8S          48   ///< Renesas H8S
#define EM_H8_500       49   ///< Renesas H8/500
#define EM_IA_64        50   ///< Intel IA-64
#define EM_MIPS_X       51   ///< Stanford MIPS-X
#define EM_COLDFIRE     52   ///< Motorola ColdFire
#define EM_68HC12       53   ///< Motorola 68HC12
#define EM_MMA          54   ///< Fujitsu MMA
#define EM_PCP          55   ///< Siemens PCP
#define EM_NCPU         56   ///< Sony nCPU
#define EM_NDR1         57   ///< Denso NDR1
#define EM_STARCORE     58   ///< Motorola Star*Core
#define EM_ME16         59   ///< Toyota ME16
#define EM_ST100        60   ///< STMicroelectronics ST100
#define EM_TINYJ        61   ///< Advanced Logic TinyJ
#define EM_X86_64       62   ///< AMD/Intel x86-64
#define EM_PDSP         63   ///< Sony DSP
#define EM_PDP10        64   ///< DEC PDP-10
#define EM_PDP11        65   ///< DEC PDP-11
#define EM_FX66         66   ///< Siemens FX66
#define EM_ST9PLUS      67   ///< STMicroelectronics ST9+
#define EM_ST7          68   ///< STMicroelectronics ST7
#define EM_68HC16       69   ///< Motorola 68HC16
#define EM_68HC11       70   ///< Motorola 68HC11
#define EM_68HC08       71   ///< Motorola 68HC08
#define EM_68HC05       72   ///< Motorola 68HC05
#define EM_SVX          73   ///< Silicon Graphics SVx
#define EM_ST19         74   ///< STMicroelectronics ST19
#define EM_VAX          75   ///< DEC VAX
#define EM_CRIS         76   ///< Axis CRIS
#define EM_JAVELIN      77   ///< Infineon Javelin
#define EM_FIREPATH     78   ///< Element 14 Firepath
#define EM_ZSP          79   ///< LSI Logic ZSP
#define EM_MMIX         80   ///< Donald Knuth's MMIX
#define EM_HUANY        81   ///< Harvard HUANY
#define EM_PRISM        82   ///< SiTera Prism
#define EM_AVR          83   ///< Atmel AVR
#define EM_FR30         84   ///< Fujitsu FR30
#define EM_D10V         85   ///< Mitsubishi D10V
#define EM_D30V         86   ///< Mitsubishi D30V
#define EM_V850         87   ///< NEC V850
#define EM_M32R         88   ///< Renesas M32R
#define EM_MN10300      89   ///< Matsushita MN10300
#define EM_MN10200      90   ///< Matsushita MN10200
#define EM_PJ           91   ///< picoJava
#define EM_OPENRISC     92   ///< OpenRISC
#define EM_ARC_COMPACT  93   ///< ARC Cores Tangent-A5
#define EM_XTENSA       94   ///< Tensilica Xtensa
#define EM_VIDEOCORE    95   ///< Alphamosaic VideoCore
#define EM_TMM_GPP      96   ///< Thompson MM GPP
#define EM_NS32K        97   ///< National Semiconductor 32000
#define EM_TPC          98   ///< Tenor TPC
#define EM_SNP1K        99   ///< Trebia SNP 1000
#define EM_ST200        100  ///< STMicroelectronics ST200
#define EM_AARCH64      183  ///< ARM 64-bit (AArch64)
#define EM_RISCV        243  ///< RISC-V
#define EM_BPF          247  ///< Linux BPF
#define EM_CSKY         252  ///< C-SKY
#define EM_LOONGARCH    258  ///< LoongArch

///
/// ELF Note types - Universal format for all known note types
///

//
/// Generic note types (n_type for all vendors)
///
#define NT_VERSION      1    ///< Version information
#define NT_ARCH         2    ///< Architecture

//
/// Core dump note types (PT_NOTE in core files)
///
#define NT_PRSTATUS     1    ///< Process status (prstatus)
#define NT_FPREGSET     2    ///< Floating point registers
#define NT_PRPSINFO     3    ///< Process info (prpsinfo)
#define NT_TASKSTRUCT   4    ///< Task structure (Linux only)
#define NT_AUXV         6    ///< Auxiliary vector
#define NT_SIGINFO      0x53494749  ///< Signal info
#define NT_FILE         0x46494c45  ///< Mapped files
#define NT_PRXFPREG     0x46e62b7f  ///< x86 fxsave format
#define NT_PPC_VMX      0x100        ///< PowerPC Altivec/VMX
#define NT_PPC_SPE      0x101        ///< PowerPC SPE
#define NT_PPC_VSX      0x102        ///< PowerPC VSX
#define NT_PPC_TAR      0x103        ///< PowerPC Target Address Register
#define NT_PPC_PPR      0x104        ///< PowerPC Program Priority Register
#define NT_PPC_DSCR     0x105        ///< PowerPC Data Stream Control Register
#define NT_PPC_EBB      0x106        ///< PowerPC Event Based Branch
#define NT_PPC_PMU      0x107        ///< PowerPC Performance Monitor
#define NT_PPC_TM_CGPR  0x108        ///< PowerPC TM checkpointed GPR
#define NT_PPC_TM_CFPR  0x109        ///< PowerPC TM checkpointed FPR
#define NT_PPC_TM_CVMX  0x10a        ///< PowerPC TM checkpointed VMX
#define NT_PPC_TM_CVSX  0x10b        ///< PowerPC TM checkpointed VSX
#define NT_PPC_TM_SPR   0x10c        ///< PowerPC TM Special Purpose Registers
#define NT_386_TLS      0x200        ///< x86 TLS slots
#define NT_386_IOPERM   0x201        ///< x86 I/O permissions
#define NT_X86_XSTATE   0x202        ///< x86 extended state (XSAVE)
#define NT_S390_HIGH_GPRS 0x300      ///< S/390 high GPRs
#define NT_S390_TIMER   0x301        ///< S/390 timer
#define NT_S390_TODCMP  0x302        ///< S/390 TOD comparator
#define NT_S390_TODPREG 0x303        ///< S/390 TOD programmable register
#define NT_S390_CTRS    0x304        ///< S/390 control registers
#define NT_S390_PREFIX  0x305        ///< S/390 prefix register
#define NT_S390_LAST_BREAK 0x306     ///< S/390 breaking event address
#define NT_S390_SYSTEM_CALL 0x307    ///< S/390 system call restart data
#define NT_S390_TDB     0x308        ///< S/390 transaction diagnostic block
#define NT_S390_VXRS_LOW 0x309       ///< S/390 vector registers 0-15 low
#define NT_S390_VXRS_HIGH 0x30a      ///< S/390 vector registers 16-31
#define NT_ARM_VFP      0x400        ///< ARM VFP registers
#define NT_ARM_TLS      0x401        ///< ARM TLS register
#define NT_ARM_HW_BREAK 0x402        ///< ARM hardware breakpoint
#define NT_ARM_HW_WATCH 0x403        ///< ARM hardware watchpoint
#define NT_ARM_SVE      0x405        ///< ARM SVE registers
#define NT_ARM_PAC_MASK 0x406        ///< ARM pointer auth code masks
#define NT_ARM_PACA_KEYS 0x407       ///< ARM pointer auth address keys
#define NT_ARM_PACG_KEYS 0x408       ///< ARM pointer auth generic keys
#define NT_ARM_TAGGED_ADDR_CTRL 0x409 ///< ARM tagged address control
#define NT_ARM_PAC_ENABLED_KEYS 0x40a ///< ARM pointer auth enabled keys
#define NT_ARM_SSVE     0x40b        ///< ARM streaming SVE registers
#define NT_ARM_ZA       0x40c        ///< ARM SME ZA register
#define NT_ARM_ZT       0x40d        ///< ARM SME2 ZT registers
#define NT_RISCV_CSR    0x900        ///< RISC-V control/status registers

//
/// GNU-specific note types (n_name = "GNU")
///
#define NT_GNU_ABI_TAG        1    ///< GNU ABI tag
#define NT_GNU_HWCAP          2    ///< Hardware capabilities
#define NT_GNU_BUILD_ID       3    ///< Unique build ID
#define NT_GNU_GOLD_VERSION   4    ///< Gold linker version
#define NT_GNU_PROPERTY_TYPE_0 5   ///< Program property note

//
/// Note name strings
///
#define ELF_NOTE_GNU      "GNU"       ///< GNU toolchain notes
#define ELF_NOTE_LINUX    "Linux"     ///< Linux kernel notes
#define ELF_NOTE_CORE     "CORE"      ///< Core dump notes
#define ELF_NOTE_FREEBSD  "FreeBSD"   ///< FreeBSD notes
#define ELF_NOTE_NETBSD   "NetBSD"    ///< NetBSD notes
#define ELF_NOTE_OPENBSD  "OpenBSD"   ///< OpenBSD notes
#define ELF_NOTE_SOLARIS  "SUNW Solaris" ///< Solaris notes

///
/// ELF Dynamic Section Entries (d_tag values)
/// Used for dynamic linking, shared libraries, and runtime behavior
///
#define DT_NULL             0    ///< End of dynamic section
#define DT_NEEDED           1    ///< Name of needed library
#define DT_PLTRELSZ         2    ///< Size of PLT relocs
#define DT_PLTGOT           3    ///< PLT and/or GOT address
#define DT_HASH             4    ///< Symbol hash table address
#define DT_STRTAB           5    ///< String table address
#define DT_SYMTAB           6    ///< Symbol table address
#define DT_RELA             7    ///< Relocation table address
#define DT_RELASZ           8    ///< Relocation table size
#define DT_RELAENT          9    ///< Relocation entry size
#define DT_STRSZ            10   ///< String table size
#define DT_SYMENT           11   ///< Symbol table entry size
#define DT_INIT             12   ///< Initialization function address
#define DT_FINI             13   ///< Termination function address
#define DT_SONAME           14   ///< Shared object name
#define DT_RPATH            15   ///< Library search path (deprecated)
#define DT_SYMBOLIC         16   ///< Symbol resolution starts here
#define DT_REL              17   ///< Relocation table address
#define DT_RELSZ            18   ///< Relocation table size
#define DT_RELENT           19   ///< Relocation entry size
#define DT_PLTREL           20   ///< Type of reloc in PLT
#define DT_DEBUG            21   ///< For debugging
#define DT_TEXTREL          22   ///< Reloc might modify .text
#define DT_JMPREL           23   ///< PLT relocation entries
#define DT_BIND_NOW         24   ///< Process all relocations at load
#define DT_INIT_ARRAY       25   ///< Array of init functions
#define DT_FINI_ARRAY       26   ///< Array of termination functions
#define DT_INIT_ARRAYSZ     27   ///< Size of init array
#define DT_FINI_ARRAYSZ     28   ///< Size of fini array
#define DT_RUNPATH          29   ///< Library search path
#define DT_FLAGS            30   ///< Flags
#define DT_ENCODING         32   ///< Start of encoded range
#define DT_PREINIT_ARRAY    32   ///< Array of preinit functions
#define DT_PREINIT_ARRAYSZ  33   ///< Size of preinit array
#define DT_SYMTAB_SHNDX     34   ///< Extended section indices

///
/// DT_* entries which fall between DT_VALRNGLO and DT_VALRNGHI use
/// the d_un.d_val field of the Elf*_Dyn structure.
///
#define DT_VALRNGLO         0x6ffffd00
#define DT_GNU_PRELINKED    0x6ffffdf5  ///< Prelinking timestamp
#define DT_GNU_CONFLICTSZ   0x6ffffdf6  ///< Size of conflict section
#define DT_GNU_LIBLISTSZ    0x6ffffdf7  ///< Size of library list
#define DT_CHECKSUM         0x6ffffdf8  ///< System checksum
#define DT_PLTPADSZ         0x6ffffdf9  ///< PLT padding size
#define DT_MOVEENT          0x6ffffdfa  ///< Move table entry size
#define DT_MOVESZ           0x6ffffdfb  ///< Move table size
#define DT_FEATURE_1        0x6ffffdfc  ///< Feature selection
#define DT_POSFLAG_1        0x6ffffdfd  ///< Flags for positional init/fini
#define DT_SYMINSZ          0x6ffffdfe  ///< Symbol information table size
#define DT_SYMINENT         0x6ffffdff  ///< Symbol information entry size
#define DT_VALRNGHI         0x6ffffdff

///
/// DT_* entries which fall between DT_ADDRRNGLO and DT_ADDRRNGHI use
/// the d_un.d_ptr field of the Elf*_Dyn structure.
///
#define DT_ADDRRNGLO        0x6ffffe00
#define DT_GNU_HASH         0x6ffffef5  ///< GNU-style hash table
#define DT_TLSDESC_PLT      0x6ffffef6  ///< PLT entry for TLS descriptor
#define DT_TLSDESC_GOT      0x6ffffef7  ///< GOT entry for TLS descriptor
#define DT_GNU_CONFLICT     0x6ffffef8  ///< Address of conflict section
#define DT_GNU_LIBLIST      0x6ffffef9  ///< Address of library list
#define DT_CONFIG           0x6ffffefa  ///< Configuration file
#define DT_DEPAUDIT         0x6ffffefb  ///< Audit library
#define DT_AUDIT            0x6ffffefc  ///< Audit library
#define DT_PLTPAD           0x6ffffefd  ///< PLT padding
#define DT_MOVETAB          0x6ffffefe  ///< Move table
#define DT_SYMINFO          0x6ffffeff  ///< Symbol information table
#define DT_ADDRRNGHI        0x6ffffeff

///
/// DT_VERSYM and DT_VERDEF/DT_VERNEED entries
///
#define DT_VERSYM           0x6ffffff0  ///< Version symbol table
#define DT_RELACOUNT        0x6ffffff9  ///< Count of RELATIVE relocs
#define DT_RELCOUNT         0x6ffffffa  ///< Count of RELATIVE relocs
#define DT_FLAGS_1          0x6ffffffb  ///< State flags
#define DT_VERDEF           0x6ffffffc  ///< Version definition table
#define DT_VERDEFNUM        0x6ffffffd  ///< Number of version definitions
#define DT_VERNEED          0x6ffffffe  ///< Version dependency table
#define DT_VERNEEDNUM       0x6fffffff  ///< Number of version dependencies

///
/// Processor-specific dynamic entries
///
#define DT_LOPROC           0x70000000  ///< Processor-specific start
#define DT_HIPROC           0x7fffffff  ///< Processor-specific end

///
/// DT_FLAGS values
///
#define DF_ORIGIN           0x00000001  ///< Object may use $ORIGIN
#define DF_SYMBOLIC         0x00000002  ///< Symbol resolutions starts here
#define DF_TEXTREL          0x00000004  ///< Object contains text relocations
#define DF_BIND_NOW         0x00000008  ///< No lazy binding
#define DF_STATIC_TLS       0x00000010  ///< Module uses static TLS

///
/// DT_FLAGS_1 values
///
#define DF_1_NOW            0x00000001  ///< Complete relocation processing
#define DF_1_GLOBAL         0x00000002  ///< Set RTLD_GLOBAL
#define DF_1_GROUP          0x00000004  ///< Set RTLD_GROUP
#define DF_1_NODELETE       0x00000008  ///< Set RTLD_NODELETE
#define DF_1_LOADFLTR       0x00000010  ///< Trigger filtee loading
#define DF_1_INITFIRST      0x00000020  ///< Initialize first
#define DF_1_NOOPEN         0x00000040  ///< Cannot dlopen
#define DF_1_ORIGIN         0x00000080  ///< $ORIGIN processing required
#define DF_1_DIRECT         0x00000100  ///< Direct binding enabled
#define DF_1_TRANS          0x00000200  ///< Object is a translator
#define DF_1_INTERPOSE      0x00000400  ///< Object is an interposer
#define DF_1_NODEFLIB       0x00000800  ///< Ignore default library search
#define DF_1_NODUMP         0x00001000  ///< Object cannot be dumped
#define DF_1_CONFALT        0x00002000  ///< Configuration alternative
#define DF_1_ENDFILTEE      0x00004000  ///< Filtee terminates filter search
#define DF_1_DISPRELDNE     0x00008000  ///< Displacement relocation done
#define DF_1_DISPRELPND     0x00010000  ///< Displacement relocation pending
#define DF_1_NODIRECT       0x00020000  ///< No direct binding
#define DF_1_IGNMULDEF      0x00040000  ///< Ignore multiple definitions
#define DF_1_NOKSYMS        0x00080000  ///< No keep symbols
#define DF_1_NOHDR          0x00100000  ///< No header
#define DF_1_EDITED         0x00200000  ///< Object has been edited
#define DF_1_NORELOC        0x00400000  ///< No relocation processing
#define DF_1_SYMINTPOSE     0x00800000  ///< Symbol interposition
#define DF_1_GLOBAUDIT      0x01000000  ///< Global audit
#define DF_1_SINGLETON      0x02000000  ///< Singleton object
#define DF_1_STUB           0x04000000  ///< Stub object
#define DF_1_PIE            0x08000000  ///< Position Independent Executable

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
