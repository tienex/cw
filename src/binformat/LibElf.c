/** @file
  ELF Binary Format Library Implementation.

  This file implements comprehensive support for ELF32, ELF64, and FatELF
  binary formats following the unified binary format API.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LibElf.h"

//
// Define INLINE for byte-swapping helper functions
//
#ifndef INLINE
#define INLINE static inline
#endif

//
// ELF Identification Indices
//
#define EI_MAG0        0   ///< File identification byte 0 index
#define EI_MAG1        1   ///< File identification byte 1 index
#define EI_MAG2        2   ///< File identification byte 2 index
#define EI_MAG3        3   ///< File identification byte 3 index
#define EI_CLASS       4   ///< File class
#define EI_DATA        5   ///< Data encoding
#define EI_VERSION     6   ///< File version
#define EI_OSABI       7   ///< OS/ABI identification
#define EI_ABIVERSION  8   ///< ABI version
#define EI_PAD         9   ///< Start of padding bytes
#define EI_NIDENT      16  ///< Size of e_ident[]

//
// ELF Class
//
#define ELFCLASSNONE   0   ///< Invalid class
#define ELFCLASS32     1   ///< 32-bit objects
#define ELFCLASS64     2   ///< 64-bit objects

//
// ELF Data Encoding
//
#define ELFDATANONE    0   ///< Invalid data encoding
#define ELFDATA2LSB    1   ///< Little-endian
#define ELFDATA2MSB    2   ///< Big-endian

//
// ELF Version
//
#define EV_NONE        0   ///< Invalid version
#define EV_CURRENT     1   ///< Current version

//
// ELF Object File Types
//
#define ET_NONE        0   ///< No file type
#define ET_REL         1   ///< Relocatable file
#define ET_EXEC        2   ///< Executable file
#define ET_DYN         3   ///< Shared object file
#define ET_CORE        4   ///< Core file

//
// ELF Machine Types
//
#define EM_NONE        0   ///< No machine
#define EM_MMIX        80  ///< MMIX
#define EM_X86_64      62  ///< AMD x86-64
#define EM_386         3   ///< Intel 80386
#define EM_ARM         40  ///< ARM
#define EM_AARCH64     183 ///< ARM 64-bit
#define EM_PPC         20  ///< PowerPC
#define EM_PPC64       21  ///< PowerPC 64-bit
#define EM_MIPS        8   ///< MIPS
#define EM_SPARC       2   ///< SPARC
#define EM_SPARCV9     43  ///< SPARC v9
#define EM_RISCV       243 ///< RISC-V
#define EM_IA_64       50  ///< Intel IA-64
#define EM_ALPHA       0x9026  ///< DEC Alpha
#define EM_68K         4   ///< Motorola 68000
#define EM_VAX         75  ///< DEC VAX
#define EM_S390        22  ///< IBM S/390
// Historical and specialized architectures
#define EM_WE32K       1   ///< AT&T WE 32100
#define EM_NS32K       97  ///< National Semiconductor 32000
#define EM_TAHOE       99  ///< Tahoe
#define EM_PDP10       64  ///< DEC PDP-10
#define EM_PDP11       65  ///< DEC PDP-11
#define EM_88K         5   ///< Motorola 88000
#define EM_DLX         0x5aa5  ///< DLX
#define EM_MOXIE       223    ///< Moxie
#define EM_LOONGARCH   258    ///< LoongArch
#define EM_CONVEX      0x5143 ///< Convex
#define EM_PYRAMID     0x5045 ///< Pyramid
#define EM_CRAY        0x4352 ///< Cray
#define EM_HPFOCUS     0x4846 ///< HP Focus
#define EM_EBC         0x1057 ///< EFI Byte Code
#define EM_8086        0  ///< Intel 8086 (uses EM_NONE with flags)
#define EM_80286       0x286  ///< Intel 80286
#define EM_29K         29     ///< AMD 29000
#define EM_AVR         83     ///< Atmel AVR
#define EM_AVR32       0x18AD ///< Atmel AVR32
#define EM_NIOS2       113    ///< Altera Nios II
#define EM_MICROBLAZE  189    ///< Xilinx MicroBlaze
#define EM_OPENRISC    92     ///< OpenRISC
#define EM_MSP430      105    ///< TI MSP430
#define EM_LANAI       244    ///< Google Lanai
#define EM_ELBRUS      175    ///< MCST Elbrus e2k
#define EM_CLIPPER     0x434C ///< Intergraph Clipper
#define EM_BPF         247    ///< Linux BPF
#define EM_HEXAGON     164    ///< Qualcomm Hexagon
#define EM_CSKY        252    ///< C-SKY
#define EM_FR30        84     ///< Fujitsu FR30
#define EM_MN10200     90     ///< Matsushita MN10200
#define EM_MN10300     89     ///< Matsushita MN10300
#define EM_FRV         0x5441 ///< Fujitsu FR-V
#define EM_NECVE       251    ///< NEC SX-Aurora
#define EM_IP2K        101    ///< Ubicom IP2000
#define EM_IQ2000      0xFEBA ///< Vitesse IQ2000
#define EM_CRIS        76     ///< Axis CRIS
#define EM_ARC         45     ///< ARC
#define EM_SH          42     ///< SuperH
#define EM_PARISC      15     ///< HP PA-RISC
#define EM_CR16        177    ///< National Semiconductor CR16
#define EM_D10V        85     ///< Mitsubishi D10V
#define EM_D30V        86     ///< Mitsubishi D30V
#define EM_XTENSA      94     ///< Tensilica Xtensa
#define EM_860         7      ///< Intel i860
#define EM_960         19     ///< Intel i960
#define EM_TI_C6000    140    ///< TI TMS320C6x
#define EM_TI_C5500    0xC500 ///< TI TMS320C55x
#define EM_TI_C5400    0xC540 ///< TI TMS320C54x
#define EM_BLACKFIN    106    ///< Analog Devices Blackfin
#define EM_EPIPHANY    0x1223 ///< Adapteva Epiphany
#define EM_M32R        88     ///< Mitsubishi M32R
#define EM_M32C        0x13b  ///< Renesas M32C
#define EM_LATTICEMICO32 138  ///< Lattice Mico32
#define EM_NDS32       167    ///< Andes NDS32
#define EM_V850        87     ///< NEC V850
#define EM_TILEPRO     188    ///< Tilera TILE-Gx
#define EM_Z80         220    ///< Zilog Z80
#define EM_Z8000       0x8000 ///< Zilog Z8000
#define EM_6502        0x6502 ///< MOS 6502
#define EM_65816       0x6516 ///< WDC 65816
#define EM_MCORE       39     ///< Motorola MCore
#define EM_TRICORE     44     ///< Infineon TriCore
#define EM_MEP         0xF00D ///< Toshiba MeP
#define EM_PICOJAVA    0xCAFE ///< Sun picoJava
#define EM_MN103       0xBEEF ///< Matsushita MN103
#define EM_METAG       174    ///< Imagination Meta
#define EM_SCORE       135    ///< Sunplus S+core
#define EM_RL78        0x00C1 ///< Renesas RL78
#define EM_RX          173    ///< Renesas RX
#define EM_VISIUM      221    ///< CDS VISium
#define EM_XGATE       115    ///< Freescale XGATE
#define EM_XSTORMY16   0xAD45 ///< Sanyo XStormy16
#define EM_KVX         256    ///< Kalray KVX
#define EM_NFP         250    ///< Netronome Flow Processor
#define EM_MT          0x2530 ///< Morpho MT
#define EM_FT32        0xFD32 ///< FTDI FT32
#define EM_H8_300      46     ///< Hitachi H8/300
#define EM_SPU         23     ///< Cell Broadband Engine SPU
#define EM_WEBASSEMBLY 0x4157 ///< WebAssembly
#define EM_PRU         144    ///< TI PRU

//
// Section Header Types
//
#define SHT_NULL       0   ///< Inactive
#define SHT_PROGBITS   1   ///< Program data
#define SHT_SYMTAB     2   ///< Symbol table
#define SHT_STRTAB     3   ///< String table
#define SHT_RELA       4   ///< Relocation entries with addends
#define SHT_HASH       5   ///< Symbol hash table
#define SHT_DYNAMIC    6   ///< Dynamic linking information
#define SHT_NOTE       7   ///< Notes
#define SHT_NOBITS     8   ///< BSS
#define SHT_REL        9   ///< Relocation entries
#define SHT_SHLIB      10  ///< Reserved
#define SHT_DYNSYM     11  ///< Dynamic linker symbol table

//
// Section Header Flags
//
#define SHF_WRITE      0x1   ///< Writable
#define SHF_ALLOC      0x2   ///< Occupies memory
#define SHF_EXECINSTR  0x4   ///< Executable
#define SHF_MERGE      0x10  ///< Might be merged
#define SHF_STRINGS    0x20  ///< Contains null-terminated strings
#define SHF_INFO_LINK  0x40  ///< sh_info contains SHT index
#define SHF_LINK_ORDER 0x80  ///< Preserve order after combining
#define SHF_TLS        0x400 ///< Thread-local storage

//
// Program Header Types
//
#define PT_NULL        0   ///< Unused
#define PT_LOAD        1   ///< Loadable segment
#define PT_DYNAMIC     2   ///< Dynamic linking information
#define PT_INTERP      3   ///< Interpreter pathname
#define PT_NOTE        4   ///< Auxiliary information
#define PT_SHLIB       5   ///< Reserved
#define PT_PHDR        6   ///< Program header table
#define PT_TLS         7   ///< Thread-local storage

//
// Program Header Flags
//
#define PF_X           0x1   ///< Execute
#define PF_W           0x2   ///< Write
#define PF_R           0x4   ///< Read

//
// Symbol Binding
//
#define STB_LOCAL      0   ///< Local symbol
#define STB_GLOBAL     1   ///< Global symbol
#define STB_WEAK       2   ///< Weak symbol

//
// Symbol Types
//
#define STT_NOTYPE     0   ///< Symbol type is unspecified
#define STT_OBJECT     1   ///< Symbol is a data object
#define STT_FUNC       2   ///< Symbol is a code object
#define STT_SECTION    3   ///< Symbol associated with a section
#define STT_FILE       4   ///< Symbol's name is file name
#define STT_COMMON     5   ///< Symbol is a common data object
#define STT_TLS        6   ///< Symbol is thread-local data object

//
// ELF32 Structures
//
#pragma pack(push, 1)

typedef struct {
  UINT8   e_ident[EI_NIDENT]; ///< Magic number and other info
  UINT16  e_type;              ///< Object file type
  UINT16  e_machine;           ///< Architecture
  UINT32  e_version;           ///< Object file version
  UINT32  e_entry;             ///< Entry point virtual address
  UINT32  e_phoff;             ///< Program header table file offset
  UINT32  e_shoff;             ///< Section header table file offset
  UINT32  e_flags;             ///< Processor-specific flags
  UINT16  e_ehsize;            ///< ELF header size in bytes
  UINT16  e_phentsize;         ///< Program header table entry size
  UINT16  e_phnum;             ///< Program header table entry count
  UINT16  e_shentsize;         ///< Section header table entry size
  UINT16  e_shnum;             ///< Section header table entry count
  UINT16  e_shstrndx;          ///< Section header string table index
} Elf32_Ehdr;

typedef struct {
  UINT32  sh_name;       ///< Section name (string tbl index)
  UINT32  sh_type;       ///< Section type
  UINT32  sh_flags;      ///< Section flags
  UINT32  sh_addr;       ///< Section virtual addr at execution
  UINT32  sh_offset;     ///< Section file offset
  UINT32  sh_size;       ///< Section size in bytes
  UINT32  sh_link;       ///< Link to another section
  UINT32  sh_info;       ///< Additional section information
  UINT32  sh_addralign;  ///< Section alignment
  UINT32  sh_entsize;    ///< Entry size if section holds table
} Elf32_Shdr;

typedef struct {
  UINT32  p_type;    ///< Segment type
  UINT32  p_offset;  ///< Segment file offset
  UINT32  p_vaddr;   ///< Segment virtual address
  UINT32  p_paddr;   ///< Segment physical address
  UINT32  p_filesz;  ///< Segment size in file
  UINT32  p_memsz;   ///< Segment size in memory
  UINT32  p_flags;   ///< Segment flags
  UINT32  p_align;   ///< Segment alignment
} Elf32_Phdr;

typedef struct {
  UINT32  st_name;   ///< Symbol name (string tbl index)
  UINT32  st_value;  ///< Symbol value
  UINT32  st_size;   ///< Symbol size
  UINT8   st_info;   ///< Symbol type and binding
  UINT8   st_other;  ///< Symbol visibility
  UINT16  st_shndx;  ///< Section index
} Elf32_Sym;

typedef struct {
  UINT32  r_offset;  ///< Address
  UINT32  r_info;    ///< Relocation type and symbol index
} Elf32_Rel;

typedef struct {
  UINT32  r_offset;  ///< Address
  UINT32  r_info;    ///< Relocation type and symbol index
  INT32   r_addend;  ///< Addend
} Elf32_Rela;

//
// ELF64 Structures
//
typedef struct {
  UINT8   e_ident[EI_NIDENT]; ///< Magic number and other info
  UINT16  e_type;              ///< Object file type
  UINT16  e_machine;           ///< Architecture
  UINT32  e_version;           ///< Object file version
  UINT64  e_entry;             ///< Entry point virtual address
  UINT64  e_phoff;             ///< Program header table file offset
  UINT64  e_shoff;             ///< Section header table file offset
  UINT32  e_flags;             ///< Processor-specific flags
  UINT16  e_ehsize;            ///< ELF header size in bytes
  UINT16  e_phentsize;         ///< Program header table entry size
  UINT16  e_phnum;             ///< Program header table entry count
  UINT16  e_shentsize;         ///< Section header table entry size
  UINT16  e_shnum;             ///< Section header table entry count
  UINT16  e_shstrndx;          ///< Section header string table index
} Elf64_Ehdr;

typedef struct {
  UINT32  sh_name;       ///< Section name (string tbl index)
  UINT32  sh_type;       ///< Section type
  UINT64  sh_flags;      ///< Section flags
  UINT64  sh_addr;       ///< Section virtual addr at execution
  UINT64  sh_offset;     ///< Section file offset
  UINT64  sh_size;       ///< Section size in bytes
  UINT32  sh_link;       ///< Link to another section
  UINT32  sh_info;       ///< Additional section information
  UINT64  sh_addralign;  ///< Section alignment
  UINT64  sh_entsize;    ///< Entry size if section holds table
} Elf64_Shdr;

typedef struct {
  UINT32  p_type;    ///< Segment type
  UINT32  p_flags;   ///< Segment flags
  UINT64  p_offset;  ///< Segment file offset
  UINT64  p_vaddr;   ///< Segment virtual address
  UINT64  p_paddr;   ///< Segment physical address
  UINT64  p_filesz;  ///< Segment size in file
  UINT64  p_memsz;   ///< Segment size in memory
  UINT64  p_align;   ///< Segment alignment
} Elf64_Phdr;

typedef struct {
  UINT32  st_name;   ///< Symbol name (string tbl index)
  UINT8   st_info;   ///< Symbol type and binding
  UINT8   st_other;  ///< Symbol visibility
  UINT16  st_shndx;  ///< Section index
  UINT64  st_value;  ///< Symbol value
  UINT64  st_size;   ///< Symbol size
} Elf64_Sym;

typedef struct {
  UINT64  r_offset;  ///< Address
  UINT64  r_info;    ///< Relocation type and symbol index
} Elf64_Rel;

typedef struct {
  UINT64  r_offset;  ///< Address
  UINT64  r_info;    ///< Relocation type and symbol index
  INT64   r_addend;  ///< Addend
} Elf64_Rela;

//
// ELF Dynamic Section
//
typedef struct {
  INT32   d_tag;   ///< Dynamic entry type
  union {
    UINT32  d_val; ///< Integer value
    UINT32  d_ptr; ///< Address value
  } d_un;
} Elf32_Dyn;

typedef struct {
  INT64   d_tag;   ///< Dynamic entry type
  union {
    UINT64  d_val; ///< Integer value
    UINT64  d_ptr; ///< Address value
  } d_un;
} Elf64_Dyn;

//
// ELF Note Header
//
typedef struct {
  UINT32  n_namesz; ///< Length of name
  UINT32  n_descsz; ///< Length of descriptor
  UINT32  n_type;   ///< Type of note
} Elf32_Nhdr;

typedef Elf32_Nhdr Elf64_Nhdr;

//
// Dynamic Section Tags
//
#define DT_NULL         0   ///< Marks end of dynamic section
#define DT_NEEDED       1   ///< Name of needed library
#define DT_PLTRELSZ     2   ///< Size of PLT relocs
#define DT_PLTGOT       3   ///< Processor defined value
#define DT_HASH         4   ///< Address of symbol hash table
#define DT_STRTAB       5   ///< Address of string table
#define DT_SYMTAB       6   ///< Address of symbol table
#define DT_RELA         7   ///< Address of Rela relocs
#define DT_RELASZ       8   ///< Total size of Rela relocs
#define DT_RELAENT      9   ///< Size of one Rela reloc
#define DT_STRSZ        10  ///< Size of string table
#define DT_SYMENT       11  ///< Size of one symbol table entry
#define DT_INIT         12  ///< Address of init function
#define DT_FINI         13  ///< Address of termination function
#define DT_SONAME       14  ///< Name of shared object
#define DT_RPATH        15  ///< Library search path (deprecated)
#define DT_SYMBOLIC     16  ///< Start symbol search here
#define DT_REL          17  ///< Address of Rel relocs
#define DT_RELSZ        18  ///< Total size of Rel relocs
#define DT_RELENT       19  ///< Size of one Rel reloc
#define DT_PLTREL       20  ///< Type of reloc in PLT
#define DT_DEBUG        21  ///< For debugging
#define DT_TEXTREL      22  ///< Reloc might modify .text
#define DT_JMPREL       23  ///< Address of PLT relocs
#define DT_BIND_NOW     24  ///< Process relocations at load
#define DT_RUNPATH      29  ///< Library search path

//
// FatELF Structures
//
typedef struct {
  UINT32  magic;     ///< FatELF magic (0x0FAEF1FA)
  UINT16  version;   ///< FatELF version
  UINT8   num_records; ///< Number of architecture records
  UINT8   reserved;  ///< Reserved, must be 0
} FatELF_Header;

typedef struct {
  UINT16  machine;   ///< ELF machine type
  UINT8   osabi;     ///< ELF OS/ABI
  UINT8   osabi_version; ///< ELF OS/ABI version
  UINT8   word_size; ///< 1=32-bit, 2=64-bit
  UINT8   byte_order; ///< 1=little-endian, 2=big-endian
  UINT8   reserved0; ///< Reserved
  UINT8   reserved1; ///< Reserved
  UINT64  offset;    ///< Offset in file
  UINT64  size;      ///< Size of binary
} FatELF_Record;

#pragma pack(pop)

//
// ELF Context Structure
//
typedef struct _ELF_CONTEXT {
  BOOLEAN         ReadOnly;       ///< Read-only mode
  BOOLEAN         Is64Bit;        ///< TRUE for ELF64
  BOOLEAN         IsFat;          ///< TRUE for FatELF
  UINT8           *FileData;      ///< File data buffer
  UINT64          FileSize;       ///< File size
  BOOLEAN         OwnBuffer;      ///< TRUE if we allocated the buffer

  ///
  /// FatELF support
  ///
  FatELF_Header   *FatHeader;
  FatELF_Record   *FatRecords;
  UINT32          SelectedArch;   ///< Currently selected architecture
  UINT64          CurrentOffset;  ///< Offset to current ELF binary

  ///
  /// ELF Headers
  ///
  union {
    Elf32_Ehdr    *Elf32;
    Elf64_Ehdr    *Elf64;
  } Header;

  ///
  /// Section Headers
  ///
  union {
    Elf32_Shdr    *Elf32;
    Elf64_Shdr    *Elf64;
  } Sections;

  ///
  /// Program Headers
  ///
  union {
    Elf32_Phdr    *Elf32;
    Elf64_Phdr    *Elf64;
  } Programs;

  ///
  /// String tables
  ///
  CHAR8           *StringTable;   ///< Section name string table
  CHAR8           *SymbolStringTable; ///< Symbol name string table

  ///
  /// Symbol table
  ///
  union {
    Elf32_Sym     *Elf32;
    Elf64_Sym     *Elf64;
  } Symbols;
  UINT32          SymbolCount;

  ///
  /// Endianness handling
  ///
  BOOLEAN         NeedsByteSwap;  ///< TRUE if file endianness != host endianness
} ELF_CONTEXT;

#define ELF_CONTEXT_FROM_BINFORMAT(ctx) ((ELF_CONTEXT *)(ctx))

//
// Helper Macros
//
#define ELF32_ST_BIND(i)    ((i) >> 4)
#define ELF32_ST_TYPE(i)    ((i) & 0xf)
#define ELF32_R_SYM(i)      ((i) >> 8)
#define ELF32_R_TYPE(i)     ((i) & 0xff)

#define ELF64_ST_BIND(i)    ((i) >> 4)
#define ELF64_ST_TYPE(i)    ((i) & 0xf)
#define ELF64_R_SYM(i)      ((i) >> 32)
#define ELF64_R_TYPE(i)     ((i) & 0xffffffffL)

//
// Byte-swapping utility functions
//
INLINE
UINT16
ElfSwap16 (
  IN  UINT16  Value
  )
{
  return ((Value & 0xFF) << 8) | ((Value >> 8) & 0xFF);
}

INLINE
UINT32
ElfSwap32 (
  IN  UINT32  Value
  )
{
  return ((Value & 0x000000FF) << 24) |
         ((Value & 0x0000FF00) << 8) |
         ((Value & 0x00FF0000) >> 8) |
         ((Value & 0xFF000000) >> 24);
}

INLINE
UINT64
ElfSwap64 (
  IN  UINT64  Value
  )
{
  return ((Value & 0x00000000000000FFULL) << 56) |
         ((Value & 0x000000000000FF00ULL) << 40) |
         ((Value & 0x0000000000FF0000ULL) << 24) |
         ((Value & 0x00000000FF000000ULL) << 8) |
         ((Value & 0x000000FF00000000ULL) >> 8) |
         ((Value & 0x0000FF0000000000ULL) >> 24) |
         ((Value & 0x00FF000000000000ULL) >> 40) |
         ((Value & 0xFF00000000000000ULL) >> 56);
}

//
// Detect host endianness
//
INLINE
BOOLEAN
IsLittleEndianHost (
  VOID
  )
{
  UINT16 test = 0x0001;
  return *((UINT8*)&test) == 0x01;
}

//
// Conditional swap macros - swap only if needed
//
#define SWAP16(ctx, val)  ((ctx)->NeedsByteSwap ? ElfSwap16(val) : (val))
#define SWAP32(ctx, val)  ((ctx)->NeedsByteSwap ? ElfSwap32(val) : (val))
#define SWAP64(ctx, val)  ((ctx)->NeedsByteSwap ? ElfSwap64(val) : (val))

//
// x86 (i386) Relocation Types
//
#define R_386_NONE           0   ///< No relocation
#define R_386_32             1   ///< Direct 32 bit
#define R_386_PC32           2   ///< PC relative 32 bit
#define R_386_GOT32          3   ///< 32 bit GOT entry
#define R_386_PLT32          4   ///< 32 bit PLT address
#define R_386_COPY           5   ///< Copy symbol at runtime
#define R_386_GLOB_DAT       6   ///< Create GOT entry
#define R_386_JMP_SLOT       7   ///< Create PLT entry
#define R_386_RELATIVE       8   ///< Adjust by program base
#define R_386_GOTOFF         9   ///< 32 bit offset to GOT
#define R_386_GOTPC          10  ///< 32 bit PC relative offset to GOT
#define R_386_32PLT          11  ///< 32 bit PLT address
#define R_386_TLS_TPOFF      14  ///< Offset in static TLS block
#define R_386_TLS_IE         15  ///< Address of GOT entry for static TLS block offset
#define R_386_TLS_GOTIE      16  ///< GOT entry for static TLS block offset
#define R_386_TLS_LE         17  ///< Offset relative to static TLS block
#define R_386_TLS_GD         18  ///< Direct 32 bit for GNU version of general dynamic thread local data
#define R_386_TLS_LDM        19  ///< Direct 32 bit for GNU version of local dynamic thread local data
#define R_386_16             20  ///< Direct 16 bit
#define R_386_PC16           21  ///< 16 bit PC relative
#define R_386_8              22  ///< Direct 8 bit
#define R_386_PC8            23  ///< 8 bit PC relative
#define R_386_TLS_GD_32      24  ///< Direct 32 bit for general dynamic thread local data
#define R_386_TLS_GD_PUSH    25  ///< Tag for pushl in GD TLS code
#define R_386_TLS_GD_CALL    26  ///< Relocation for call to __tls_get_addr()
#define R_386_TLS_GD_POP     27  ///< Tag for popl in GD TLS code
#define R_386_TLS_LDM_32     28  ///< Direct 32 bit for local dynamic thread local data
#define R_386_TLS_LDM_PUSH   29  ///< Tag for pushl in LDM TLS code
#define R_386_TLS_LDM_CALL   30  ///< Relocation for call to __tls_get_addr() in LDM code
#define R_386_TLS_LDM_POP    31  ///< Tag for popl in LDM TLS code
#define R_386_TLS_LDO_32     32  ///< Offset relative to TLS block
#define R_386_TLS_IE_32      33  ///< GOT entry for negated static TLS block offset
#define R_386_TLS_LE_32      34  ///< Negated offset relative to static TLS block
#define R_386_TLS_DTPMOD32   35  ///< ID of module containing symbol
#define R_386_TLS_DTPOFF32   36  ///< Offset in TLS block
#define R_386_TLS_TPOFF32    37  ///< Negated offset in static TLS block
#define R_386_SIZE32         38  ///< 32-bit symbol size
#define R_386_TLS_GOTDESC    39  ///< GOT offset for TLS descriptor
#define R_386_TLS_DESC_CALL  40  ///< Marker of call through TLS descriptor
#define R_386_TLS_DESC       41  ///< TLS descriptor containing pointer to code and to argument
#define R_386_IRELATIVE      42  ///< Adjust indirectly by program base
#define R_386_GOT32X         43  ///< Load from 32 bit GOT entry, relaxable

//
// x86-64 (AMD64) Relocation Types
//
#define R_X86_64_NONE          0   ///< No relocation
#define R_X86_64_64            1   ///< Direct 64 bit
#define R_X86_64_PC32          2   ///< PC relative 32 bit signed
#define R_X86_64_GOT32         3   ///< 32 bit GOT entry
#define R_X86_64_PLT32         4   ///< 32 bit PLT address
#define R_X86_64_COPY          5   ///< Copy symbol at runtime
#define R_X86_64_GLOB_DAT      6   ///< Create GOT entry
#define R_X86_64_JUMP_SLOT     7   ///< Create PLT entry
#define R_X86_64_RELATIVE      8   ///< Adjust by program base
#define R_X86_64_GOTPCREL      9   ///< 32 bit signed PC relative offset to GOT
#define R_X86_64_32            10  ///< Direct 32 bit zero extended
#define R_X86_64_32S           11  ///< Direct 32 bit sign extended
#define R_X86_64_16            12  ///< Direct 16 bit zero extended
#define R_X86_64_PC16          13  ///< 16 bit sign extended pc relative
#define R_X86_64_8             14  ///< Direct 8 bit sign extended
#define R_X86_64_PC8           15  ///< 8 bit sign extended pc relative
#define R_X86_64_DTPMOD64      16  ///< ID of module containing symbol
#define R_X86_64_DTPOFF64      17  ///< Offset in module's TLS block
#define R_X86_64_TPOFF64       18  ///< Offset in initial TLS block
#define R_X86_64_TLSGD         19  ///< 32 bit signed PC relative offset to two GOT entries for GD symbol
#define R_X86_64_TLSLD         20  ///< 32 bit signed PC relative offset to two GOT entries for LD symbol
#define R_X86_64_DTPOFF32      21  ///< Offset in TLS block
#define R_X86_64_GOTTPOFF      22  ///< 32 bit signed PC relative offset to GOT entry for IE symbol
#define R_X86_64_TPOFF32       23  ///< Offset in initial TLS block
#define R_X86_64_PC64          24  ///< PC relative 64 bit
#define R_X86_64_GOTOFF64      25  ///< 64 bit offset to GOT
#define R_X86_64_GOTPC32       26  ///< 32 bit signed pc relative offset to GOT
#define R_X86_64_GOT64         27  ///< 64-bit GOT entry offset
#define R_X86_64_GOTPCREL64    28  ///< 64-bit PC relative offset to GOT entry
#define R_X86_64_GOTPC64       29  ///< 64-bit PC relative offset to GOT
#define R_X86_64_GOTPLT64      30  ///< Like GOT64, says PLT entry needed
#define R_X86_64_PLTOFF64      31  ///< 64-bit GOT relative offset to PLT entry
#define R_X86_64_SIZE32        32  ///< Size of symbol plus 32-bit addend
#define R_X86_64_SIZE64        33  ///< Size of symbol plus 64-bit addend
#define R_X86_64_GOTPC32_TLSDESC 34  ///< GOT offset for TLS descriptor
#define R_X86_64_TLSDESC_CALL  35  ///< Marker for call through TLS descriptor
#define R_X86_64_TLSDESC       36  ///< TLS descriptor
#define R_X86_64_IRELATIVE     37  ///< Adjust indirectly by program base
#define R_X86_64_RELATIVE64    38  ///< 64-bit adjust by program base
#define R_X86_64_GOTPCRELX     41  ///< Load from 32 bit signed pc relative offset to GOT entry without REX prefix, relaxable
#define R_X86_64_REX_GOTPCRELX 42  ///< Load from 32 bit signed pc relative offset to GOT entry with REX prefix, relaxable

//
// ARM 32-bit Relocation Types
//
#define R_ARM_NONE             0   ///< No relocation
#define R_ARM_PC24             1   ///< Deprecated ARM ((S + A) | T) - P
#define R_ARM_ABS32            2   ///< Direct 32 bit (S + A) | T
#define R_ARM_REL32            3   ///< PC relative 32 bit ((S + A) | T) - P
#define R_ARM_LDR_PC_G0        4   ///< S + A - P
#define R_ARM_ABS16            5   ///< Direct 16 bit S + A
#define R_ARM_ABS12            6   ///< Direct 12 bit S + A
#define R_ARM_THM_ABS5         7   ///< Direct & 0x7C (LDR, STR) S + A
#define R_ARM_ABS8             8   ///< Direct 8 bit S + A
#define R_ARM_SBREL32          9   ///< ((S + A) | T) - B(S)
#define R_ARM_THM_CALL         10  ///< Thumb BL ((S + A) | T) - P
#define R_ARM_THM_PC8          11  ///< Thumb PC relative 8 bit S + A - Pa
#define R_ARM_BREL_ADJ         12  ///< Delta B(S) + A
#define R_ARM_TLS_DESC         13  ///< TLS descriptor
#define R_ARM_THM_SWI8         14  ///< Obsolete
#define R_ARM_XPC25            15  ///< Obsolete
#define R_ARM_THM_XPC22        16  ///< Obsolete
#define R_ARM_TLS_DTPMOD32     17  ///< ID of module containing symbol
#define R_ARM_TLS_DTPOFF32     18  ///< Offset in TLS block
#define R_ARM_TLS_TPOFF32      19  ///< Offset in static TLS block
#define R_ARM_COPY             20  ///< Copy symbol at runtime
#define R_ARM_GLOB_DAT         21  ///< Create GOT entry
#define R_ARM_JUMP_SLOT        22  ///< Create PLT entry
#define R_ARM_RELATIVE         23  ///< Adjust by program base
#define R_ARM_GOTOFF32         24  ///< 32 bit offset to GOT
#define R_ARM_BASE_PREL        25  ///< 32 bit PC relative offset to GOT
#define R_ARM_GOT_BREL         26  ///< 32 bit GOT entry
#define R_ARM_PLT32            27  ///< Deprecated PLT reloc
#define R_ARM_CALL             28  ///< PC relative ((S + A) | T) - P
#define R_ARM_JUMP24           29  ///< PC relative ((S + A) | T) - P
#define R_ARM_THM_JUMP24       30  ///< Thumb PC relative ((S + A) | T) - P
#define R_ARM_BASE_ABS         31  ///< Adjust by program base
#define R_ARM_ALU_PCREL_7_0    32  ///< Obsolete
#define R_ARM_ALU_PCREL_15_8   33  ///< Obsolete
#define R_ARM_ALU_PCREL_23_15  34  ///< Obsolete
#define R_ARM_LDR_SBREL_11_0_NC 35  ///< Deprecated
#define R_ARM_ALU_SBREL_19_12_NC 36  ///< Deprecated
#define R_ARM_ALU_SBREL_27_20_CK 37  ///< Deprecated
#define R_ARM_TARGET1          38  ///< ((S + A) | T) or ((S + A) | T) - P
#define R_ARM_SBREL31          39  ///< Deprecated
#define R_ARM_V4BX             40  ///< BX target
#define R_ARM_TARGET2          41  ///< Platform specific
#define R_ARM_PREL31           42  ///< ((S + A) | T) - P
#define R_ARM_MOVW_ABS_NC      43  ///< Direct 16-bit (S + A) | T
#define R_ARM_MOVT_ABS         44  ///< Direct high 16-bit S + A
#define R_ARM_MOVW_PREL_NC     45  ///< PC relative 16-bit ((S + A) | T) - P
#define R_ARM_MOVT_PREL        46  ///< PC relative high ((S + A) | T) - P
#define R_ARM_THM_MOVW_ABS_NC  47  ///< Direct 16 bit (S + A) | T
#define R_ARM_THM_MOVT_ABS     48  ///< Direct high 16 bit S + A
#define R_ARM_THM_MOVW_PREL_NC 49  ///< PC relative 16 bit ((S + A) | T) - P
#define R_ARM_THM_MOVT_PREL    50  ///< PC relative high ((S + A) | T) - P
#define R_ARM_THM_JUMP19       51  ///< Thumb PC relative ((S + A) | T) - P
#define R_ARM_THM_JUMP6        52  ///< Thumb PC relative S + A - P
#define R_ARM_THM_ALU_PREL_11_0 53  ///< Thumb PC relative (S + A) - Pa
#define R_ARM_THM_PC12         54  ///< Thumb PC relative S + A - Pa
#define R_ARM_ABS32_NOI        55  ///< Direct 32 bit S + A
#define R_ARM_REL32_NOI        56  ///< PC relative 32 bit S + A - P
#define R_ARM_ALU_PC_G0_NC     57  ///< PC relative (S + A) - P
#define R_ARM_ALU_PC_G0        58  ///< PC relative (S + A) - P
#define R_ARM_ALU_PC_G1_NC     59  ///< PC relative (S + A) - P
#define R_ARM_ALU_PC_G1        60  ///< PC relative (S + A) - P
#define R_ARM_ALU_PC_G2        61  ///< PC relative (S + A) - P
#define R_ARM_LDR_PC_G1        62  ///< PC relative S + A - P
#define R_ARM_LDR_PC_G2        63  ///< PC relative S + A - P
#define R_ARM_LDRS_PC_G0       64  ///< PC relative S + A - P
#define R_ARM_LDRS_PC_G1       65  ///< PC relative S + A - P
#define R_ARM_LDRS_PC_G2       66  ///< PC relative S + A - P
#define R_ARM_LDC_PC_G0        67  ///< PC relative S + A - P
#define R_ARM_LDC_PC_G1        68  ///< PC relative S + A - P
#define R_ARM_LDC_PC_G2        69  ///< PC relative S + A - P
#define R_ARM_ALU_SB_G0_NC     70  ///< B(S) + A - P
#define R_ARM_ALU_SB_G0        71  ///< B(S) + A - P
#define R_ARM_ALU_SB_G1_NC     72  ///< B(S) + A - P
#define R_ARM_ALU_SB_G1        73  ///< B(S) + A - P
#define R_ARM_ALU_SB_G2        74  ///< B(S) + A - P
#define R_ARM_LDR_SB_G0        75  ///< B(S) + A - P
#define R_ARM_LDR_SB_G1        76  ///< B(S) + A - P
#define R_ARM_LDR_SB_G2        77  ///< B(S) + A - P
#define R_ARM_LDRS_SB_G0       78  ///< B(S) + A - P
#define R_ARM_LDRS_SB_G1       79  ///< B(S) + A - P
#define R_ARM_LDRS_SB_G2       80  ///< B(S) + A - P
#define R_ARM_LDC_SB_G0        81  ///< B(S) + A - P
#define R_ARM_LDC_SB_G1        82  ///< B(S) + A - P
#define R_ARM_LDC_SB_G2        83  ///< B(S) + A - P
#define R_ARM_MOVW_BREL_NC     84  ///< ((S + A) | T) - B(S)
#define R_ARM_MOVT_BREL        85  ///< S + A - B(S)
#define R_ARM_MOVW_BREL        86  ///< ((S + A) | T) - B(S)
#define R_ARM_THM_MOVW_BREL_NC 87  ///< ((S + A) | T) - B(S)
#define R_ARM_THM_MOVT_BREL    88  ///< S + A - B(S)
#define R_ARM_THM_MOVW_BREL    89  ///< ((S + A) | T) - B(S)
#define R_ARM_TLS_GOTDESC      90  ///< GOT entry for TLS descriptor
#define R_ARM_TLS_CALL         91  ///< TLS call
#define R_ARM_TLS_DESCSEQ      92  ///< TLS descriptor sequence
#define R_ARM_THM_TLS_CALL     93  ///< Thumb TLS call
#define R_ARM_PLT32_ABS        94  ///< PLT entry (S + A) | T
#define R_ARM_GOT_ABS          95  ///< GOT entry S + A
#define R_ARM_GOT_PREL         96  ///< PC relative GOT S + A - P
#define R_ARM_GOT_BREL12       97  ///< GOT offset B(S) + A - GOT_ORG
#define R_ARM_GOTOFF12         98  ///< 12 bit GOT offset ((S + A) | T) - GOT_ORG
#define R_ARM_GOTRELAX         99  ///< For future expansion
#define R_ARM_GNU_VTENTRY      100 ///< Deprecated
#define R_ARM_GNU_VTINHERIT    101 ///< Deprecated
#define R_ARM_THM_JUMP11       102 ///< Thumb PC relative ((S + A) | T) - P
#define R_ARM_THM_JUMP8        103 ///< Thumb PC relative ((S + A) | T) - P
#define R_ARM_TLS_GD32         104 ///< PC relative &(GOT(S+A)-P) + (GOT(S+A)-GOT_ORG)
#define R_ARM_TLS_LDM32        105 ///< PC relative &(GOT(S+A)-P) + (GOT(S+A)-GOT_ORG)
#define R_ARM_TLS_LDO32        106 ///< Direct 32 bit S + A - TLS
#define R_ARM_TLS_IE32         107 ///< PC relative (GOT(S + A) - P) + (GOT(S + A) - GOT_ORG)
#define R_ARM_TLS_LE32         108 ///< Direct 32 bit S + A - tp
#define R_ARM_TLS_LDO12        109 ///< 12 bit S + A - TLS
#define R_ARM_TLS_LE12         110 ///< 12 bit S + A - tp
#define R_ARM_TLS_IE12GP       111 ///< 12 bit GOT(S + A) - GOT_ORG
#define R_ARM_PRIVATE_0        112 ///< Private (n = 0, 1, ...)
#define R_ARM_PRIVATE_1        113 ///< Private
#define R_ARM_PRIVATE_2        114 ///< Private
#define R_ARM_PRIVATE_3        115 ///< Private
#define R_ARM_PRIVATE_4        116 ///< Private
#define R_ARM_PRIVATE_5        117 ///< Private
#define R_ARM_PRIVATE_6        118 ///< Private
#define R_ARM_PRIVATE_7        119 ///< Private
#define R_ARM_PRIVATE_8        120 ///< Private
#define R_ARM_PRIVATE_9        121 ///< Private
#define R_ARM_PRIVATE_10       122 ///< Private
#define R_ARM_PRIVATE_11       123 ///< Private
#define R_ARM_PRIVATE_12       124 ///< Private
#define R_ARM_PRIVATE_13       125 ///< Private
#define R_ARM_PRIVATE_14       126 ///< Private
#define R_ARM_PRIVATE_15       127 ///< Private
#define R_ARM_ME_TOO           128 ///< Obsolete
#define R_ARM_THM_TLS_DESCSEQ16 129 ///< Thumb TLS descriptor 16-bit
#define R_ARM_THM_TLS_DESCSEQ32 130 ///< Thumb TLS descriptor 32-bit
#define R_ARM_IRELATIVE        160 ///< Adjust indirectly by program base

//
// AArch64 (ARM 64-bit) Relocation Types
//
#define R_AARCH64_NONE                0   ///< No relocation
#define R_AARCH64_ABS64               257 ///< Direct 64 bit
#define R_AARCH64_ABS32               258 ///< Direct 32 bit
#define R_AARCH64_ABS16               259 ///< Direct 16 bit
#define R_AARCH64_PREL64              260 ///< PC-relative 64 bit
#define R_AARCH64_PREL32              261 ///< PC-relative 32 bit
#define R_AARCH64_PREL16              262 ///< PC-relative 16 bit
#define R_AARCH64_MOVW_UABS_G0        263 ///< MOVZ G(S+A) bits 0-15
#define R_AARCH64_MOVW_UABS_G0_NC     264 ///< MOVK G(S+A) bits 0-15, no check
#define R_AARCH64_MOVW_UABS_G1        265 ///< MOVZ G(S+A) bits 16-31
#define R_AARCH64_MOVW_UABS_G1_NC     266 ///< MOVK G(S+A) bits 16-31, no check
#define R_AARCH64_MOVW_UABS_G2        267 ///< MOVZ G(S+A) bits 32-47
#define R_AARCH64_MOVW_UABS_G2_NC     268 ///< MOVK G(S+A) bits 32-47, no check
#define R_AARCH64_MOVW_UABS_G3        269 ///< MOVZ G(S+A) bits 48-63
#define R_AARCH64_MOVW_SABS_G0        270 ///< MOVN G(S+A) bits 0-15
#define R_AARCH64_MOVW_SABS_G1        271 ///< MOVN G(S+A) bits 16-31
#define R_AARCH64_MOVW_SABS_G2        272 ///< MOVN G(S+A) bits 32-47
#define R_AARCH64_LD_PREL_LO19        273 ///< PC-rel. LD imm. bits 2-20 of G(S+A)-P
#define R_AARCH64_ADR_PREL_LO21       274 ///< PC-rel. ADR imm. bits 0-20 of G(S+A)-P
#define R_AARCH64_ADR_PREL_PG_HI21    275 ///< PC-rel. ADRP imm. bits 12-32 of Page(S+A)-Page(P)
#define R_AARCH64_ADR_PREL_PG_HI21_NC 276 ///< Likewise, no overflow check
#define R_AARCH64_ADD_ABS_LO12_NC     277 ///< ADD imm. bits 0-11 of G(S+A)
#define R_AARCH64_LDST8_ABS_LO12_NC   278 ///< LDST8 bits 0-11 of G(S+A)
#define R_AARCH64_TSTBR14             279 ///< TBZ/TBNZ bits 2-15 of G(S+A)-P
#define R_AARCH64_CONDBR19            280 ///< Cond branch bits 2-20 of G(S+A)-P
#define R_AARCH64_JUMP26              282 ///< B, BL bits 2-27 of G(S+A)-P
#define R_AARCH64_CALL26              283 ///< BL bits 2-27 of G(S+A)-P
#define R_AARCH64_LDST16_ABS_LO12_NC  284 ///< LDST16 bits 1-11 of G(S+A)
#define R_AARCH64_LDST32_ABS_LO12_NC  285 ///< LDST32 bits 2-11 of G(S+A)
#define R_AARCH64_LDST64_ABS_LO12_NC  286 ///< LDST64 bits 3-11 of G(S+A)
#define R_AARCH64_LDST128_ABS_LO12_NC 299 ///< LDST128 bits 4-11 of G(S+A)
#define R_AARCH64_GOT_LD_PREL19       309 ///< PC-rel. GOT off. load bits 2-20 of G(GOT(S+A))-P
#define R_AARCH64_ADR_GOT_PAGE        311 ///< PC-rel. GOT page ADRP bits 12-32 of Page(GOT(S+A))-Page(P)
#define R_AARCH64_LD64_GOT_LO12_NC    312 ///< GOT LD64 bits 3-11 of G(GOT(S+A))
#define R_AARCH64_TLSGD_ADR_PAGE21    513 ///< ADRP bits 12-32 of Page(G(TLSGD(S+A)))-Page(P)
#define R_AARCH64_TLSGD_ADD_LO12_NC   514 ///< ADD bits 0-11 of G(TLSGD(S+A))
#define R_AARCH64_TLSIE_ADR_GOTTPREL_PAGE21 543 ///< ADRP bits 12-32 Page(G(TPREL(S+A)))-Page(P)
#define R_AARCH64_TLSIE_LD64_GOTTPREL_LO12_NC 544 ///< LD64 bits 3-11 G(TPREL(S+A))
#define R_AARCH64_TLSLE_ADD_TPREL_HI12 549 ///< ADD bits 12-23 of G(TPREL(S+A))
#define R_AARCH64_TLSLE_ADD_TPREL_LO12 550 ///< ADD bits 0-11 of G(TPREL(S+A))
#define R_AARCH64_TLSLE_ADD_TPREL_LO12_NC 551 ///< ADD bits 0-11 of G(TPREL(S+A))
#define R_AARCH64_COPY                1024 ///< Copy symbol at runtime
#define R_AARCH64_GLOB_DAT            1025 ///< Create GOT entry
#define R_AARCH64_JUMP_SLOT           1026 ///< Create PLT entry
#define R_AARCH64_RELATIVE            1027 ///< Adjust by program base
#define R_AARCH64_TLS_DTPMOD64        1028 ///< Module number, 64 bit
#define R_AARCH64_TLS_DTPREL64        1029 ///< Module-relative offset, 64 bit
#define R_AARCH64_TLS_TPREL64         1030 ///< TP-relative offset, 64 bit
#define R_AARCH64_TLSDESC             1031 ///< TLS Descriptor
#define R_AARCH64_IRELATIVE           1032 ///< Indirect relative

//
// PowerPC 32-bit Relocation Types
//
#define R_PPC_NONE                0   ///< No relocation
#define R_PPC_ADDR32              1   ///< 32bit absolute address
#define R_PPC_ADDR24              2   ///< 26bit address, 2 bits ignored
#define R_PPC_ADDR16              3   ///< 16bit absolute address
#define R_PPC_ADDR16_LO           4   ///< lower 16bit of absolute address
#define R_PPC_ADDR16_HI           5   ///< high 16bit of absolute address
#define R_PPC_ADDR16_HA           6   ///< adjusted high 16bit
#define R_PPC_ADDR14              7   ///< 16bit address, 2 bits ignored
#define R_PPC_ADDR14_BRTAKEN      8   ///< Branch predict taken
#define R_PPC_ADDR14_BRNTAKEN     9   ///< Branch predict not taken
#define R_PPC_REL24               10  ///< PC relative 26 bit
#define R_PPC_REL14               11  ///< PC relative 16 bit
#define R_PPC_REL14_BRTAKEN       12  ///< PC relative 16 bit, branch taken
#define R_PPC_REL14_BRNTAKEN      13  ///< PC relative 16 bit, branch not taken
#define R_PPC_GOT16               14  ///< 16 bit GOT entry
#define R_PPC_GOT16_LO            15  ///< Lower 16 bit GOT entry
#define R_PPC_GOT16_HI            16  ///< High 16 bit GOT entry
#define R_PPC_GOT16_HA            17  ///< Adjusted high 16 bit GOT entry
#define R_PPC_PLTREL24            18  ///< 26 bit PC relative to PLT
#define R_PPC_COPY                19  ///< Copy symbol at runtime
#define R_PPC_GLOB_DAT            20  ///< Create GOT entry
#define R_PPC_JMP_SLOT            21  ///< Create PLT entry
#define R_PPC_RELATIVE            22  ///< Adjust by program base
#define R_PPC_LOCAL24PC           23  ///< 26 bit local PC relative
#define R_PPC_UADDR32             24  ///< 32bit unaligned absolute address
#define R_PPC_UADDR16             25  ///< 16bit unaligned absolute address
#define R_PPC_REL32               26  ///< 32bit PC relative address
#define R_PPC_PLT32               27  ///< 32bit absolute PLT address
#define R_PPC_PLTREL32            28  ///< 32bit PC relative to PLT
#define R_PPC_PLT16_LO            29  ///< Lower 16 bit of PLT
#define R_PPC_PLT16_HI            30  ///< High 16 bit of PLT
#define R_PPC_PLT16_HA            31  ///< Adjusted high 16 bit of PLT
#define R_PPC_SDAREL16            32  ///< 16 bit offset in SDA
#define R_PPC_SECTOFF             33  ///< 16bit offset from beginning of section
#define R_PPC_SECTOFF_LO          34  ///< Lower 16 bit section offset
#define R_PPC_SECTOFF_HI          35  ///< High 16 bit section offset
#define R_PPC_SECTOFF_HA          36  ///< Adjusted high 16 bit section offset
#define R_PPC_TLS                 67  ///< TLS-related
#define R_PPC_DTPMOD32            68  ///< Module number, 32 bit
#define R_PPC_TPREL16             69  ///< TP-relative offset, 16 bit
#define R_PPC_TPREL16_LO          70  ///< TP-relative offset, low 16 bit
#define R_PPC_TPREL16_HI          71  ///< TP-relative offset, high 16 bit
#define R_PPC_TPREL16_HA          72  ///< TP-relative offset, adjusted high 16
#define R_PPC_TPREL32             73  ///< TP-relative offset, 32 bit
#define R_PPC_DTPREL16            74  ///< Module-relative offset, 16 bit
#define R_PPC_DTPREL16_LO         75  ///< Module-relative offset, low 16 bit
#define R_PPC_DTPREL16_HI         76  ///< Module-relative offset, high 16 bit
#define R_PPC_DTPREL16_HA         77  ///< Module-relative offset, adjusted high 16
#define R_PPC_DTPREL32            78  ///< Module-relative offset, 32 bit
#define R_PPC_GOT_TLSGD16         79  ///< GOT entry for TLSGD, 16 bit
#define R_PPC_GOT_TLSGD16_LO      80  ///< GOT entry for TLSGD, low 16 bit
#define R_PPC_GOT_TLSGD16_HI      81  ///< GOT entry for TLSGD, high 16 bit
#define R_PPC_GOT_TLSGD16_HA      82  ///< GOT entry for TLSGD, adjusted high 16
#define R_PPC_GOT_TLSLD16         83  ///< GOT entry for TLSLD, 16 bit
#define R_PPC_GOT_TLSLD16_LO      84  ///< GOT entry for TLSLD, low 16 bit
#define R_PPC_GOT_TLSLD16_HI      85  ///< GOT entry for TLSLD, high 16 bit
#define R_PPC_GOT_TLSLD16_HA      86  ///< GOT entry for TLSLD, adjusted high 16
#define R_PPC_GOT_TPREL16         87  ///< GOT entry for TP-relative, 16 bit
#define R_PPC_GOT_TPREL16_LO      88  ///< GOT entry for TP-relative, low 16 bit
#define R_PPC_GOT_TPREL16_HI      89  ///< GOT entry for TP-relative, high 16 bit
#define R_PPC_GOT_TPREL16_HA      90  ///< GOT entry for TP-relative, adjusted high 16
#define R_PPC_GOT_DTPREL16        91  ///< GOT entry for module-relative, 16 bit
#define R_PPC_GOT_DTPREL16_LO     92  ///< GOT entry for module-relative, low 16 bit
#define R_PPC_GOT_DTPREL16_HI     93  ///< GOT entry for module-relative, high 16 bit
#define R_PPC_GOT_DTPREL16_HA     94  ///< GOT entry for module-relative, adjusted high 16
#define R_PPC_IRELATIVE           248 ///< Indirect relative

//
// PowerPC 64-bit Relocation Types
//
#define R_PPC64_NONE              0   ///< No relocation
#define R_PPC64_ADDR32            1   ///< 32bit absolute address
#define R_PPC64_ADDR24            2   ///< 26bit address, 2 bits ignored
#define R_PPC64_ADDR16            3   ///< 16bit absolute address
#define R_PPC64_ADDR16_LO         4   ///< lower 16bit of absolute address
#define R_PPC64_ADDR16_HI         5   ///< high 16bit of absolute address
#define R_PPC64_ADDR16_HA         6   ///< adjusted high 16bit
#define R_PPC64_ADDR14            7   ///< 16bit address, 2 bits ignored
#define R_PPC64_ADDR14_BRTAKEN    8   ///< Branch predict taken
#define R_PPC64_ADDR14_BRNTAKEN   9   ///< Branch predict not taken
#define R_PPC64_REL24             10  ///< PC relative 26 bit
#define R_PPC64_REL14             11  ///< PC relative 16 bit
#define R_PPC64_REL14_BRTAKEN     12  ///< PC relative 16 bit, branch taken
#define R_PPC64_REL14_BRNTAKEN    13  ///< PC relative 16 bit, branch not taken
#define R_PPC64_GOT16             14  ///< 16 bit GOT entry
#define R_PPC64_GOT16_LO          15  ///< Lower 16 bit GOT entry
#define R_PPC64_GOT16_HI          16  ///< High 16 bit GOT entry
#define R_PPC64_GOT16_HA          17  ///< Adjusted high 16 bit GOT entry
#define R_PPC64_COPY              19  ///< Copy symbol at runtime
#define R_PPC64_GLOB_DAT          20  ///< Create GOT entry
#define R_PPC64_JMP_SLOT          21  ///< Create PLT entry
#define R_PPC64_RELATIVE          22  ///< Adjust by program base
#define R_PPC64_UADDR32           24  ///< 32bit unaligned absolute address
#define R_PPC64_UADDR16           25  ///< 16bit unaligned absolute address
#define R_PPC64_REL32             26  ///< 32bit PC relative address
#define R_PPC64_ADDR64            38  ///< 64bit absolute address
#define R_PPC64_ADDR16_HIGHER     39  ///< High 16 bits of 64-bit address
#define R_PPC64_ADDR16_HIGHERA    40  ///< Adjusted high 16 bits of 64-bit address
#define R_PPC64_ADDR16_HIGHEST    41  ///< Highest 16 bits of 64-bit address
#define R_PPC64_ADDR16_HIGHESTA   42  ///< Adjusted highest 16 bits of 64-bit address
#define R_PPC64_UADDR64           43  ///< 64bit unaligned absolute address
#define R_PPC64_REL64             44  ///< 64bit PC relative address
#define R_PPC64_TOC16             47  ///< 16 bit GOT pointer offset
#define R_PPC64_TOC16_LO          48  ///< Lower 16 bit GOT pointer offset
#define R_PPC64_TOC16_HI          49  ///< High 16 bit GOT pointer offset
#define R_PPC64_TOC16_HA          50  ///< Adjusted high 16 bit GOT pointer offset
#define R_PPC64_TOC               51  ///< GOT pointer
#define R_PPC64_PLTGOT16          52  ///< PLT GOT 16 bit offset
#define R_PPC64_PLTGOT16_LO       53  ///< PLT GOT low 16 bit offset
#define R_PPC64_PLTGOT16_HI       54  ///< PLT GOT high 16 bit offset
#define R_PPC64_PLTGOT16_HA       55  ///< PLT GOT adjusted high 16 bit offset
#define R_PPC64_ADDR16_DS         56  ///< 16bit address, 2 bits ignored
#define R_PPC64_ADDR16_LO_DS      57  ///< Lower 16bit of address, 2 bits ignored
#define R_PPC64_GOT16_DS          58  ///< 16 bit GOT entry, 2 bits ignored
#define R_PPC64_GOT16_LO_DS       59  ///< Lower 16 bit GOT entry, 2 bits ignored
#define R_PPC64_TOC16_DS          63  ///< 16 bit GOT pointer offset, 2 bits ignored
#define R_PPC64_TOC16_LO_DS       64  ///< Lower 16 bit GOT pointer offset, 2 bits ignored
#define R_PPC64_TLS               67  ///< TLS-related
#define R_PPC64_DTPMOD64          68  ///< Module number, 64 bit
#define R_PPC64_TPREL16           69  ///< TP-relative offset, 16 bit
#define R_PPC64_TPREL16_LO        70  ///< TP-relative offset, low 16 bit
#define R_PPC64_TPREL16_HI        71  ///< TP-relative offset, high 16 bit
#define R_PPC64_TPREL16_HA        72  ///< TP-relative offset, adjusted high 16
#define R_PPC64_TPREL64           73  ///< TP-relative offset, 64 bit
#define R_PPC64_DTPREL16          74  ///< Module-relative offset, 16 bit
#define R_PPC64_DTPREL16_LO       75  ///< Module-relative offset, low 16 bit
#define R_PPC64_DTPREL16_HI       76  ///< Module-relative offset, high 16 bit
#define R_PPC64_DTPREL16_HA       77  ///< Module-relative offset, adjusted high 16
#define R_PPC64_DTPREL64          78  ///< Module-relative offset, 64 bit
#define R_PPC64_GOT_TLSGD16       79  ///< GOT entry for TLSGD, 16 bit
#define R_PPC64_GOT_TLSGD16_LO    80  ///< GOT entry for TLSGD, low 16 bit
#define R_PPC64_GOT_TLSGD16_HI    81  ///< GOT entry for TLSGD, high 16 bit
#define R_PPC64_GOT_TLSGD16_HA    82  ///< GOT entry for TLSGD, adjusted high 16
#define R_PPC64_GOT_TLSLD16       83  ///< GOT entry for TLSLD, 16 bit
#define R_PPC64_GOT_TLSLD16_LO    84  ///< GOT entry for TLSLD, low 16 bit
#define R_PPC64_GOT_TLSLD16_HI    85  ///< GOT entry for TLSLD, high 16 bit
#define R_PPC64_GOT_TLSLD16_HA    86  ///< GOT entry for TLSLD, adjusted high 16
#define R_PPC64_GOT_TPREL16_DS    87  ///< GOT entry for TP-relative, 16 bit, 2 bits ignored
#define R_PPC64_GOT_TPREL16_LO_DS 88  ///< GOT entry for TP-relative, low 16 bit, 2 bits ignored
#define R_PPC64_GOT_TPREL16_HI    89  ///< GOT entry for TP-relative, high 16 bit
#define R_PPC64_GOT_TPREL16_HA    90  ///< GOT entry for TP-relative, adjusted high 16
#define R_PPC64_GOT_DTPREL16_DS   91  ///< GOT entry for module-relative, 16 bit, 2 bits ignored
#define R_PPC64_GOT_DTPREL16_LO_DS 92  ///< GOT entry for module-relative, low 16 bit, 2 bits ignored
#define R_PPC64_GOT_DTPREL16_HI   93  ///< GOT entry for module-relative, high 16 bit
#define R_PPC64_GOT_DTPREL16_HA   94  ///< GOT entry for module-relative, adjusted high 16
#define R_PPC64_TPREL16_DS        95  ///< TP-relative offset, 16 bit, 2 bits ignored
#define R_PPC64_TPREL16_LO_DS     96  ///< TP-relative offset, low 16 bit, 2 bits ignored
#define R_PPC64_TPREL16_HIGHER    97  ///< TP-relative offset, high 16 bits of 64-bit value
#define R_PPC64_TPREL16_HIGHERA   98  ///< TP-relative offset, adjusted high 16 bits of 64-bit value
#define R_PPC64_TPREL16_HIGHEST   99  ///< TP-relative offset, highest 16 bits of 64-bit value
#define R_PPC64_TPREL16_HIGHESTA  100 ///< TP-relative offset, adjusted highest 16 bits of 64-bit value
#define R_PPC64_DTPREL16_DS       101 ///< Module-relative offset, 16 bit, 2 bits ignored
#define R_PPC64_DTPREL16_LO_DS    102 ///< Module-relative offset, low 16 bit, 2 bits ignored
#define R_PPC64_DTPREL16_HIGHER   103 ///< Module-relative offset, high 16 bits of 64-bit value
#define R_PPC64_DTPREL16_HIGHERA  104 ///< Module-relative offset, adjusted high 16 bits of 64-bit value
#define R_PPC64_DTPREL16_HIGHEST  105 ///< Module-relative offset, highest 16 bits of 64-bit value
#define R_PPC64_DTPREL16_HIGHESTA 106 ///< Module-relative offset, adjusted highest 16 bits of 64-bit value
#define R_PPC64_TLSGD             107 ///< TLS general dynamic
#define R_PPC64_TLSLD             108 ///< TLS local dynamic
#define R_PPC64_TOCSAVE           109 ///< TOC save
#define R_PPC64_IRELATIVE         248 ///< Indirect relative
#define R_PPC64_REL16             249 ///< PC relative 16 bit
#define R_PPC64_REL16_LO          250 ///< PC relative 16 bit, low
#define R_PPC64_REL16_HI          251 ///< PC relative 16 bit, high
#define R_PPC64_REL16_HA          252 ///< PC relative 16 bit, adjusted high

//
// MIPS Relocation Types
//
#define R_MIPS_NONE               0   ///< No relocation
#define R_MIPS_16                 1   ///< Direct 16 bit
#define R_MIPS_32                 2   ///< Direct 32 bit
#define R_MIPS_REL32              3   ///< PC relative 32 bit
#define R_MIPS_26                 4   ///< Direct 26 bit shifted
#define R_MIPS_HI16               5   ///< High 16 bit
#define R_MIPS_LO16               6   ///< Low 16 bit
#define R_MIPS_GPREL16            7   ///< GP relative 16 bit
#define R_MIPS_LITERAL            8   ///< 16 bit literal entry
#define R_MIPS_GOT16              9   ///< 16 bit GOT entry
#define R_MIPS_PC16               10  ///< PC relative 16 bit
#define R_MIPS_CALL16             11  ///< 16 bit GOT entry for function
#define R_MIPS_GPREL32            12  ///< GP relative 32 bit
#define R_MIPS_SHIFT5             16  ///< 5 bit shift
#define R_MIPS_SHIFT6             17  ///< 6 bit shift
#define R_MIPS_64                 18  ///< Direct 64 bit
#define R_MIPS_GOT_DISP           19  ///< GOT displacement
#define R_MIPS_GOT_PAGE           20  ///< GOT page
#define R_MIPS_GOT_OFST           21  ///< GOT offset
#define R_MIPS_GOT_HI16           22  ///< GOT HI 16 bit
#define R_MIPS_GOT_LO16           23  ///< GOT LO 16 bit
#define R_MIPS_SUB                24  ///< Subtraction
#define R_MIPS_INSERT_A           25  ///< Insert A
#define R_MIPS_INSERT_B           26  ///< Insert B
#define R_MIPS_DELETE             27  ///< Delete
#define R_MIPS_HIGHER             28  ///< Higher 16 bits
#define R_MIPS_HIGHEST            29  ///< Highest 16 bits
#define R_MIPS_CALL_HI16          30  ///< Call HI 16 bit
#define R_MIPS_CALL_LO16          31  ///< Call LO 16 bit
#define R_MIPS_SCN_DISP           32  ///< Section displacement
#define R_MIPS_REL16              33  ///< Relative 16 bit
#define R_MIPS_ADD_IMMEDIATE      34  ///< Add immediate
#define R_MIPS_PJUMP              35  ///< Predicted jump
#define R_MIPS_RELGOT             36  ///< Relative GOT
#define R_MIPS_JALR               37  ///< JAL to register
#define R_MIPS_TLS_DTPMOD32       38  ///< Module number 32 bit
#define R_MIPS_TLS_DTPREL32       39  ///< Module-relative offset 32 bit
#define R_MIPS_TLS_DTPMOD64       40  ///< Module number 64 bit
#define R_MIPS_TLS_DTPREL64       41  ///< Module-relative offset 64 bit
#define R_MIPS_TLS_GD             42  ///< 16 bit GOT offset for GD
#define R_MIPS_TLS_LDM            43  ///< 16 bit GOT offset for LDM
#define R_MIPS_TLS_DTPREL_HI16    44  ///< Module-relative offset, high 16 bits
#define R_MIPS_TLS_DTPREL_LO16    45  ///< Module-relative offset, low 16 bits
#define R_MIPS_TLS_GOTTPREL       46  ///< 16 bit GOT offset for IE
#define R_MIPS_TLS_TPREL32        47  ///< TP-relative offset, 32 bit
#define R_MIPS_TLS_TPREL64        48  ///< TP-relative offset, 64 bit
#define R_MIPS_TLS_TPREL_HI16     49  ///< TP-relative offset, high 16 bits
#define R_MIPS_TLS_TPREL_LO16     50  ///< TP-relative offset, low 16 bits
#define R_MIPS_GLOB_DAT           51  ///< Create GOT entry
#define R_MIPS_COPY               126 ///< Copy symbol at runtime
#define R_MIPS_JUMP_SLOT          127 ///< Create PLT entry

//
// SPARC Relocation Types
//
#define R_SPARC_NONE              0   ///< No relocation
#define R_SPARC_8                 1   ///< Direct 8 bit
#define R_SPARC_16                2   ///< Direct 16 bit
#define R_SPARC_32                3   ///< Direct 32 bit
#define R_SPARC_DISP8             4   ///< PC relative 8 bit
#define R_SPARC_DISP16            5   ///< PC relative 16 bit
#define R_SPARC_DISP32            6   ///< PC relative 32 bit
#define R_SPARC_WDISP30           7   ///< PC relative 30 bit shifted
#define R_SPARC_WDISP22           8   ///< PC relative 22 bit shifted
#define R_SPARC_HI22              9   ///< High 22 bit
#define R_SPARC_22                10  ///< Direct 22 bit
#define R_SPARC_13                11  ///< Direct 13 bit
#define R_SPARC_LO10              12  ///< Low 10 bit
#define R_SPARC_GOT10             13  ///< GOT 10 bit
#define R_SPARC_GOT13             14  ///< GOT 13 bit
#define R_SPARC_GOT22             15  ///< GOT 22 bit
#define R_SPARC_PC10              16  ///< PC relative 10 bit
#define R_SPARC_PC22              17  ///< PC relative 22 bit
#define R_SPARC_WPLT30            18  ///< 30 bit PC relative PLT address
#define R_SPARC_COPY              19  ///< Copy symbol at runtime
#define R_SPARC_GLOB_DAT          20  ///< Create GOT entry
#define R_SPARC_JMP_SLOT          21  ///< Create PLT entry
#define R_SPARC_RELATIVE          22  ///< Adjust by program base
#define R_SPARC_UA32              23  ///< Direct 32 bit unaligned
#define R_SPARC_PLT32             24  ///< Direct 32 bit to PLT entry
#define R_SPARC_HIPLT22           25  ///< High 22 bit PLT entry
#define R_SPARC_LOPLT10           26  ///< Low 10 bit PLT entry
#define R_SPARC_PCPLT32           27  ///< PC relative 32 bit to PLT entry
#define R_SPARC_PCPLT22           28  ///< PC relative 22 bit to PLT entry
#define R_SPARC_PCPLT10           29  ///< PC relative 10 bit to PLT entry
#define R_SPARC_10                30  ///< Direct 10 bit
#define R_SPARC_11                31  ///< Direct 11 bit
#define R_SPARC_64                32  ///< Direct 64 bit
#define R_SPARC_OLO10             33  ///< 10 bit with secondary addend
#define R_SPARC_HH22              34  ///< Top 22 bits of 64 bit value
#define R_SPARC_HM10              35  ///< High middle 10 bits of 64 bit value
#define R_SPARC_LM22              36  ///< Low middle 22 bits of 64 bit value
#define R_SPARC_PC_HH22           37  ///< Top 22 bits of pc relative 64 bit
#define R_SPARC_PC_HM10           38  ///< High middle 10 bit of pc relative 64 bit
#define R_SPARC_PC_LM22           39  ///< Low middle 22 bits of pc relative 64 bit
#define R_SPARC_WDISP16           40  ///< PC relative 16 bit shifted
#define R_SPARC_WDISP19           41  ///< PC relative 19 bit shifted
#define R_SPARC_7                 43  ///< Direct 7 bit
#define R_SPARC_5                 44  ///< Direct 5 bit
#define R_SPARC_6                 45  ///< Direct 6 bit
#define R_SPARC_DISP64            46  ///< PC relative 64 bit
#define R_SPARC_PLT64             47  ///< Direct 64 bit to PLT entry
#define R_SPARC_HIX22             48  ///< High 22 bit complemented
#define R_SPARC_LOX10             49  ///< Low 10 bit complemented
#define R_SPARC_H44               50  ///< High 22 bit
#define R_SPARC_M44               51  ///< Middle 10 bit
#define R_SPARC_L44               52  ///< Low 10 bit
#define R_SPARC_REGISTER          53  ///< SETHI of register number
#define R_SPARC_UA64              54  ///< Direct 64 bit unaligned
#define R_SPARC_UA16              55  ///< Direct 16 bit unaligned
#define R_SPARC_TLS_GD_HI22       56  ///< High 22 bit GD GOT offset
#define R_SPARC_TLS_GD_LO10       57  ///< Low 10 bit GD GOT offset
#define R_SPARC_TLS_GD_ADD        58  ///< GD add
#define R_SPARC_TLS_GD_CALL       59  ///< GD call
#define R_SPARC_TLS_LDM_HI22      60  ///< High 22 bit LDM GOT offset
#define R_SPARC_TLS_LDM_LO10      61  ///< Low 10 bit LDM GOT offset
#define R_SPARC_TLS_LDM_ADD       62  ///< LDM add
#define R_SPARC_TLS_LDM_CALL      63  ///< LDM call
#define R_SPARC_TLS_LDO_HIX22     64  ///< High 22 bit LDO offset
#define R_SPARC_TLS_LDO_LOX10     65  ///< Low 10 bit LDO offset
#define R_SPARC_TLS_LDO_ADD       66  ///< LDO add
#define R_SPARC_TLS_IE_HI22       67  ///< High 22 bit IE GOT offset
#define R_SPARC_TLS_IE_LO10       68  ///< Low 10 bit IE GOT offset
#define R_SPARC_TLS_IE_LD         69  ///< IE ld
#define R_SPARC_TLS_IE_LDX        70  ///< IE ldx
#define R_SPARC_TLS_IE_ADD        71  ///< IE add
#define R_SPARC_TLS_LE_HIX22      72  ///< High 22 bit LE offset
#define R_SPARC_TLS_LE_LOX10      73  ///< Low 10 bit LE offset
#define R_SPARC_TLS_DTPMOD32      74  ///< Module ID, 32 bit
#define R_SPARC_TLS_DTPMOD64      75  ///< Module ID, 64 bit
#define R_SPARC_TLS_DTPOFF32      76  ///< Module-relative offset, 32 bit
#define R_SPARC_TLS_DTPOFF64      77  ///< Module-relative offset, 64 bit
#define R_SPARC_TLS_TPOFF32       78  ///< TP-relative offset, 32 bit
#define R_SPARC_TLS_TPOFF64       79  ///< TP-relative offset, 64 bit

//
// RISC-V Relocation Types
//
#define R_RISCV_NONE              0   ///< No relocation
#define R_RISCV_32                1   ///< Direct 32 bit
#define R_RISCV_64                2   ///< Direct 64 bit
#define R_RISCV_RELATIVE          3   ///< Adjust by program base
#define R_RISCV_COPY              4   ///< Copy symbol at runtime
#define R_RISCV_JUMP_SLOT         5   ///< Create PLT entry
#define R_RISCV_TLS_DTPMOD32      6   ///< Module ID, 32 bit
#define R_RISCV_TLS_DTPMOD64      7   ///< Module ID, 64 bit
#define R_RISCV_TLS_DTPREL32      8   ///< Module-relative offset, 32 bit
#define R_RISCV_TLS_DTPREL64      9   ///< Module-relative offset, 64 bit
#define R_RISCV_TLS_TPREL32       10  ///< TP-relative offset, 32 bit
#define R_RISCV_TLS_TPREL64       11  ///< TP-relative offset, 64 bit
#define R_RISCV_BRANCH            16  ///< PC-relative branch
#define R_RISCV_JAL               17  ///< PC-relative jump
#define R_RISCV_CALL              18  ///< PC-relative call
#define R_RISCV_CALL_PLT          19  ///< PC-relative call (PLT)
#define R_RISCV_GOT_HI20          20  ///< PC-relative GOT reference
#define R_RISCV_TLS_GOT_HI20      21  ///< PC-relative TLS IE GOT offset
#define R_RISCV_TLS_GD_HI20       22  ///< PC-relative TLS GD GOT offset
#define R_RISCV_PCREL_HI20        23  ///< PC-relative reference
#define R_RISCV_PCREL_LO12_I      24  ///< PC-relative reference (I-type)
#define R_RISCV_PCREL_LO12_S      25  ///< PC-relative reference (S-type)
#define R_RISCV_HI20              26  ///< Absolute address
#define R_RISCV_LO12_I            27  ///< Absolute address (I-type)
#define R_RISCV_LO12_S            28  ///< Absolute address (S-type)
#define R_RISCV_TPREL_HI20        29  ///< TP-relative TLS LE reference
#define R_RISCV_TPREL_LO12_I      30  ///< TP-relative TLS LE reference (I-type)
#define R_RISCV_TPREL_LO12_S      31  ///< TP-relative TLS LE reference (S-type)
#define R_RISCV_TPREL_ADD         32  ///< TP-relative TLS LE addition
#define R_RISCV_ADD8              33  ///< 8-bit label addition
#define R_RISCV_ADD16             34  ///< 16-bit label addition
#define R_RISCV_ADD32             35  ///< 32-bit label addition
#define R_RISCV_ADD64             36  ///< 64-bit label addition
#define R_RISCV_SUB8              37  ///< 8-bit label subtraction
#define R_RISCV_SUB16             38  ///< 16-bit label subtraction
#define R_RISCV_SUB32             39  ///< 32-bit label subtraction
#define R_RISCV_SUB64             40  ///< 64-bit label subtraction
#define R_RISCV_GNU_VTINHERIT     41  ///< GNU C++ vtable hierarchy
#define R_RISCV_GNU_VTENTRY       42  ///< GNU C++ vtable member usage
#define R_RISCV_ALIGN             43  ///< Alignment statement
#define R_RISCV_RVC_BRANCH        44  ///< PC-relative branch (compressed)
#define R_RISCV_RVC_JUMP          45  ///< PC-relative jump (compressed)
#define R_RISCV_RVC_LUI           46  ///< Absolute address (compressed)
#define R_RISCV_GPREL_I           47  ///< GP-relative reference (I-type)
#define R_RISCV_GPREL_S           48  ///< GP-relative reference (S-type)
#define R_RISCV_TPREL_I           49  ///< TP-relative reference (I-type)
#define R_RISCV_TPREL_S           50  ///< TP-relative reference (S-type)
#define R_RISCV_RELAX             51  ///< Instruction pair can be relaxed
#define R_RISCV_SUB6              52  ///< Local label subtraction
#define R_RISCV_SET6              53  ///< Local label assignment
#define R_RISCV_SET8              54  ///< Local label assignment
#define R_RISCV_SET16             55  ///< Local label assignment
#define R_RISCV_SET32             56  ///< Local label assignment
#define R_RISCV_32_PCREL          57  ///< PC-relative 32 bit

//
// IA-64 Relocation Types
//
#define R_IA64_NONE               0x00 ///< No relocation
#define R_IA64_IMM14              0x21 ///< Immediate14
#define R_IA64_IMM22              0x22 ///< Immediate22
#define R_IA64_IMM64              0x23 ///< Immediate64
#define R_IA64_DIR32MSB           0x24 ///< Direct 32 MSB
#define R_IA64_DIR32LSB           0x25 ///< Direct 32 LSB
#define R_IA64_DIR64MSB           0x26 ///< Direct 64 MSB
#define R_IA64_DIR64LSB           0x27 ///< Direct 64 LSB
#define R_IA64_GPREL22            0x2a ///< GP-relative 22 bit
#define R_IA64_GPREL64I           0x2b ///< GP-relative 64 bit
#define R_IA64_GPREL32MSB         0x2c ///< GP-relative 32 MSB
#define R_IA64_GPREL32LSB         0x2d ///< GP-relative 32 LSB
#define R_IA64_GPREL64MSB         0x2e ///< GP-relative 64 MSB
#define R_IA64_GPREL64LSB         0x2f ///< GP-relative 64 LSB
#define R_IA64_LTOFF22            0x32 ///< LT-relative 22 bit
#define R_IA64_LTOFF64I           0x33 ///< LT-relative 64 bit
#define R_IA64_PLTOFF22           0x3a ///< PLT-relative 22 bit
#define R_IA64_PLTOFF64I          0x3b ///< PLT-relative 64 bit
#define R_IA64_PLTOFF64MSB        0x3e ///< PLT-relative 64 MSB
#define R_IA64_PLTOFF64LSB        0x3f ///< PLT-relative 64 LSB
#define R_IA64_FPTR64I            0x43 ///< Function pointer 64 bit
#define R_IA64_FPTR32MSB          0x44 ///< Function pointer 32 MSB
#define R_IA64_FPTR32LSB          0x45 ///< Function pointer 32 LSB
#define R_IA64_FPTR64MSB          0x46 ///< Function pointer 64 MSB
#define R_IA64_FPTR64LSB          0x47 ///< Function pointer 64 LSB
#define R_IA64_PCREL60B           0x48 ///< PC-relative 60 bit bundle
#define R_IA64_PCREL21B           0x49 ///< PC-relative 21 bit bundle
#define R_IA64_PCREL21M           0x4a ///< PC-relative 21 bit mov
#define R_IA64_PCREL21F           0x4b ///< PC-relative 21 bit fchkf
#define R_IA64_PCREL32MSB         0x4c ///< PC-relative 32 MSB
#define R_IA64_PCREL32LSB         0x4d ///< PC-relative 32 LSB
#define R_IA64_PCREL64MSB         0x4e ///< PC-relative 64 MSB
#define R_IA64_PCREL64LSB         0x4f ///< PC-relative 64 LSB
#define R_IA64_LTOFF_FPTR22       0x52 ///< LT-relative function pointer 22
#define R_IA64_LTOFF_FPTR64I      0x53 ///< LT-relative function pointer 64
#define R_IA64_LTOFF_FPTR32MSB    0x54 ///< LT-relative function pointer 32 MSB
#define R_IA64_LTOFF_FPTR32LSB    0x55 ///< LT-relative function pointer 32 LSB
#define R_IA64_LTOFF_FPTR64MSB    0x56 ///< LT-relative function pointer 64 MSB
#define R_IA64_LTOFF_FPTR64LSB    0x57 ///< LT-relative function pointer 64 LSB
#define R_IA64_SEGREL32MSB        0x5c ///< Segment-relative 32 MSB
#define R_IA64_SEGREL32LSB        0x5d ///< Segment-relative 32 LSB
#define R_IA64_SEGREL64MSB        0x5e ///< Segment-relative 64 MSB
#define R_IA64_SEGREL64LSB        0x5f ///< Segment-relative 64 LSB
#define R_IA64_SECREL32MSB        0x64 ///< Section-relative 32 MSB
#define R_IA64_SECREL32LSB        0x65 ///< Section-relative 32 LSB
#define R_IA64_SECREL64MSB        0x66 ///< Section-relative 64 MSB
#define R_IA64_SECREL64LSB        0x67 ///< Section-relative 64 LSB
#define R_IA64_REL32MSB           0x6c ///< Relative 32 MSB
#define R_IA64_REL32LSB           0x6d ///< Relative 32 LSB
#define R_IA64_REL64MSB           0x6e ///< Relative 64 MSB
#define R_IA64_REL64LSB           0x6f ///< Relative 64 LSB
#define R_IA64_LTV32MSB           0x74 ///< LTV 32 MSB
#define R_IA64_LTV32LSB           0x75 ///< LTV 32 LSB
#define R_IA64_LTV64MSB           0x76 ///< LTV 64 MSB
#define R_IA64_LTV64LSB           0x77 ///< LTV 64 LSB
#define R_IA64_PCREL21BI          0x79 ///< PC-relative 21 bit indirect
#define R_IA64_PCREL22            0x7a ///< PC-relative 22 bit
#define R_IA64_PCREL64I           0x7b ///< PC-relative 64 bit
#define R_IA64_IPLTMSB            0x80 ///< Indirect PLT MSB
#define R_IA64_IPLTLSB            0x81 ///< Indirect PLT LSB
#define R_IA64_COPY               0x84 ///< Copy symbol at runtime
#define R_IA64_SUB                0x85 ///< Subtraction
#define R_IA64_LTOFF22X           0x86 ///< LT-relative 22 bit extended
#define R_IA64_LDXMOV             0x87 ///< Load extended move
#define R_IA64_TPREL14            0x91 ///< TP-relative 14 bit
#define R_IA64_TPREL22            0x92 ///< TP-relative 22 bit
#define R_IA64_TPREL64I           0x93 ///< TP-relative 64 bit
#define R_IA64_TPREL64MSB         0x96 ///< TP-relative 64 MSB
#define R_IA64_TPREL64LSB         0x97 ///< TP-relative 64 LSB
#define R_IA64_LTOFF_TPREL22      0x9a ///< LT-relative TP-relative 22
#define R_IA64_DTPMOD64MSB        0xa6 ///< DTPMOD 64 MSB
#define R_IA64_DTPMOD64LSB        0xa7 ///< DTPMOD 64 LSB
#define R_IA64_LTOFF_DTPMOD22     0xaa ///< LT-relative DTPMOD 22
#define R_IA64_DTPREL14           0xb1 ///< DTP-relative 14 bit
#define R_IA64_DTPREL22           0xb2 ///< DTP-relative 22 bit
#define R_IA64_DTPREL64I          0xb3 ///< DTP-relative 64 bit
#define R_IA64_DTPREL32MSB        0xb4 ///< DTP-relative 32 MSB
#define R_IA64_DTPREL32LSB        0xb5 ///< DTP-relative 32 LSB
#define R_IA64_DTPREL64MSB        0xb6 ///< DTP-relative 64 MSB
#define R_IA64_DTPREL64LSB        0xb7 ///< DTP-relative 64 LSB
#define R_IA64_LTOFF_DTPREL22     0xba ///< LT-relative DTP-relative 22

//
// Alpha Relocation Types
//
#define R_ALPHA_NONE              0   ///< No relocation
#define R_ALPHA_REFLONG           1   ///< Direct 32 bit
#define R_ALPHA_REFQUAD           2   ///< Direct 64 bit
#define R_ALPHA_GPREL32           3   ///< GP relative 32 bit
#define R_ALPHA_LITERAL           4   ///< GP relative 16 bit w/optimization
#define R_ALPHA_LITUSE            5   ///< Optimization hint for LITERAL
#define R_ALPHA_GPDISP            6   ///< Add displacement to GP
#define R_ALPHA_BRADDR            7   ///< PC+4 relative 23 bit shifted
#define R_ALPHA_HINT              8   ///< PC+4 relative 16 bit shifted
#define R_ALPHA_SREL16            9   ///< PC relative 16 bit
#define R_ALPHA_SREL32            10  ///< PC relative 32 bit
#define R_ALPHA_SREL64            11  ///< PC relative 64 bit
#define R_ALPHA_GPRELHIGH         17  ///< GP relative 32 bit, high 16 bits
#define R_ALPHA_GPRELLOW          18  ///< GP relative 32 bit, low 16 bits
#define R_ALPHA_GPREL16           19  ///< GP relative 16 bit
#define R_ALPHA_COPY              24  ///< Copy symbol at runtime
#define R_ALPHA_GLOB_DAT          25  ///< Create GOT entry
#define R_ALPHA_JMP_SLOT          26  ///< Create PLT entry
#define R_ALPHA_RELATIVE          27  ///< Adjust by program base
#define R_ALPHA_TLS_GD_HI         28  ///< High 16 bit TLS GD offset
#define R_ALPHA_TLSLDM            29  ///< Local dynamic TLS
#define R_ALPHA_DTPMOD64          30  ///< Module number 64 bit
#define R_ALPHA_GOTDTPREL         31  ///< GOT offset for DTPREL
#define R_ALPHA_DTPREL64          32  ///< Module-relative offset 64 bit
#define R_ALPHA_DTPRELHI          33  ///< High 16 bit DTPREL
#define R_ALPHA_DTPRELLO          34  ///< Low 16 bit DTPREL
#define R_ALPHA_DTPREL16          35  ///< 16 bit DTPREL
#define R_ALPHA_GOTTPREL          36  ///< GOT offset for TPREL
#define R_ALPHA_TPREL64           37  ///< TP-relative offset 64 bit
#define R_ALPHA_TPRELHI           38  ///< High 16 bit TPREL
#define R_ALPHA_TPRELLO           39  ///< Low 16 bit TPREL
#define R_ALPHA_TPREL16           40  ///< 16 bit TPREL

//
// M68K Relocation Types
//
#define R_68K_NONE                0   ///< No relocation
#define R_68K_32                  1   ///< Direct 32 bit
#define R_68K_16                  2   ///< Direct 16 bit
#define R_68K_8                   3   ///< Direct 8 bit
#define R_68K_PC32                4   ///< PC relative 32 bit
#define R_68K_PC16                5   ///< PC relative 16 bit
#define R_68K_PC8                 6   ///< PC relative 8 bit
#define R_68K_GOT32               7   ///< 32 bit GOT entry
#define R_68K_GOT16               8   ///< 16 bit GOT entry
#define R_68K_GOT8                9   ///< 8 bit GOT entry
#define R_68K_GOT32O              10  ///< 32 bit GOT offset
#define R_68K_GOT16O              11  ///< 16 bit GOT offset
#define R_68K_GOT8O               12  ///< 8 bit GOT offset
#define R_68K_PLT32               13  ///< 32 bit PLT address
#define R_68K_PLT16               14  ///< 16 bit PLT address
#define R_68K_PLT8                15  ///< 8 bit PLT address
#define R_68K_PLT32O              16  ///< 32 bit PLT offset
#define R_68K_PLT16O              17  ///< 16 bit PLT offset
#define R_68K_PLT8O               18  ///< 8 bit PLT offset
#define R_68K_COPY                19  ///< Copy symbol at runtime
#define R_68K_GLOB_DAT            20  ///< Create GOT entry
#define R_68K_JMP_SLOT            21  ///< Create PLT entry
#define R_68K_RELATIVE            22  ///< Adjust by program base
#define R_68K_TLS_GD32            25  ///< 32 bit TLS GD offset
#define R_68K_TLS_GD16            26  ///< 16 bit TLS GD offset
#define R_68K_TLS_GD8             27  ///< 8 bit TLS GD offset
#define R_68K_TLS_LDM32           28  ///< 32 bit TLS LDM offset
#define R_68K_TLS_LDM16           29  ///< 16 bit TLS LDM offset
#define R_68K_TLS_LDM8            30  ///< 8 bit TLS LDM offset
#define R_68K_TLS_LDO32           31  ///< 32 bit module-relative offset
#define R_68K_TLS_LDO16           32  ///< 16 bit module-relative offset
#define R_68K_TLS_LDO8            33  ///< 8 bit module-relative offset
#define R_68K_TLS_IE32            34  ///< 32 bit GOT offset for IE
#define R_68K_TLS_IE16            35  ///< 16 bit GOT offset for IE
#define R_68K_TLS_IE8             36  ///< 8 bit GOT offset for IE
#define R_68K_TLS_LE32            37  ///< 32 bit TP-relative offset
#define R_68K_TLS_LE16            38  ///< 16 bit TP-relative offset
#define R_68K_TLS_LE8             39  ///< 8 bit TP-relative offset
#define R_68K_TLS_DTPMOD32        40  ///< 32 bit module number
#define R_68K_TLS_DTPREL32        41  ///< 32 bit module-relative offset
#define R_68K_TLS_TPREL32         42  ///< 32 bit TP-relative offset

//
// S/390 Relocation Types
//
#define R_390_NONE                0   ///< No relocation
#define R_390_8                   1   ///< Direct 8 bit
#define R_390_12                  2   ///< Direct 12 bit
#define R_390_16                  3   ///< Direct 16 bit
#define R_390_32                  4   ///< Direct 32 bit
#define R_390_PC32                5   ///< PC relative 32 bit
#define R_390_GOT12               6   ///< 12 bit GOT offset
#define R_390_GOT32               7   ///< 32 bit GOT offset
#define R_390_PLT32               8   ///< 32 bit PLT address
#define R_390_COPY                9   ///< Copy symbol at runtime
#define R_390_GLOB_DAT            10  ///< Create GOT entry
#define R_390_JMP_SLOT            11  ///< Create PLT entry
#define R_390_RELATIVE            12  ///< Adjust by program base
#define R_390_GOTOFF32            13  ///< 32 bit offset to GOT
#define R_390_GOTPC               14  ///< 32 bit PC relative offset to GOT
#define R_390_GOT16               15  ///< 16 bit GOT offset
#define R_390_PC16                16  ///< PC relative 16 bit
#define R_390_PC16DBL             17  ///< PC relative 16 bit shifted by 1
#define R_390_PLT16DBL            18  ///< 16 bit PLT address shifted by 1
#define R_390_PC32DBL             19  ///< PC relative 32 bit shifted by 1
#define R_390_PLT32DBL            20  ///< 32 bit PLT address shifted by 1
#define R_390_GOTPCDBL            21  ///< 32 bit PC rel GOT offset shifted by 1
#define R_390_64                  22  ///< Direct 64 bit
#define R_390_PC64                23  ///< PC relative 64 bit
#define R_390_GOT64               24  ///< 64 bit GOT offset
#define R_390_PLT64               25  ///< 64 bit PLT address
#define R_390_GOTENT              26  ///< 32 bit PC rel to GOT entry >> 1
#define R_390_GOTOFF16            27  ///< 16 bit offset to GOT
#define R_390_GOTOFF64            28  ///< 64 bit offset to GOT
#define R_390_GOTPLT12            29  ///< 12 bit offset to jump slot
#define R_390_GOTPLT16            30  ///< 16 bit offset to jump slot
#define R_390_GOTPLT32            31  ///< 32 bit offset to jump slot
#define R_390_GOTPLT64            32  ///< 64 bit offset to jump slot
#define R_390_GOTPLTENT           33  ///< 32 bit rel offset to jump slot
#define R_390_PLTOFF16            34  ///< 16 bit offset from GOT to PLT
#define R_390_PLTOFF32            35  ///< 32 bit offset from GOT to PLT
#define R_390_PLTOFF64            36  ///< 64 bit offset from GOT to PLT
#define R_390_TLS_LOAD            37  ///< TLS load insn tag
#define R_390_TLS_GDCALL          38  ///< TLS gd call insn tag
#define R_390_TLS_LDCALL          39  ///< TLS ld call insn tag
#define R_390_TLS_GD32            40  ///< 32 bit TLS GD offset
#define R_390_TLS_GD64            41  ///< 64 bit TLS GD offset
#define R_390_TLS_GOTIE12         42  ///< 12 bit TLS IE GOT offset
#define R_390_TLS_GOTIE32         43  ///< 32 bit TLS IE GOT offset
#define R_390_TLS_GOTIE64         44  ///< 64 bit TLS IE GOT offset
#define R_390_TLS_LDM32           45  ///< 32 bit TLS LDM offset
#define R_390_TLS_LDM64           46  ///< 64 bit TLS LDM offset
#define R_390_TLS_IE32            47  ///< 32 bit TLS IE offset
#define R_390_TLS_IE64            48  ///< 64 bit TLS IE offset
#define R_390_TLS_IEENT           49  ///< 32 bit TLS IE entry
#define R_390_TLS_LE32            50  ///< 32 bit TLS LE offset
#define R_390_TLS_LE64            51  ///< 64 bit TLS LE offset
#define R_390_TLS_LDO32           52  ///< 32 bit TLS LD offset
#define R_390_TLS_LDO64           53  ///< 64 bit TLS LD offset
#define R_390_TLS_DTPMOD          54  ///< Module ID
#define R_390_TLS_DTPOFF          55  ///< Module-relative offset
#define R_390_TLS_TPOFF           56  ///< TP-relative offset
#define R_390_20                  57  ///< Direct 20 bit
#define R_390_GOT20               58  ///< 20 bit GOT offset
#define R_390_GOTPLT20            59  ///< 20 bit GOTPLT offset
#define R_390_TLS_GOTIE20         60  ///< 20 bit TLS IE GOT offset

//
// VAX Relocation Types
//
#define R_VAX_NONE                0   ///< No relocation
#define R_VAX_32                  1   ///< Direct 32 bit
#define R_VAX_16                  2   ///< Direct 16 bit
#define R_VAX_8                   3   ///< Direct 8 bit
#define R_VAX_PC32                4   ///< PC relative 32 bit
#define R_VAX_PC16                5   ///< PC relative 16 bit
#define R_VAX_PC8                 6   ///< PC relative 8 bit
#define R_VAX_GOT32               7   ///< 32 bit GOT entry
#define R_VAX_PLT32               13  ///< 32 bit PLT address
#define R_VAX_COPY                19  ///< Copy symbol at runtime
#define R_VAX_GLOB_DAT            20  ///< Create GOT entry
#define R_VAX_JMP_SLOT            21  ///< Create PLT entry
#define R_VAX_RELATIVE            22  ///< Adjust by program base

//
// SH (SuperH) Relocation Types
//
#define R_SH_NONE                 0   ///< No relocation
#define R_SH_DIR32                1   ///< Direct 32 bit
#define R_SH_REL32                2   ///< PC relative 32 bit
#define R_SH_DIR8WPN              3   ///< 8 bit PC relative branch div 2
#define R_SH_IND12W               4   ///< 12 bit PC relative branch div 2
#define R_SH_DIR8WPL              5   ///< 8 bit unsigned PC relative div 4
#define R_SH_DIR8WPZ              6   ///< 8 bit unsigned PC relative div 2
#define R_SH_DIR8BP               7   ///< 8 bit GBR relative
#define R_SH_DIR8W                8   ///< 8 bit GBR relative div 2
#define R_SH_DIR8L                9   ///< 8 bit GBR relative div 4
#define R_SH_SWITCH16             25  ///< 16 bit switch table
#define R_SH_SWITCH32             26  ///< 32 bit switch table
#define R_SH_USES                 27  ///< USES pseudo-op
#define R_SH_COUNT                28  ///< COUNT pseudo-op
#define R_SH_ALIGN                29  ///< ALIGN pseudo-op
#define R_SH_CODE                 30  ///< CODE pseudo-op
#define R_SH_DATA                 31  ///< DATA pseudo-op
#define R_SH_LABEL                32  ///< LABEL pseudo-op
#define R_SH_SWITCH8              33  ///< 8 bit switch table
#define R_SH_GNU_VTINHERIT        34  ///< GNU vtable hierarchy
#define R_SH_GNU_VTENTRY          35  ///< GNU vtable member usage
#define R_SH_TLS_GD_32            144 ///< TLS GD 32 bit
#define R_SH_TLS_LD_32            145 ///< TLS LD 32 bit
#define R_SH_TLS_LDO_32           146 ///< TLS LDO 32 bit
#define R_SH_TLS_IE_32            147 ///< TLS IE 32 bit
#define R_SH_TLS_LE_32            148 ///< TLS LE 32 bit
#define R_SH_TLS_DTPMOD32         149 ///< TLS DTPMOD 32 bit
#define R_SH_TLS_DTPOFF32         150 ///< TLS DTPOFF 32 bit
#define R_SH_TLS_TPOFF32          151 ///< TLS TPOFF 32 bit
#define R_SH_GOT32                160 ///< GOT 32 bit
#define R_SH_PLT32                161 ///< PLT 32 bit
#define R_SH_COPY                 162 ///< Copy symbol at runtime
#define R_SH_GLOB_DAT             163 ///< Create GOT entry
#define R_SH_JMP_SLOT             164 ///< Create PLT entry
#define R_SH_RELATIVE             165 ///< Adjust by program base
#define R_SH_GOTOFF               166 ///< Offset to GOT
#define R_SH_GOTPC                167 ///< PC relative offset to GOT

//
// AVR Relocation Types
//
#define R_AVR_NONE                0   ///< No relocation
#define R_AVR_32                  1   ///< Direct 32 bit
#define R_AVR_7_PCREL             2   ///< PC relative 7 bit
#define R_AVR_13_PCREL            3   ///< PC relative 13 bit
#define R_AVR_16                  4   ///< Direct 16 bit
#define R_AVR_16_PM               5   ///< Direct 16 bit (program memory)
#define R_AVR_LO8_LDI             6   ///< Low 8 bit of 16 bit value
#define R_AVR_HI8_LDI             7   ///< High 8 bit of 16 bit value
#define R_AVR_HH8_LDI             8   ///< High 8 bit of 24 bit value
#define R_AVR_LO8_LDI_NEG         9   ///< Low 8 bit negated
#define R_AVR_HI8_LDI_NEG         10  ///< High 8 bit negated
#define R_AVR_HH8_LDI_NEG         11  ///< High 8 bit of 24 bit negated
#define R_AVR_LO8_LDI_PM          12  ///< Low 8 bit (program memory)
#define R_AVR_HI8_LDI_PM          13  ///< High 8 bit (program memory)
#define R_AVR_HH8_LDI_PM          14  ///< High 8 bit of 24 bit (PM)
#define R_AVR_LO8_LDI_PM_NEG      15  ///< Low 8 bit negated (PM)
#define R_AVR_HI8_LDI_PM_NEG      16  ///< High 8 bit negated (PM)
#define R_AVR_HH8_LDI_PM_NEG      17  ///< High 8 bit of 24 bit negated (PM)
#define R_AVR_CALL                18  ///< Direct call
#define R_AVR_LDI                 19  ///< Load immediate
#define R_AVR_6                   20  ///< Direct 6 bit
#define R_AVR_6_ADIW              21  ///< Direct 6 bit ADIW
#define R_AVR_MS8_LDI             22  ///< Mid-high 8 bit of 32 bit value
#define R_AVR_MS8_LDI_NEG         23  ///< Mid-high 8 bit negated
#define R_AVR_LO8_LDI_GS          24  ///< Low 8 bit (GOT)
#define R_AVR_HI8_LDI_GS          25  ///< High 8 bit (GOT)
#define R_AVR_8                   26  ///< Direct 8 bit
#define R_AVR_8_LO8               27  ///< Low 8 bit (byte addressing)
#define R_AVR_8_HI8               28  ///< High 8 bit (byte addressing)
#define R_AVR_8_HLO8              29  ///< High low 8 bit (byte addressing)
#define R_AVR_DIFF8               30  ///< Difference 8 bit
#define R_AVR_DIFF16              31  ///< Difference 16 bit
#define R_AVR_DIFF32              32  ///< Difference 32 bit
#define R_AVR_LDS_STS_16          33  ///< LDS/STS 16 bit

//
// Xtensa Relocation Types
//
#define R_XTENSA_NONE             0   ///< No relocation
#define R_XTENSA_32               1   ///< Direct 32 bit
#define R_XTENSA_RTLD             2   ///< Runtime linker
#define R_XTENSA_GLOB_DAT         3   ///< Create GOT entry
#define R_XTENSA_JMP_SLOT         4   ///< Create PLT entry
#define R_XTENSA_RELATIVE         5   ///< Adjust by program base
#define R_XTENSA_PLT              6   ///< 32 bit PLT address
#define R_XTENSA_OP0              8   ///< Operand 0 relocation
#define R_XTENSA_OP1              9   ///< Operand 1 relocation
#define R_XTENSA_OP2              10  ///< Operand 2 relocation
#define R_XTENSA_ASM_EXPAND       11  ///< Assembly expansion
#define R_XTENSA_ASM_SIMPLIFY     12  ///< Assembly simplification
#define R_XTENSA_32_PCREL         14  ///< PC relative 32 bit
#define R_XTENSA_GNU_VTINHERIT    15  ///< GNU vtable hierarchy
#define R_XTENSA_GNU_VTENTRY      16  ///< GNU vtable member usage
#define R_XTENSA_DIFF8            17  ///< Difference 8 bit
#define R_XTENSA_DIFF16           18  ///< Difference 16 bit
#define R_XTENSA_DIFF32           19  ///< Difference 32 bit
#define R_XTENSA_SLOT0_OP         20  ///< Slot 0 operation
#define R_XTENSA_SLOT1_OP         21  ///< Slot 1 operation
#define R_XTENSA_SLOT2_OP         22  ///< Slot 2 operation
#define R_XTENSA_SLOT3_OP         23  ///< Slot 3 operation
#define R_XTENSA_SLOT4_OP         24  ///< Slot 4 operation
#define R_XTENSA_SLOT5_OP         25  ///< Slot 5 operation
#define R_XTENSA_SLOT6_OP         26  ///< Slot 6 operation
#define R_XTENSA_SLOT7_OP         27  ///< Slot 7 operation
#define R_XTENSA_SLOT8_OP         28  ///< Slot 8 operation
#define R_XTENSA_SLOT9_OP         29  ///< Slot 9 operation
#define R_XTENSA_SLOT10_OP        30  ///< Slot 10 operation
#define R_XTENSA_SLOT11_OP        31  ///< Slot 11 operation
#define R_XTENSA_SLOT12_OP        32  ///< Slot 12 operation
#define R_XTENSA_SLOT13_OP        33  ///< Slot 13 operation
#define R_XTENSA_SLOT14_OP        34  ///< Slot 14 operation
#define R_XTENSA_SLOT0_ALT        35  ///< Slot 0 alternate
#define R_XTENSA_SLOT1_ALT        36  ///< Slot 1 alternate
#define R_XTENSA_SLOT2_ALT        37  ///< Slot 2 alternate
#define R_XTENSA_SLOT3_ALT        38  ///< Slot 3 alternate
#define R_XTENSA_SLOT4_ALT        39  ///< Slot 4 alternate
#define R_XTENSA_SLOT5_ALT        40  ///< Slot 5 alternate
#define R_XTENSA_SLOT6_ALT        41  ///< Slot 6 alternate
#define R_XTENSA_SLOT7_ALT        42  ///< Slot 7 alternate
#define R_XTENSA_SLOT8_ALT        43  ///< Slot 8 alternate
#define R_XTENSA_SLOT9_ALT        44  ///< Slot 9 alternate
#define R_XTENSA_SLOT10_ALT       45  ///< Slot 10 alternate
#define R_XTENSA_SLOT11_ALT       46  ///< Slot 11 alternate
#define R_XTENSA_SLOT12_ALT       47  ///< Slot 12 alternate
#define R_XTENSA_SLOT13_ALT       48  ///< Slot 13 alternate
#define R_XTENSA_SLOT14_ALT       49  ///< Slot 14 alternate
#define R_XTENSA_TLSDESC_FN       50  ///< TLS descriptor function
#define R_XTENSA_TLSDESC_ARG      51  ///< TLS descriptor argument
#define R_XTENSA_TLS_DTPOFF       52  ///< TLS DTPOFF
#define R_XTENSA_TLS_TPOFF        53  ///< TLS TPOFF
#define R_XTENSA_TLS_FUNC         54  ///< TLS function
#define R_XTENSA_TLS_ARG          55  ///< TLS argument
#define R_XTENSA_TLS_CALL         56  ///< TLS call

//
// OpenRISC Relocation Types
//
#define R_OR1K_NONE               0   ///< No relocation
#define R_OR1K_32                 1   ///< Direct 32 bit
#define R_OR1K_16                 2   ///< Direct 16 bit
#define R_OR1K_8                  3   ///< Direct 8 bit
#define R_OR1K_LO_16_IN_INSN      4   ///< Low 16 bit in instruction
#define R_OR1K_HI_16_IN_INSN      5   ///< High 16 bit in instruction
#define R_OR1K_INSN_REL_26        6   ///< PC relative 26 bit
#define R_OR1K_GNU_VTENTRY        7   ///< GNU vtable member usage
#define R_OR1K_GNU_VTINHERIT      8   ///< GNU vtable hierarchy
#define R_OR1K_32_PCREL           9   ///< PC relative 32 bit
#define R_OR1K_16_PCREL           10  ///< PC relative 16 bit
#define R_OR1K_8_PCREL            11  ///< PC relative 8 bit
#define R_OR1K_GOTPC_HI16         12  ///< PC relative GOT high 16
#define R_OR1K_GOTPC_LO16         13  ///< PC relative GOT low 16
#define R_OR1K_GOT16              14  ///< GOT 16 bit
#define R_OR1K_PLT26              15  ///< PLT 26 bit
#define R_OR1K_GOTOFF_HI16        16  ///< GOT offset high 16
#define R_OR1K_GOTOFF_LO16        17  ///< GOT offset low 16
#define R_OR1K_COPY               18  ///< Copy symbol at runtime
#define R_OR1K_GLOB_DAT           19  ///< Create GOT entry
#define R_OR1K_JMP_SLOT           20  ///< Create PLT entry
#define R_OR1K_RELATIVE           21  ///< Adjust by program base
#define R_OR1K_TLS_GD_HI16        22  ///< TLS GD high 16
#define R_OR1K_TLS_GD_LO16        23  ///< TLS GD low 16
#define R_OR1K_TLS_LDM_HI16       24  ///< TLS LDM high 16
#define R_OR1K_TLS_LDM_LO16       25  ///< TLS LDM low 16
#define R_OR1K_TLS_LDO_HI16       26  ///< TLS LDO high 16
#define R_OR1K_TLS_LDO_LO16       27  ///< TLS LDO low 16
#define R_OR1K_TLS_IE_HI16        28  ///< TLS IE high 16
#define R_OR1K_TLS_IE_LO16        29  ///< TLS IE low 16
#define R_OR1K_TLS_LE_HI16        30  ///< TLS LE high 16
#define R_OR1K_TLS_LE_LO16        31  ///< TLS LE low 16
#define R_OR1K_TLS_TPOFF          32  ///< TLS TPOFF
#define R_OR1K_TLS_DTPOFF         33  ///< TLS DTPOFF
#define R_OR1K_TLS_DTPMOD         34  ///< TLS DTPMOD

/**
  Convert ELF machine type to BINFORMAT_MACHINE.

  @param[in]  ElfMachine  ELF machine type.

  @return BINFORMAT_MACHINE type.

**/
STATIC
BINFORMAT_MACHINE
ElfMachineToGeneric (
  IN  UINT16  ElfMachine
  )
{
  switch (ElfMachine) {
    case EM_MMIX:    return BinMachineMMIX;
    case EM_386:     return BinMachineX86;
    case EM_X86_64:  return BinMachineX64;
    case EM_ARM:     return BinMachineARM;
    case EM_AARCH64: return BinMachineARM64;
    case EM_PPC:     return BinMachinePowerPC;
    case EM_PPC64:   return BinMachinePowerPC64;
    case EM_MIPS:    return BinMachineMIPS;
    case EM_SPARC:   return BinMachineSPARC;
    case EM_SPARCV9: return BinMachineSPARC64;
    case EM_RISCV:   return BinMachineRISCV64; // Context determines 32/64
    case EM_IA_64:   return BinMachineIA64;
    case EM_ALPHA:   return BinMachineAlpha;
    case EM_68K:     return BinMachineM68K;
    case EM_VAX:     return BinMachineVAX;
    case EM_S390:    return BinMachineS390;
    case EM_WE32K:    return BinMachineWE32K;
    case EM_NS32K:    return BinMachineNS32K;
    case EM_TAHOE:    return BinMachineTahoe;
    case EM_PDP10:    return BinMachinePDP10;
    case EM_PDP11:    return BinMachinePDP11;
    case EM_88K:      return BinMachineM88K;
    case EM_DLX:      return BinMachineDLX;
    case EM_MOXIE:    return BinMachineMoxie;
    case EM_LOONGARCH: return BinMachineLoongArch64; // Context determines 32/64
    case EM_CONVEX:   return BinMachineConvex;
    case EM_PYRAMID:  return BinMachinePyramid;
    case EM_CRAY:     return BinMachineCray;
    case EM_HPFOCUS:  return BinMachineHPFocus;
    case EM_EBC:      return BinMachineEBC;
    case EM_8086:     return BinMachineI8086;
    case EM_80286:    return BinMachineI80286;
    case EM_29K:      return BinMachineAM29K;
    case EM_AVR:      return BinMachineAVR;
    case EM_AVR32:    return BinMachineAVR32;
    case EM_NIOS2:    return BinMachineNios2;
    case EM_MICROBLAZE: return BinMachineMicroBlaze;
    case EM_OPENRISC: return BinMachineOpenRISC;
    case EM_MSP430:   return BinMachineMSP430;
    case EM_LANAI:    return BinMachineLanai;
    case EM_ELBRUS:   return BinMachineElbrus2K;
    case EM_CLIPPER:  return BinMachineClipper;
    case EM_BPF:      return BinMachineBPF;
    case EM_HEXAGON:  return BinMachineHexagon;
    case EM_CSKY:     return BinMachineCSKY;
    case EM_FR30:     return BinMachineFR30;
    case EM_MN10200:  return BinMachineMN10200;
    case EM_MN10300:  return BinMachineMN10300;
    case EM_FRV:      return BinMachineFRV;
    case EM_NECVE:    return BinMachineNECVE;
    case EM_IP2K:     return BinMachineIP2K;
    case EM_IQ2000:   return BinMachineIQ2000;
    case EM_CRIS:     return BinMachineCRIS;
    case EM_ARC:      return BinMachineARC;
    case EM_SH:       return BinMachineSuperH;
    case EM_PARISC:   return BinMachinePARISC;
    case EM_CR16:     return BinMachineCR16;
    case EM_D10V:     return BinMachineD10V;
    case EM_D30V:     return BinMachineD30V;
    case EM_XTENSA:   return BinMachineXtensa;
    case EM_860:      return BinMachineI860;
    case EM_960:      return BinMachineI960;
    case EM_TI_C6000: return BinMachineTIC6X;
    case EM_TI_C5500: return BinMachineTIC55X;
    case EM_TI_C5400: return BinMachineTIC54X;
    case EM_BLACKFIN: return BinMachineBlackfin;
    case EM_EPIPHANY: return BinMachineEpiphany;
    case EM_M32R:     return BinMachineM32R;
    case EM_M32C:     return BinMachineM32C;
    case EM_LATTICEMICO32: return BinMachineLM32;
    case EM_NDS32:    return BinMachineNDS32;
    case EM_V850:     return BinMachineV850;
    case EM_TILEPRO:  return BinMachineTILE;
    case EM_Z80:      return BinMachineZ80;
    case EM_Z8000:    return BinMachineZ8000;
    case EM_6502:     return BinMachine6502;
    case EM_65816:    return BinMachine65816;
    case EM_MCORE:    return BinMachineMCore;
    case EM_TRICORE:  return BinMachineTriCore;
    case EM_MEP:      return BinMachineMEP;
    case EM_PICOJAVA: return BinMachinePicoJava;
    case EM_MN103:    return BinMachineAM33;
    case EM_METAG:    return BinMachineMetag;
    case EM_SCORE:    return BinMachineSCORE;
    case EM_RL78:     return BinMachineRL78;
    case EM_RX:       return BinMachineRX;
    case EM_VISIUM:   return BinMachineVisium;
    case EM_XGATE:    return BinMachineXGATE;
    case EM_XSTORMY16: return BinMachineXStormy16;
    case EM_KVX:      return BinMachineKVX;
    case EM_NFP:      return BinMachineNFP;
    case EM_MT:       return BinMachineMT;
    case EM_FT32:     return BinMachineFT32;
    case EM_H8_300:   return BinMachineH8300;
    case EM_SPU:      return BinMachineSPU;
    case EM_WEBASSEMBLY: return BinMachineWASM;
    case EM_PRU:      return BinMachinePRU;
    default:         return BinMachineUnknown;
  }
}

/**
  Convert ELF file type to BINFORMAT_FILE_TYPE.

  @param[in]  ElfType  ELF file type.

  @return BINFORMAT_FILE_TYPE.

**/
STATIC
BINFORMAT_FILE_TYPE
ElfTypeToGeneric (
  IN  UINT16  ElfType
  )
{
  switch (ElfType) {
    case ET_REL:  return BinFileTypeRelocatable;
    case ET_EXEC: return BinFileTypeExecutable;
    case ET_DYN:  return BinFileTypeSharedLibrary;
    case ET_CORE: return BinFileTypeCore;
    default:      return BinFileTypeUnknown;
  }
}

/**
  Initialize ELF context from file.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FilePath   Path to ELF file.
  @param[in]   ReadOnly   TRUE for read-only access.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfInitFile (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  )
{
  FILE          *File;
  ELF_CONTEXT   *ElfCtx;
  UINT64        FileSize;

  if (Context == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  File = fopen(FilePath, ReadOnly ? "rb" : "r+b");
  if (File == NULL) {
    return BINFORMAT_ERROR_IO;
  }

  //
  // Get file size
  //
  fseek(File, 0, SEEK_END);
  FileSize = ftell(File);
  fseek(File, 0, SEEK_SET);

  //
  // Allocate context
  //
  ElfCtx = (ELF_CONTEXT *)calloc(1, sizeof(ELF_CONTEXT));
  if (ElfCtx == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Allocate file buffer
  //
  ElfCtx->FileData = (UINT8 *)malloc(FileSize);
  if (ElfCtx->FileData == NULL) {
    free(ElfCtx);
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Read file
  //
  if (fread(ElfCtx->FileData, 1, FileSize, File) != FileSize) {
    free(ElfCtx->FileData);
    free(ElfCtx);
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);

  ElfCtx->FileSize = FileSize;
  ElfCtx->OwnBuffer = TRUE;
  ElfCtx->ReadOnly = ReadOnly;

  //
  // Check for FatELF
  //
  if (FileSize >= sizeof(FatELF_Header)) {
    FatELF_Header *FatHdr = (FatELF_Header *)ElfCtx->FileData;
    if (FatHdr->magic == FATELF_MAGIC) {
      ElfCtx->IsFat = TRUE;
      ElfCtx->FatHeader = FatHdr;
      ElfCtx->FatRecords = (FatELF_Record *)(ElfCtx->FileData + sizeof(FatELF_Header));
      ElfCtx->SelectedArch = 0;

      if (FatHdr->num_records > 0) {
        ElfCtx->CurrentOffset = ElfCtx->FatRecords[0].offset;
      }
    }
  }

  //
  // Parse ELF header
  //
  UINT8 *ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;

  if (ElfData[EI_MAG0] != ELF_MAGIC_0 ||
      ElfData[EI_MAG1] != ELF_MAGIC_1 ||
      ElfData[EI_MAG2] != ELF_MAGIC_2 ||
      ElfData[EI_MAG3] != ELF_MAGIC_3) {
    free(ElfCtx->FileData);
    free(ElfCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  ElfCtx->Is64Bit = (ElfData[EI_CLASS] == ELFCLASS64);

  //
  // Determine if byte-swapping is needed
  //
  BOOLEAN FileIsLittleEndian = (ElfData[EI_DATA] == ELFDATA2LSB);
  ElfCtx->NeedsByteSwap = (FileIsLittleEndian != IsLittleEndianHost());

  if (ElfCtx->Is64Bit) {
    ElfCtx->Header.Elf64 = (Elf64_Ehdr *)ElfData;
    ElfCtx->Sections.Elf64 = (Elf64_Shdr *)(ElfData + SWAP64(ElfCtx, ElfCtx->Header.Elf64->e_shoff));
    ElfCtx->Programs.Elf64 = (Elf64_Phdr *)(ElfData + SWAP64(ElfCtx, ElfCtx->Header.Elf64->e_phoff));

    //
    // Get string table
    //
    UINT16 shstrndx = SWAP16(ElfCtx, ElfCtx->Header.Elf64->e_shstrndx);
    UINT16 shnum = SWAP16(ElfCtx, ElfCtx->Header.Elf64->e_shnum);
    if (shstrndx < shnum) {
      UINT64 strtab_offset = SWAP64(ElfCtx, ElfCtx->Sections.Elf64[shstrndx].sh_offset);
      ElfCtx->StringTable = (CHAR8 *)(ElfData + strtab_offset);
    }
  } else {
    ElfCtx->Header.Elf32 = (Elf32_Ehdr *)ElfData;
    ElfCtx->Sections.Elf32 = (Elf32_Shdr *)(ElfData + SWAP32(ElfCtx, ElfCtx->Header.Elf32->e_shoff));
    ElfCtx->Programs.Elf32 = (Elf32_Phdr *)(ElfData + SWAP32(ElfCtx, ElfCtx->Header.Elf32->e_phoff));

    //
    // Get string table
    //
    UINT16 shstrndx = SWAP16(ElfCtx, ElfCtx->Header.Elf32->e_shstrndx);
    UINT16 shnum = SWAP16(ElfCtx, ElfCtx->Header.Elf32->e_shnum);
    if (shstrndx < shnum) {
      UINT32 strtab_offset = SWAP32(ElfCtx, ElfCtx->Sections.Elf32[shstrndx].sh_offset);
      ElfCtx->StringTable = (CHAR8 *)(ElfData + strtab_offset);
    }
  }

  *Context = (BINFORMAT_CONTEXT *)ElfCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Initialize ELF context from memory buffer.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   Buffer     Pointer to ELF data.
  @param[in]   Size       Size of ELF data.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfInitMemory (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST VOID         *Buffer,
  IN  UINT64             Size
  )
{
  ELF_CONTEXT   *ElfCtx;
  UINT8         *ElfData;
  BOOLEAN       FileIsLittleEndian;

  if (Context == NULL || Buffer == NULL || Size < sizeof(Elf32_Ehdr)) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate context
  //
  ElfCtx = (ELF_CONTEXT *)calloc(1, sizeof(ELF_CONTEXT));
  if (ElfCtx == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  ElfCtx->FileData = (UINT8 *)Buffer;
  ElfCtx->FileSize = Size;
  ElfCtx->OwnBuffer = FALSE;
  ElfCtx->ReadOnly = TRUE;

  //
  // Check for FatELF
  //
  if (Size >= sizeof(FatELF_Header)) {
    FatELF_Header *FatHdr = (FatELF_Header *)ElfCtx->FileData;
    if (FatHdr->magic == FATELF_MAGIC) {
      ElfCtx->IsFat = TRUE;
      ElfCtx->FatHeader = FatHdr;
      ElfCtx->FatRecords = (FatELF_Record *)(ElfCtx->FileData + sizeof(FatELF_Header));
      ElfCtx->SelectedArch = 0;

      if (FatHdr->num_records > 0) {
        ElfCtx->CurrentOffset = ElfCtx->FatRecords[0].offset;
      }
    }
  }

  //
  // Parse ELF header
  //
  ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;

  if (ElfData[EI_MAG0] != ELF_MAGIC_0 ||
      ElfData[EI_MAG1] != ELF_MAGIC_1 ||
      ElfData[EI_MAG2] != ELF_MAGIC_2 ||
      ElfData[EI_MAG3] != ELF_MAGIC_3) {
    free(ElfCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  ElfCtx->Is64Bit = (ElfData[EI_CLASS] == ELFCLASS64);

  //
  // Determine if byte-swapping is needed
  //
  FileIsLittleEndian = (ElfData[EI_DATA] == ELFDATA2LSB);
  ElfCtx->NeedsByteSwap = (FileIsLittleEndian != IsLittleEndianHost());

  if (ElfCtx->Is64Bit) {
    ElfCtx->Header.Elf64 = (Elf64_Ehdr *)ElfData;
    ElfCtx->Sections.Elf64 = (Elf64_Shdr *)(ElfData + SWAP64(ElfCtx, ElfCtx->Header.Elf64->e_shoff));
    ElfCtx->Programs.Elf64 = (Elf64_Phdr *)(ElfData + SWAP64(ElfCtx, ElfCtx->Header.Elf64->e_phoff));

    UINT16 shstrndx = SWAP16(ElfCtx, ElfCtx->Header.Elf64->e_shstrndx);
    UINT16 shnum = SWAP16(ElfCtx, ElfCtx->Header.Elf64->e_shnum);
    if (shstrndx < shnum) {
      UINT64 strtab_offset = SWAP64(ElfCtx, ElfCtx->Sections.Elf64[shstrndx].sh_offset);
      ElfCtx->StringTable = (CHAR8 *)(ElfData + strtab_offset);
    }
  } else {
    ElfCtx->Header.Elf32 = (Elf32_Ehdr *)ElfData;
    ElfCtx->Sections.Elf32 = (Elf32_Shdr *)(ElfData + SWAP32(ElfCtx, ElfCtx->Header.Elf32->e_shoff));
    ElfCtx->Programs.Elf32 = (Elf32_Phdr *)(ElfData + SWAP32(ElfCtx, ElfCtx->Header.Elf32->e_phoff));

    UINT16 shstrndx = SWAP16(ElfCtx, ElfCtx->Header.Elf32->e_shstrndx);
    UINT16 shnum = SWAP16(ElfCtx, ElfCtx->Header.Elf32->e_shnum);
    if (shstrndx < shnum) {
      UINT32 strtab_offset = SWAP32(ElfCtx, ElfCtx->Sections.Elf32[shstrndx].sh_offset);
      ElfCtx->StringTable = (CHAR8 *)(ElfData + strtab_offset);
    }
  }

  *Context = (BINFORMAT_CONTEXT *)ElfCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Create a new ELF context for writing.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FileType   Type of binary to create.
  @param[in]   Machine    Target machine.
  @param[in]   Is64Bit    TRUE for 64-bit format.

  @retval BINFORMAT_SUCCESS       Context created successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfCreate (
  OUT BINFORMAT_CONTEXT   **Context,
  IN  BINFORMAT_FILE_TYPE FileType,
  IN  BINFORMAT_MACHINE   Machine,
  IN  BOOLEAN             Is64Bit
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT64       InitialSize;

  if (Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate context
  //
  ElfCtx = (ELF_CONTEXT *)calloc(1, sizeof(ELF_CONTEXT));
  if (ElfCtx == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  ElfCtx->Is64Bit = Is64Bit;
  ElfCtx->ReadOnly = FALSE;
  ElfCtx->OwnBuffer = TRUE;

  //
  // Allocate initial buffer
  //
  InitialSize = Is64Bit ? sizeof(Elf64_Ehdr) : sizeof(Elf32_Ehdr);
  ElfCtx->FileData = (UINT8 *)calloc(1, InitialSize);
  if (ElfCtx->FileData == NULL) {
    free(ElfCtx);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  ElfCtx->FileSize = InitialSize;

  //
  // Initialize ELF header
  //
  if (Is64Bit) {
    Elf64_Ehdr *Hdr = (Elf64_Ehdr *)ElfCtx->FileData;
    Hdr->e_ident[EI_MAG0] = ELF_MAGIC_0;
    Hdr->e_ident[EI_MAG1] = ELF_MAGIC_1;
    Hdr->e_ident[EI_MAG2] = ELF_MAGIC_2;
    Hdr->e_ident[EI_MAG3] = ELF_MAGIC_3;
    Hdr->e_ident[EI_CLASS] = ELFCLASS64;
    Hdr->e_ident[EI_DATA] = ELFDATA2LSB;
    Hdr->e_ident[EI_VERSION] = EV_CURRENT;
    Hdr->e_version = EV_CURRENT;
    Hdr->e_ehsize = sizeof(Elf64_Ehdr);
    Hdr->e_phentsize = sizeof(Elf64_Phdr);
    Hdr->e_shentsize = sizeof(Elf64_Shdr);
    ElfCtx->Header.Elf64 = Hdr;
  } else {
    Elf32_Ehdr *Hdr = (Elf32_Ehdr *)ElfCtx->FileData;
    Hdr->e_ident[EI_MAG0] = ELF_MAGIC_0;
    Hdr->e_ident[EI_MAG1] = ELF_MAGIC_1;
    Hdr->e_ident[EI_MAG2] = ELF_MAGIC_2;
    Hdr->e_ident[EI_MAG3] = ELF_MAGIC_3;
    Hdr->e_ident[EI_CLASS] = ELFCLASS32;
    Hdr->e_ident[EI_DATA] = ELFDATA2LSB;
    Hdr->e_ident[EI_VERSION] = EV_CURRENT;
    Hdr->e_version = EV_CURRENT;
    Hdr->e_ehsize = sizeof(Elf32_Ehdr);
    Hdr->e_phentsize = sizeof(Elf32_Phdr);
    Hdr->e_shentsize = sizeof(Elf32_Shdr);
    ElfCtx->Header.Elf32 = Hdr;
  }

  *Context = (BINFORMAT_CONTEXT *)ElfCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Close and free an ELF context.

  @param[in]  Context    Context to close.

**/
STATIC
VOID
ElfClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  ELF_CONTEXT  *ElfCtx;

  if (Context == NULL) {
    return;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (ElfCtx->OwnBuffer && ElfCtx->FileData != NULL) {
    free(ElfCtx->FileData);
  }

  free(ElfCtx);
}

/**
  Get ELF header information.

  @param[in]   Context      ELF context.
  @param[out]  HeaderInfo   Pointer to receive header information.

  @retval BINFORMAT_SUCCESS       Header information retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetHeader (
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  )
{
  ELF_CONTEXT  *ElfCtx;

  if (Context == NULL || HeaderInfo == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);
  memset(HeaderInfo, 0, sizeof(BINFORMAT_HEADER_INFO));

  HeaderInfo->Is64Bit = ElfCtx->Is64Bit;

  if (ElfCtx->Is64Bit) {
    Elf64_Ehdr *Hdr = ElfCtx->Header.Elf64;
    HeaderInfo->FileType = ElfTypeToGeneric(SWAP16(ElfCtx, Hdr->e_type));
    HeaderInfo->Machine = ElfMachineToGeneric(SWAP16(ElfCtx, Hdr->e_machine));
    HeaderInfo->Endianness = (Hdr->e_ident[EI_DATA] == ELFDATA2LSB) ?
                             BinEndianLittle : BinEndianBig;
    HeaderInfo->Version = SWAP32(ElfCtx, Hdr->e_version);
    HeaderInfo->EntryPoint = SWAP64(ElfCtx, Hdr->e_entry);
    HeaderInfo->Flags = SWAP32(ElfCtx, Hdr->e_flags);
    HeaderInfo->SectionCount = SWAP16(ElfCtx, Hdr->e_shnum);
    HeaderInfo->SegmentCount = SWAP16(ElfCtx, Hdr->e_phnum);
  } else {
    Elf32_Ehdr *Hdr = ElfCtx->Header.Elf32;
    HeaderInfo->FileType = ElfTypeToGeneric(SWAP16(ElfCtx, Hdr->e_type));
    HeaderInfo->Machine = ElfMachineToGeneric(SWAP16(ElfCtx, Hdr->e_machine));
    HeaderInfo->Endianness = (Hdr->e_ident[EI_DATA] == ELFDATA2LSB) ?
                             BinEndianLittle : BinEndianBig;
    HeaderInfo->Version = SWAP32(ElfCtx, Hdr->e_version);
    HeaderInfo->EntryPoint = SWAP32(ElfCtx, Hdr->e_entry);
    HeaderInfo->Flags = SWAP32(ElfCtx, Hdr->e_flags);
    HeaderInfo->SectionCount = SWAP16(ElfCtx, Hdr->e_shnum);
    HeaderInfo->SegmentCount = SWAP16(ElfCtx, Hdr->e_phnum);
  }

  //
  // Handle FatELF
  //
  if (ElfCtx->IsFat && ElfCtx->FatHeader != NULL) {
    HeaderInfo->ArchitectureCount = ElfCtx->FatHeader->num_records;
    for (UINT32 i = 0; i < ElfCtx->FatHeader->num_records && i < BINFORMAT_MAX_ARCHITECTURES; i++) {
      HeaderInfo->Architectures[i].Machine = ElfMachineToGeneric(ElfCtx->FatRecords[i].machine);
      HeaderInfo->Architectures[i].Offset = ElfCtx->FatRecords[i].offset;
      HeaderInfo->Architectures[i].Size = ElfCtx->FatRecords[i].size;
      HeaderInfo->Architectures[i].CpuSubtype = ElfCtx->FatRecords[i].osabi;
    }
  }

  return BINFORMAT_SUCCESS;
}

/**
  Get section information by index.

  @param[in]   Context      ELF context.
  @param[in]   Index        Section index.
  @param[out]  Section      Pointer to receive section information.

  @retval BINFORMAT_SUCCESS       Section information retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetSection (
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  OUT BINFORMAT_SECTION   *Section
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT32       SectionCount;

  if (Context == NULL || Section == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  SectionCount = ElfCtx->Is64Bit ? SWAP16(ElfCtx, ElfCtx->Header.Elf64->e_shnum) :
                                   SWAP16(ElfCtx, ElfCtx->Header.Elf32->e_shnum);

  if (Index >= SectionCount) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  memset(Section, 0, sizeof(BINFORMAT_SECTION));

  if (ElfCtx->Is64Bit) {
    Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[Index];

    UINT32 sh_name = SWAP32(ElfCtx, Shdr->sh_name);
    if (ElfCtx->StringTable != NULL && sh_name < 0x10000) {
      strncpy(Section->Name, ElfCtx->StringTable + sh_name,
              BINFORMAT_MAX_SECTION_NAME - 1);
    }

    Section->Type = SWAP32(ElfCtx, Shdr->sh_type);
    Section->Flags = SWAP64(ElfCtx, Shdr->sh_flags);
    Section->VirtualAddress = SWAP64(ElfCtx, Shdr->sh_addr);
    Section->FileOffset = SWAP64(ElfCtx, Shdr->sh_offset);
    Section->Size = SWAP64(ElfCtx, Shdr->sh_size);
    Section->Link = SWAP32(ElfCtx, Shdr->sh_link);
    Section->Info = SWAP32(ElfCtx, Shdr->sh_info);
    Section->Alignment = SWAP64(ElfCtx, Shdr->sh_addralign);
    Section->EntrySize = SWAP64(ElfCtx, Shdr->sh_entsize);
    Section->Data = ElfCtx->FileData + ElfCtx->CurrentOffset + SWAP64(ElfCtx, Shdr->sh_offset);
  } else {
    Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[Index];

    UINT32 sh_name = SWAP32(ElfCtx, Shdr->sh_name);
    if (ElfCtx->StringTable != NULL && sh_name < 0x10000) {
      strncpy(Section->Name, ElfCtx->StringTable + sh_name,
              BINFORMAT_MAX_SECTION_NAME - 1);
    }

    Section->Type = SWAP32(ElfCtx, Shdr->sh_type);
    Section->Flags = SWAP32(ElfCtx, Shdr->sh_flags);
    Section->VirtualAddress = SWAP32(ElfCtx, Shdr->sh_addr);
    Section->FileOffset = SWAP32(ElfCtx, Shdr->sh_offset);
    Section->Size = SWAP32(ElfCtx, Shdr->sh_size);
    Section->Link = SWAP32(ElfCtx, Shdr->sh_link);
    Section->Info = SWAP32(ElfCtx, Shdr->sh_info);
    Section->Alignment = SWAP32(ElfCtx, Shdr->sh_addralign);
    Section->EntrySize = SWAP32(ElfCtx, Shdr->sh_entsize);
    Section->Data = ElfCtx->FileData + ElfCtx->CurrentOffset + SWAP32(ElfCtx, Shdr->sh_offset);
  }

  return BINFORMAT_SUCCESS;
}

/**
  Get section information by name.

  @param[in]   Context      ELF context.
  @param[in]   Name         Section name.
  @param[out]  Section      Pointer to receive section information.

  @retval BINFORMAT_SUCCESS       Section found and retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetSectionByName (
  IN  BINFORMAT_CONTEXT   *Context,
  IN  CONST CHAR8         *Name,
  OUT BINFORMAT_SECTION   *Section
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT32       SectionCount;
  UINT32       i;

  if (Context == NULL || Name == NULL || Section == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  for (i = 0; i < SectionCount; i++) {
    BINFORMAT_SECTION TempSection;
    BINFORMAT_STATUS Status;

    Status = ElfGetSection(Context, i, &TempSection);
    if (!BINFORMAT_IS_ERROR(Status)) {
      if (strcmp(TempSection.Name, Name) == 0) {
        memcpy(Section, &TempSection, sizeof(BINFORMAT_SECTION));
        return BINFORMAT_SUCCESS;
      }
    }
  }

  return BINFORMAT_ERROR_NOT_FOUND;
}

/**
  Get segment information by index.

  @param[in]   Context      ELF context.
  @param[in]   Index        Segment index.
  @param[out]  Segment      Pointer to receive segment information.

  @retval BINFORMAT_SUCCESS       Segment information retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetSegment (
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              Index,
  OUT BINFORMAT_SEGMENT   *Segment
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT32       SegmentCount;

  if (Context == NULL || Segment == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  SegmentCount = ElfCtx->Is64Bit ? SWAP16(ElfCtx, ElfCtx->Header.Elf64->e_phnum) :
                                   SWAP16(ElfCtx, ElfCtx->Header.Elf32->e_phnum);

  if (Index >= SegmentCount) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  memset(Segment, 0, sizeof(BINFORMAT_SEGMENT));

  if (ElfCtx->Is64Bit) {
    Elf64_Phdr *Phdr = &ElfCtx->Programs.Elf64[Index];
    Segment->Type = SWAP32(ElfCtx, Phdr->p_type);
    Segment->Flags = SWAP32(ElfCtx, Phdr->p_flags);
    Segment->FileOffset = SWAP64(ElfCtx, Phdr->p_offset);
    Segment->VirtualAddress = SWAP64(ElfCtx, Phdr->p_vaddr);
    Segment->PhysicalAddress = SWAP64(ElfCtx, Phdr->p_paddr);
    Segment->FileSize = SWAP64(ElfCtx, Phdr->p_filesz);
    Segment->MemorySize = SWAP64(ElfCtx, Phdr->p_memsz);
    Segment->Alignment = SWAP64(ElfCtx, Phdr->p_align);
  } else {
    Elf32_Phdr *Phdr = &ElfCtx->Programs.Elf32[Index];
    Segment->Type = SWAP32(ElfCtx, Phdr->p_type);
    Segment->Flags = SWAP32(ElfCtx, Phdr->p_flags);
    Segment->FileOffset = SWAP32(ElfCtx, Phdr->p_offset);
    Segment->VirtualAddress = SWAP32(ElfCtx, Phdr->p_vaddr);
    Segment->PhysicalAddress = SWAP32(ElfCtx, Phdr->p_paddr);
    Segment->FileSize = SWAP32(ElfCtx, Phdr->p_filesz);
    Segment->MemorySize = SWAP32(ElfCtx, Phdr->p_memsz);
    Segment->Alignment = SWAP32(ElfCtx, Phdr->p_align);
  }

  return BINFORMAT_SUCCESS;
}

/**
  Find and initialize symbol table in ELF context.

  @param[in]  ElfCtx  ELF context.

  @retval BINFORMAT_SUCCESS       Symbol table found and initialized.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfFindSymbolTable (
  IN  ELF_CONTEXT  *ElfCtx
  )
{
  UINT32  SectionCount;
  UINT32  i;

  if (ElfCtx->Symbols.Elf32 != NULL || ElfCtx->Symbols.Elf64 != NULL) {
    return BINFORMAT_SUCCESS; // Already initialized
  }

  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  for (i = 0; i < SectionCount; i++) {
    UINT32  sh_type;
    UINT32  sh_link;
    UINT64  sh_offset;
    UINT64  sh_size;
    UINT64  sh_entsize;

    if (ElfCtx->Is64Bit) {
      Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[i];
      sh_type = Shdr->sh_type;
      sh_link = Shdr->sh_link;
      sh_offset = Shdr->sh_offset;
      sh_size = Shdr->sh_size;
      sh_entsize = Shdr->sh_entsize;
    } else {
      Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[i];
      sh_type = Shdr->sh_type;
      sh_link = Shdr->sh_link;
      sh_offset = Shdr->sh_offset;
      sh_size = Shdr->sh_size;
      sh_entsize = Shdr->sh_entsize;
    }

    if (sh_type == SHT_SYMTAB || sh_type == SHT_DYNSYM) {
      UINT8 *ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;

      if (ElfCtx->Is64Bit) {
        ElfCtx->Symbols.Elf64 = (Elf64_Sym *)(ElfData + sh_offset);
        ElfCtx->SymbolCount = (UINT32)(sh_size / sizeof(Elf64_Sym));
      } else {
        ElfCtx->Symbols.Elf32 = (Elf32_Sym *)(ElfData + sh_offset);
        ElfCtx->SymbolCount = (UINT32)(sh_size / sizeof(Elf32_Sym));
      }

      // Get symbol string table
      if (sh_link < SectionCount) {
        UINT64 strtab_offset;
        if (ElfCtx->Is64Bit) {
          strtab_offset = ElfCtx->Sections.Elf64[sh_link].sh_offset;
        } else {
          strtab_offset = ElfCtx->Sections.Elf32[sh_link].sh_offset;
        }
        ElfCtx->SymbolStringTable = (CHAR8 *)(ElfData + strtab_offset);
      }

      return BINFORMAT_SUCCESS;
    }
  }

  return BINFORMAT_ERROR_NOT_FOUND;
}

/**
  Get symbol information by index.

  @param[in]   Context      ELF context.
  @param[in]   Index        Symbol index.
  @param[out]  Symbol       Pointer to receive symbol information.

  @retval BINFORMAT_SUCCESS       Symbol information retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetSymbol (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  UINT32             Index,
  OUT BINFORMAT_SYMBOL   *Symbol
  )
{
  ELF_CONTEXT       *ElfCtx;
  BINFORMAT_STATUS  Status;

  if (Context == NULL || Symbol == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  // Find symbol table if not already done
  if (ElfCtx->Symbols.Elf32 == NULL && ElfCtx->Symbols.Elf64 == NULL) {
    Status = ElfFindSymbolTable(ElfCtx);
    if (BINFORMAT_IS_ERROR(Status)) {
      return Status;
    }
  }

  if (Index >= ElfCtx->SymbolCount) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  memset(Symbol, 0, sizeof(BINFORMAT_SYMBOL));

  if (ElfCtx->Is64Bit) {
    Elf64_Sym *Sym = &ElfCtx->Symbols.Elf64[Index];

    UINT32 st_name = SWAP32(ElfCtx, Sym->st_name);
    if (ElfCtx->SymbolStringTable != NULL && st_name < 0x10000) {
      strncpy(Symbol->Name, ElfCtx->SymbolStringTable + st_name,
              BINFORMAT_MAX_SYMBOL_NAME - 1);
    }

    Symbol->Value = SWAP64(ElfCtx, Sym->st_value);
    Symbol->Size = SWAP64(ElfCtx, Sym->st_size);
    Symbol->Bind = ELF64_ST_BIND(Sym->st_info);  // st_info is single byte, no swap needed
    Symbol->Type = ELF64_ST_TYPE(Sym->st_info);
    Symbol->SectionIndex = SWAP16(ElfCtx, Sym->st_shndx);
    Symbol->Other = Sym->st_other;  // Single byte, no swap needed
  } else {
    Elf32_Sym *Sym = &ElfCtx->Symbols.Elf32[Index];

    UINT32 st_name = SWAP32(ElfCtx, Sym->st_name);
    if (ElfCtx->SymbolStringTable != NULL && st_name < 0x10000) {
      strncpy(Symbol->Name, ElfCtx->SymbolStringTable + st_name,
              BINFORMAT_MAX_SYMBOL_NAME - 1);
    }

    Symbol->Value = SWAP32(ElfCtx, Sym->st_value);
    Symbol->Size = SWAP32(ElfCtx, Sym->st_size);
    Symbol->Bind = ELF32_ST_BIND(Sym->st_info);  // st_info is single byte, no swap needed
    Symbol->Type = ELF32_ST_TYPE(Sym->st_info);
    Symbol->SectionIndex = SWAP16(ElfCtx, Sym->st_shndx);
    Symbol->Other = Sym->st_other;  // Single byte, no swap needed
  }

  return BINFORMAT_SUCCESS;
}

/**
  Get symbol information by name.

  @param[in]   Context      ELF context.
  @param[in]   Name         Symbol name.
  @param[out]  Symbol       Pointer to receive symbol information.

  @retval BINFORMAT_SUCCESS       Symbol found and retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetSymbolByName (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  CONST CHAR8        *Name,
  OUT BINFORMAT_SYMBOL   *Symbol
  )
{
  ELF_CONTEXT       *ElfCtx;
  BINFORMAT_STATUS  Status;
  UINT32            i;

  if (Context == NULL || Name == NULL || Symbol == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  // Find symbol table if not already done
  if (ElfCtx->Symbols.Elf32 == NULL && ElfCtx->Symbols.Elf64 == NULL) {
    Status = ElfFindSymbolTable(ElfCtx);
    if (BINFORMAT_IS_ERROR(Status)) {
      return Status;
    }
  }

  for (i = 0; i < ElfCtx->SymbolCount; i++) {
    BINFORMAT_SYMBOL TempSymbol;

    Status = ElfGetSymbol(Context, i, &TempSymbol);
    if (!BINFORMAT_IS_ERROR(Status)) {
      if (strcmp(TempSymbol.Name, Name) == 0) {
        memcpy(Symbol, &TempSymbol, sizeof(BINFORMAT_SYMBOL));
        return BINFORMAT_SUCCESS;
      }
    }
  }

  return BINFORMAT_ERROR_NOT_FOUND;
}

/**
  Get relocations for a section.

  @param[in]   Context       ELF context.
  @param[in]   SectionIndex  Section index.
  @param[out]  Relocations   Pointer to receive relocations array.
  @param[out]  Count         Pointer to receive relocation count.

  @retval BINFORMAT_SUCCESS       Relocations retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetRelocations (
  IN  BINFORMAT_CONTEXT     *Context,
  IN  UINT32                SectionIndex,
  OUT BINFORMAT_RELOCATION  **Relocations,
  OUT UINT32                *Count
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT32       SectionCount;
  UINT32       i;

  if (Context == NULL || Relocations == NULL || Count == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  // Find relocation section for the given section
  for (i = 0; i < SectionCount; i++) {
    UINT32  sh_type;
    UINT32  sh_info;
    UINT64  sh_offset;
    UINT64  sh_size;
    UINT64  sh_entsize;

    if (ElfCtx->Is64Bit) {
      Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[i];
      sh_type = Shdr->sh_type;
      sh_info = Shdr->sh_info;
      sh_offset = Shdr->sh_offset;
      sh_size = Shdr->sh_size;
      sh_entsize = Shdr->sh_entsize;
    } else {
      Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[i];
      sh_type = Shdr->sh_type;
      sh_info = Shdr->sh_info;
      sh_offset = Shdr->sh_offset;
      sh_size = Shdr->sh_size;
      sh_entsize = Shdr->sh_entsize;
    }

    if ((sh_type == SHT_REL || sh_type == SHT_RELA) && sh_info == SectionIndex) {
      UINT32 RelCount;
      BINFORMAT_RELOCATION *Rels;
      UINT8 *ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;
      UINT32 j;

      RelCount = (UINT32)(sh_size / sh_entsize);
      Rels = (BINFORMAT_RELOCATION *)calloc(RelCount, sizeof(BINFORMAT_RELOCATION));
      if (Rels == NULL) {
        return BINFORMAT_ERROR_OUT_OF_MEMORY;
      }

      for (j = 0; j < RelCount; j++) {
        if (sh_type == SHT_RELA) {
          if (ElfCtx->Is64Bit) {
            Elf64_Rela *Rela = (Elf64_Rela *)(ElfData + sh_offset) + j;
            Rels[j].Offset = Rela->r_offset;
            Rels[j].SymbolIndex = ELF64_R_SYM(Rela->r_info);
            Rels[j].Type = ELF64_R_TYPE(Rela->r_info);
            Rels[j].Addend = Rela->r_addend;
          } else {
            Elf32_Rela *Rela = (Elf32_Rela *)(ElfData + sh_offset) + j;
            Rels[j].Offset = Rela->r_offset;
            Rels[j].SymbolIndex = ELF32_R_SYM(Rela->r_info);
            Rels[j].Type = ELF32_R_TYPE(Rela->r_info);
            Rels[j].Addend = Rela->r_addend;
          }
        } else {
          if (ElfCtx->Is64Bit) {
            Elf64_Rel *Rel = (Elf64_Rel *)(ElfData + sh_offset) + j;
            Rels[j].Offset = Rel->r_offset;
            Rels[j].SymbolIndex = ELF64_R_SYM(Rel->r_info);
            Rels[j].Type = ELF64_R_TYPE(Rel->r_info);
            Rels[j].Addend = 0;
          } else {
            Elf32_Rel *Rel = (Elf32_Rel *)(ElfData + sh_offset) + j;
            Rels[j].Offset = Rel->r_offset;
            Rels[j].SymbolIndex = ELF32_R_SYM(Rel->r_info);
            Rels[j].Type = ELF32_R_TYPE(Rel->r_info);
            Rels[j].Addend = 0;
          }
        }
      }

      *Relocations = Rels;
      *Count = RelCount;
      return BINFORMAT_SUCCESS;
    }
  }

  *Relocations = NULL;
  *Count = 0;
  return BINFORMAT_ERROR_NOT_FOUND;
}

/**
  Compute ELF hash value for a symbol name.

  @param[in]  Name  Symbol name.

  @return ELF hash value.

**/
STATIC
UINT32
ElfHash (
  IN  CONST CHAR8  *Name
  )
{
  UINT32  Hash = 0;
  UINT32  Temp;

  while (*Name != '\0') {
    Hash = (Hash << 4) + (UINT8)*Name++;
    Temp = Hash & 0xF0000000;
    if (Temp != 0) {
      Hash ^= Temp >> 24;
    }
    Hash &= ~Temp;
  }

  return Hash;
}

/**
  Compute GNU hash value for a symbol name.

  @param[in]  Name  Symbol name.

  @return GNU hash value.

**/
STATIC
UINT32
GnuHash (
  IN  CONST CHAR8  *Name
  )
{
  UINT32  Hash = 5381;

  while (*Name != '\0') {
    Hash = Hash * 33 + (UINT8)*Name++;
  }

  return Hash;
}

/**
  Compute checksum of ELF file.

  @param[in]  Context  ELF context.

  @return Checksum value, or -1 on error.

**/
STATIC
INT64
ElfChecksum (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT64       Sum = 0;
  UINT64       i;
  UINT8        *Data;
  UINT64       Size;

  if (Context == NULL) {
    return -1;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);
  Data = ElfCtx->FileData + ElfCtx->CurrentOffset;

  // Get size from header
  if (ElfCtx->Is64Bit) {
    Elf64_Ehdr *Hdr = ElfCtx->Header.Elf64;
    if (Hdr->e_shoff > 0 && Hdr->e_shnum > 0) {
      Size = Hdr->e_shoff + (Hdr->e_shnum * Hdr->e_shentsize);
    } else {
      Size = Hdr->e_ehsize;
    }
  } else {
    Elf32_Ehdr *Hdr = ElfCtx->Header.Elf32;
    if (Hdr->e_shoff > 0 && Hdr->e_shnum > 0) {
      Size = Hdr->e_shoff + (Hdr->e_shnum * Hdr->e_shentsize);
    } else {
      Size = Hdr->e_ehsize;
    }
  }

  // Compute simple checksum (sum of all bytes)
  for (i = 0; i < Size && i < (ElfCtx->FileSize - ElfCtx->CurrentOffset); i++) {
    Sum += Data[i];
  }

  return (INT64)(Sum & 0x7FFFFFFFFFFFFFFF);
}

/**
  Get string from string table.

  @param[in]  Context  ELF context.
  @param[in]  Section  Section index of string table.
  @param[in]  Offset   Offset into string table.

  @return Pointer to string, or NULL on error.

**/
STATIC
CONST CHAR8 *
ElfGetString (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  UINT32             Section,
  IN  UINT64             Offset
  )
{
  ELF_CONTEXT   *ElfCtx;
  UINT32        SectionCount;
  UINT8         *ElfData;
  UINT64        StrTabOffset;
  UINT64        StrTabSize;

  if (Context == NULL) {
    return NULL;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);
  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  if (Section >= SectionCount) {
    return NULL;
  }

  ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;

  if (ElfCtx->Is64Bit) {
    Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[Section];
    if (Shdr->sh_type != SHT_STRTAB) {
      return NULL;
    }
    StrTabOffset = Shdr->sh_offset;
    StrTabSize = Shdr->sh_size;
  } else {
    Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[Section];
    if (Shdr->sh_type != SHT_STRTAB) {
      return NULL;
    }
    StrTabOffset = Shdr->sh_offset;
    StrTabSize = Shdr->sh_size;
  }

  if (Offset >= StrTabSize) {
    return NULL;
  }

  return (CONST CHAR8 *)(ElfData + StrTabOffset + Offset);
}

/**
  Get dynamic section entry by index.

  @param[in]   Context  ELF context.
  @param[in]   Index    Dynamic entry index.
  @param[out]  Tag      Pointer to receive tag value.
  @param[out]  Value    Pointer to receive entry value.

  @retval BINFORMAT_SUCCESS       Dynamic entry retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetDynamic (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  UINT32             Index,
  OUT INT64              *Tag,
  OUT UINT64             *Value
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT32       SectionCount;
  UINT32       i;

  if (Context == NULL || Tag == NULL || Value == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);
  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  // Find dynamic section
  for (i = 0; i < SectionCount; i++) {
    UINT32  sh_type;
    UINT64  sh_offset;
    UINT64  sh_size;
    UINT64  sh_entsize;

    if (ElfCtx->Is64Bit) {
      Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[i];
      sh_type = Shdr->sh_type;
      sh_offset = Shdr->sh_offset;
      sh_size = Shdr->sh_size;
      sh_entsize = sizeof(Elf64_Dyn);
    } else {
      Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[i];
      sh_type = Shdr->sh_type;
      sh_offset = Shdr->sh_offset;
      sh_size = Shdr->sh_size;
      sh_entsize = sizeof(Elf32_Dyn);
    }

    if (sh_type == SHT_DYNAMIC) {
      UINT32  DynCount = (UINT32)(sh_size / sh_entsize);
      UINT8   *ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;

      if (Index >= DynCount) {
        return BINFORMAT_ERROR_NOT_FOUND;
      }

      if (ElfCtx->Is64Bit) {
        Elf64_Dyn *Dyn = (Elf64_Dyn *)(ElfData + sh_offset) + Index;
        *Tag = Dyn->d_tag;
        *Value = Dyn->d_un.d_val;
      } else {
        Elf32_Dyn *Dyn = (Elf32_Dyn *)(ElfData + sh_offset) + Index;
        *Tag = Dyn->d_tag;
        *Value = Dyn->d_un.d_val;
      }

      return BINFORMAT_SUCCESS;
    }
  }

  return BINFORMAT_ERROR_NOT_FOUND;
}

/**
  Get note section entry.

  @param[in]   Context   ELF context.
  @param[in]   Section   Section index.
  @param[in]   Offset    Offset into note section.
  @param[out]  NameSize  Pointer to receive name size.
  @param[out]  DescSize  Pointer to receive descriptor size.
  @param[out]  Type      Pointer to receive note type.
  @param[out]  NameOff   Pointer to receive name offset.
  @param[out]  DescOff   Pointer to receive descriptor offset.

  @retval BINFORMAT_SUCCESS       Note retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfGetNote (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  UINT32             Section,
  IN  UINT64             Offset,
  OUT UINT32             *NameSize,
  OUT UINT32             *DescSize,
  OUT UINT32             *Type,
  OUT UINT64             *NameOff,
  OUT UINT64             *DescOff
  )
{
  ELF_CONTEXT   *ElfCtx;
  UINT32        SectionCount;
  UINT8         *ElfData;
  UINT64        SecOffset;
  UINT64        SecSize;
  Elf32_Nhdr    *Note;

  if (Context == NULL || NameSize == NULL || DescSize == NULL ||
      Type == NULL || NameOff == NULL || DescOff == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);
  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  if (Section >= SectionCount) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;

  if (ElfCtx->Is64Bit) {
    Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[Section];
    if (Shdr->sh_type != SHT_NOTE) {
      return BINFORMAT_ERROR_INVALID_PARAMETER;
    }
    SecOffset = Shdr->sh_offset;
    SecSize = Shdr->sh_size;
  } else {
    Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[Section];
    if (Shdr->sh_type != SHT_NOTE) {
      return BINFORMAT_ERROR_INVALID_PARAMETER;
    }
    SecOffset = Shdr->sh_offset;
    SecSize = Shdr->sh_size;
  }

  if (Offset + sizeof(Elf32_Nhdr) > SecSize) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  Note = (Elf32_Nhdr *)(ElfData + SecOffset + Offset);
  *NameSize = Note->n_namesz;
  *DescSize = Note->n_descsz;
  *Type = Note->n_type;
  *NameOff = SecOffset + Offset + sizeof(Elf32_Nhdr);

  // Descriptor follows name, aligned to 4 bytes
  *DescOff = *NameOff + ((Note->n_namesz + 3) & ~3);

  return BINFORMAT_SUCCESS;
}

/**
  Write ELF file to disk.

  @param[in]  Context   ELF context.
  @param[in]  FilePath  Path to output file.

  @retval BINFORMAT_SUCCESS       File written successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfWriteFile (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  CONST CHAR8        *FilePath
  )
{
  ELF_CONTEXT  *ElfCtx;
  FILE         *File;
  UINT64       Written;

  if (Context == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (ElfCtx->FileData == NULL || ElfCtx->FileSize == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Open output file
  //
  File = fopen(FilePath, "wb");
  if (File == NULL) {
    return BINFORMAT_ERROR_IO;
  }

  //
  // Write entire file data
  //
  Written = fwrite(ElfCtx->FileData, 1, ElfCtx->FileSize, File);
  fclose(File);

  if (Written != ElfCtx->FileSize) {
    return BINFORMAT_ERROR_IO;
  }

  return BINFORMAT_SUCCESS;
}

/**
  Write ELF to memory buffer.

  @param[in]   Context  ELF context.
  @param[out]  Buffer   Output buffer.
  @param[in]   Size     Buffer size.
  @param[out]  Written  Bytes written.

  @retval BINFORMAT_SUCCESS       Data written successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfWriteMemory (
  IN  BINFORMAT_CONTEXT  *Context,
  OUT VOID               *Buffer,
  IN  UINT64             Size,
  OUT UINT64             *Written
  )
{
  ELF_CONTEXT  *ElfCtx;

  if (Context == NULL || Buffer == NULL || Written == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (ElfCtx->FileData == NULL || ElfCtx->FileSize == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (Size < ElfCtx->FileSize) {
    return BINFORMAT_ERROR_BUFFER_TOO_SMALL;
  }

  //
  // Copy ELF data to output buffer
  //
  memcpy(Buffer, ElfCtx->FileData, ElfCtx->FileSize);
  *Written = ElfCtx->FileSize;

  return BINFORMAT_SUCCESS;
}

/**
  Add new section to ELF file.

  @param[in]   Context  ELF context.
  @param[in]   Section  Section to add.
  @param[out]  Index    Section index (optional).

  @retval BINFORMAT_SUCCESS       Section added successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfAddSection (
  IN  BINFORMAT_CONTEXT   *Context,
  IN  BINFORMAT_SECTION   *Section,
  OUT UINT32              *Index
  )
{
  ELF_CONTEXT  *ElfCtx;
  UINT32       SectionCount;
  UINT64       OldSize;
  UINT64       NewSize;
  UINT64       ShdrSize;
  UINT64       NewShoff;
  UINT8        *NewData;

  if (Context == NULL || Section == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (ElfCtx->ReadOnly) {
    return BINFORMAT_ERROR_UNSUPPORTED;
  }

  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  //
  // Calculate new file size
  //
  ShdrSize = ElfCtx->Is64Bit ? sizeof(Elf64_Shdr) : sizeof(Elf32_Shdr);
  OldSize = ElfCtx->FileSize;
  NewSize = OldSize + ShdrSize + Section->Size;

  //
  // Reallocate buffer
  //
  NewData = (UINT8 *)realloc(ElfCtx->FileData, NewSize);
  if (NewData == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  ElfCtx->FileData = NewData;
  ElfCtx->FileSize = NewSize;

  //
  // Add section data at end
  //
  NewShoff = OldSize;
  if (Section->Data != NULL && Section->Size > 0) {
    memcpy(ElfCtx->FileData + NewShoff, Section->Data, Section->Size);
  }

  //
  // Add section header
  //
  if (ElfCtx->Is64Bit) {
    Elf64_Shdr NewShdr = {0};
    NewShdr.sh_type = Section->Type;
    NewShdr.sh_flags = Section->Flags;
    NewShdr.sh_addr = Section->VirtualAddress;
    NewShdr.sh_offset = NewShoff;
    NewShdr.sh_size = Section->Size;
    NewShdr.sh_link = Section->Link;
    NewShdr.sh_info = Section->Info;
    NewShdr.sh_addralign = Section->Alignment;
    NewShdr.sh_entsize = Section->EntrySize;

    // Reallocate section header table
    ElfCtx->Sections.Elf64 = (Elf64_Shdr *)realloc(
      ElfCtx->Sections.Elf64,
      (SectionCount + 1) * sizeof(Elf64_Shdr)
    );
    if (ElfCtx->Sections.Elf64 == NULL) {
      return BINFORMAT_ERROR_OUT_OF_MEMORY;
    }

    memcpy(&ElfCtx->Sections.Elf64[SectionCount], &NewShdr, sizeof(Elf64_Shdr));
    ElfCtx->Header.Elf64->e_shnum = SectionCount + 1;
  } else {
    Elf32_Shdr NewShdr = {0};
    NewShdr.sh_type = Section->Type;
    NewShdr.sh_flags = (UINT32)Section->Flags;
    NewShdr.sh_addr = (UINT32)Section->VirtualAddress;
    NewShdr.sh_offset = (UINT32)NewShoff;
    NewShdr.sh_size = (UINT32)Section->Size;
    NewShdr.sh_link = Section->Link;
    NewShdr.sh_info = Section->Info;
    NewShdr.sh_addralign = (UINT32)Section->Alignment;
    NewShdr.sh_entsize = (UINT32)Section->EntrySize;

    ElfCtx->Sections.Elf32 = (Elf32_Shdr *)realloc(
      ElfCtx->Sections.Elf32,
      (SectionCount + 1) * sizeof(Elf32_Shdr)
    );
    if (ElfCtx->Sections.Elf32 == NULL) {
      return BINFORMAT_ERROR_OUT_OF_MEMORY;
    }

    memcpy(&ElfCtx->Sections.Elf32[SectionCount], &NewShdr, sizeof(Elf32_Shdr));
    ElfCtx->Header.Elf32->e_shnum = SectionCount + 1;
  }

  if (Index != NULL) {
    *Index = SectionCount;
  }

  return BINFORMAT_SUCCESS;
}

/**
  Add symbol to symbol table.

  @param[in]   Context  ELF context.
  @param[in]   Symbol   Symbol to add.
  @param[out]  Index    Symbol index (optional).

  @retval BINFORMAT_SUCCESS       Symbol added successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfAddSymbol (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  BINFORMAT_SYMBOL   *Symbol,
  OUT UINT32             *Index
  )
{
  ELF_CONTEXT       *ElfCtx;
  BINFORMAT_STATUS  Status;

  if (Context == NULL || Symbol == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (ElfCtx->ReadOnly) {
    return BINFORMAT_ERROR_UNSUPPORTED;
  }

  //
  // Find symbol table if not already done
  //
  if (ElfCtx->Symbols.Elf32 == NULL && ElfCtx->Symbols.Elf64 == NULL) {
    Status = ElfFindSymbolTable(ElfCtx);
    if (BINFORMAT_IS_ERROR(Status)) {
      return BINFORMAT_ERROR_NOT_FOUND;
    }
  }

  //
  // TODO: Implement symbol table expansion
  // For now, return not implemented
  //
  (VOID)Index;
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Add relocation entry.

  @param[in]  Context       ELF context.
  @param[in]  SectionIndex  Target section index.
  @param[in]  Relocation    Relocation to add.

  @retval BINFORMAT_SUCCESS       Relocation added successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfAddRelocation (
  IN  BINFORMAT_CONTEXT     *Context,
  IN  UINT32                SectionIndex,
  IN  BINFORMAT_RELOCATION  *Relocation
  )
{
  ELF_CONTEXT  *ElfCtx;

  if (Context == NULL || Relocation == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (ElfCtx->ReadOnly) {
    return BINFORMAT_ERROR_UNSUPPORTED;
  }

  //
  // TODO: Implement relocation table expansion
  // For now, return not implemented
  //
  (VOID)SectionIndex;
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Select architecture in FatELF binary.

  @param[in]   Context      ELF context.
  @param[in]   ArchIndex    Architecture index.

  @retval BINFORMAT_SUCCESS       Architecture selected.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
ElfSelectArchitecture (
  IN  BINFORMAT_CONTEXT   *Context,
  IN  UINT32              ArchIndex
  )
{
  ELF_CONTEXT  *ElfCtx;

  if (Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx = ELF_CONTEXT_FROM_BINFORMAT(Context);

  if (!ElfCtx->IsFat) {
    return BINFORMAT_ERROR_UNSUPPORTED;
  }

  if (ArchIndex >= ElfCtx->FatHeader->num_records) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  ElfCtx->SelectedArch = ArchIndex;
  ElfCtx->CurrentOffset = ElfCtx->FatRecords[ArchIndex].offset;

  //
  // Re-parse headers for selected architecture
  //
  UINT8 *ElfData = ElfCtx->FileData + ElfCtx->CurrentOffset;
  ElfCtx->Is64Bit = (ElfCtx->FatRecords[ArchIndex].word_size == 2);

  if (ElfCtx->Is64Bit) {
    ElfCtx->Header.Elf64 = (Elf64_Ehdr *)ElfData;
    ElfCtx->Sections.Elf64 = (Elf64_Shdr *)(ElfData + ElfCtx->Header.Elf64->e_shoff);
    ElfCtx->Programs.Elf64 = (Elf64_Phdr *)(ElfData + ElfCtx->Header.Elf64->e_phoff);
  } else {
    ElfCtx->Header.Elf32 = (Elf32_Ehdr *)ElfData;
    ElfCtx->Sections.Elf32 = (Elf32_Shdr *)(ElfData + ElfCtx->Header.Elf32->e_shoff);
    ElfCtx->Programs.Elf32 = (Elf32_Phdr *)(ElfData + ElfCtx->Header.Elf32->e_phoff);
  }

  return BINFORMAT_SUCCESS;
}

//
// ELF API Table
//
STATIC CONST BINFORMAT_API gElfApi = {
  .LibraryName = "libelf",
  .Version = 1,
  .InitFile = ElfInitFile,
  .InitMemory = ElfInitMemory,
  .Create = ElfCreate,
  .Close = ElfClose,
  .GetHeader = ElfGetHeader,
  .GetSection = ElfGetSection,
  .GetSectionByName = ElfGetSectionByName,
  .GetSegment = ElfGetSegment,
  .GetSymbol = ElfGetSymbol,
  .GetSymbolByName = ElfGetSymbolByName,
  .GetRelocations = ElfGetRelocations,
  .AddSection = ElfAddSection,
  .AddSymbol = ElfAddSymbol,
  .AddRelocation = ElfAddRelocation,
  .WriteFile = ElfWriteFile,
  .WriteMemory = ElfWriteMemory,
  .SelectArchitecture = ElfSelectArchitecture
};

/**
  Get the ELF library API table.

  @return Pointer to ELF library API table.

**/
CONST BINFORMAT_API *
ElfGetApi (
  VOID
  )
{
  return &gElfApi;
}
