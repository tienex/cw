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
#include "../../include/binformat/LibElf.h"

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

  if (ElfCtx->Is64Bit) {
    ElfCtx->Header.Elf64 = (Elf64_Ehdr *)ElfData;
    ElfCtx->Sections.Elf64 = (Elf64_Shdr *)(ElfData + ElfCtx->Header.Elf64->e_shoff);
    ElfCtx->Programs.Elf64 = (Elf64_Phdr *)(ElfData + ElfCtx->Header.Elf64->e_phoff);

    //
    // Get string table
    //
    if (ElfCtx->Header.Elf64->e_shstrndx < ElfCtx->Header.Elf64->e_shnum) {
      ElfCtx->StringTable = (CHAR8 *)(ElfData +
        ElfCtx->Sections.Elf64[ElfCtx->Header.Elf64->e_shstrndx].sh_offset);
    }
  } else {
    ElfCtx->Header.Elf32 = (Elf32_Ehdr *)ElfData;
    ElfCtx->Sections.Elf32 = (Elf32_Shdr *)(ElfData + ElfCtx->Header.Elf32->e_shoff);
    ElfCtx->Programs.Elf32 = (Elf32_Phdr *)(ElfData + ElfCtx->Header.Elf32->e_phoff);

    //
    // Get string table
    //
    if (ElfCtx->Header.Elf32->e_shstrndx < ElfCtx->Header.Elf32->e_shnum) {
      ElfCtx->StringTable = (CHAR8 *)(ElfData +
        ElfCtx->Sections.Elf32[ElfCtx->Header.Elf32->e_shstrndx].sh_offset);
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

  if (ElfCtx->Is64Bit) {
    ElfCtx->Header.Elf64 = (Elf64_Ehdr *)ElfData;
    ElfCtx->Sections.Elf64 = (Elf64_Shdr *)(ElfData + ElfCtx->Header.Elf64->e_shoff);
    ElfCtx->Programs.Elf64 = (Elf64_Phdr *)(ElfData + ElfCtx->Header.Elf64->e_phoff);

    if (ElfCtx->Header.Elf64->e_shstrndx < ElfCtx->Header.Elf64->e_shnum) {
      ElfCtx->StringTable = (CHAR8 *)(ElfData +
        ElfCtx->Sections.Elf64[ElfCtx->Header.Elf64->e_shstrndx].sh_offset);
    }
  } else {
    ElfCtx->Header.Elf32 = (Elf32_Ehdr *)ElfData;
    ElfCtx->Sections.Elf32 = (Elf32_Shdr *)(ElfData + ElfCtx->Header.Elf32->e_shoff);
    ElfCtx->Programs.Elf32 = (Elf32_Phdr *)(ElfData + ElfCtx->Header.Elf32->e_phoff);

    if (ElfCtx->Header.Elf32->e_shstrndx < ElfCtx->Header.Elf32->e_shnum) {
      ElfCtx->StringTable = (CHAR8 *)(ElfData +
        ElfCtx->Sections.Elf32[ElfCtx->Header.Elf32->e_shstrndx].sh_offset);
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
    HeaderInfo->FileType = ElfTypeToGeneric(Hdr->e_type);
    HeaderInfo->Machine = ElfMachineToGeneric(Hdr->e_machine);
    HeaderInfo->Endianness = (Hdr->e_ident[EI_DATA] == ELFDATA2LSB) ?
                             BinEndianLittle : BinEndianBig;
    HeaderInfo->Version = Hdr->e_version;
    HeaderInfo->EntryPoint = Hdr->e_entry;
    HeaderInfo->Flags = Hdr->e_flags;
    HeaderInfo->SectionCount = Hdr->e_shnum;
    HeaderInfo->SegmentCount = Hdr->e_phnum;
  } else {
    Elf32_Ehdr *Hdr = ElfCtx->Header.Elf32;
    HeaderInfo->FileType = ElfTypeToGeneric(Hdr->e_type);
    HeaderInfo->Machine = ElfMachineToGeneric(Hdr->e_machine);
    HeaderInfo->Endianness = (Hdr->e_ident[EI_DATA] == ELFDATA2LSB) ?
                             BinEndianLittle : BinEndianBig;
    HeaderInfo->Version = Hdr->e_version;
    HeaderInfo->EntryPoint = Hdr->e_entry;
    HeaderInfo->Flags = Hdr->e_flags;
    HeaderInfo->SectionCount = Hdr->e_shnum;
    HeaderInfo->SegmentCount = Hdr->e_phnum;
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

  SectionCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_shnum :
                                   ElfCtx->Header.Elf32->e_shnum;

  if (Index >= SectionCount) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  memset(Section, 0, sizeof(BINFORMAT_SECTION));

  if (ElfCtx->Is64Bit) {
    Elf64_Shdr *Shdr = &ElfCtx->Sections.Elf64[Index];

    if (ElfCtx->StringTable != NULL && Shdr->sh_name < 0x10000) {
      strncpy(Section->Name, ElfCtx->StringTable + Shdr->sh_name,
              BINFORMAT_MAX_SECTION_NAME - 1);
    }

    Section->Type = Shdr->sh_type;
    Section->Flags = Shdr->sh_flags;
    Section->VirtualAddress = Shdr->sh_addr;
    Section->FileOffset = Shdr->sh_offset;
    Section->Size = Shdr->sh_size;
    Section->Link = Shdr->sh_link;
    Section->Info = Shdr->sh_info;
    Section->Alignment = Shdr->sh_addralign;
    Section->EntrySize = Shdr->sh_entsize;
    Section->Data = ElfCtx->FileData + ElfCtx->CurrentOffset + Shdr->sh_offset;
  } else {
    Elf32_Shdr *Shdr = &ElfCtx->Sections.Elf32[Index];

    if (ElfCtx->StringTable != NULL && Shdr->sh_name < 0x10000) {
      strncpy(Section->Name, ElfCtx->StringTable + Shdr->sh_name,
              BINFORMAT_MAX_SECTION_NAME - 1);
    }

    Section->Type = Shdr->sh_type;
    Section->Flags = Shdr->sh_flags;
    Section->VirtualAddress = Shdr->sh_addr;
    Section->FileOffset = Shdr->sh_offset;
    Section->Size = Shdr->sh_size;
    Section->Link = Shdr->sh_link;
    Section->Info = Shdr->sh_info;
    Section->Alignment = Shdr->sh_addralign;
    Section->EntrySize = Shdr->sh_entsize;
    Section->Data = ElfCtx->FileData + ElfCtx->CurrentOffset + Shdr->sh_offset;
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

  SegmentCount = ElfCtx->Is64Bit ? ElfCtx->Header.Elf64->e_phnum :
                                   ElfCtx->Header.Elf32->e_phnum;

  if (Index >= SegmentCount) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  memset(Segment, 0, sizeof(BINFORMAT_SEGMENT));

  if (ElfCtx->Is64Bit) {
    Elf64_Phdr *Phdr = &ElfCtx->Programs.Elf64[Index];
    Segment->Type = Phdr->p_type;
    Segment->Flags = Phdr->p_flags;
    Segment->FileOffset = Phdr->p_offset;
    Segment->VirtualAddress = Phdr->p_vaddr;
    Segment->PhysicalAddress = Phdr->p_paddr;
    Segment->FileSize = Phdr->p_filesz;
    Segment->MemorySize = Phdr->p_memsz;
    Segment->Alignment = Phdr->p_align;
  } else {
    Elf32_Phdr *Phdr = &ElfCtx->Programs.Elf32[Index];
    Segment->Type = Phdr->p_type;
    Segment->Flags = Phdr->p_flags;
    Segment->FileOffset = Phdr->p_offset;
    Segment->VirtualAddress = Phdr->p_vaddr;
    Segment->PhysicalAddress = Phdr->p_paddr;
    Segment->FileSize = Phdr->p_filesz;
    Segment->MemorySize = Phdr->p_memsz;
    Segment->Alignment = Phdr->p_align;
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

    if (ElfCtx->SymbolStringTable != NULL && Sym->st_name < 0x10000) {
      strncpy(Symbol->Name, ElfCtx->SymbolStringTable + Sym->st_name,
              BINFORMAT_MAX_SYMBOL_NAME - 1);
    }

    Symbol->Value = Sym->st_value;
    Symbol->Size = Sym->st_size;
    Symbol->Bind = ELF64_ST_BIND(Sym->st_info);
    Symbol->Type = ELF64_ST_TYPE(Sym->st_info);
    Symbol->SectionIndex = Sym->st_shndx;
  } else {
    Elf32_Sym *Sym = &ElfCtx->Symbols.Elf32[Index];

    if (ElfCtx->SymbolStringTable != NULL && Sym->st_name < 0x10000) {
      strncpy(Symbol->Name, ElfCtx->SymbolStringTable + Sym->st_name,
              BINFORMAT_MAX_SYMBOL_NAME - 1);
    }

    Symbol->Value = Sym->st_value;
    Symbol->Size = Sym->st_size;
    Symbol->Bind = ELF32_ST_BIND(Sym->st_info);
    Symbol->Type = ELF32_ST_TYPE(Sym->st_info);
    Symbol->SectionIndex = Sym->st_shndx;
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

//
// Stub implementations for unsupported operations
//
STATIC BINFORMAT_STATUS ElfAddSection(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SECTION *Sec, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS ElfAddSymbol(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SYMBOL *Sym, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS ElfAddRelocation(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, IN BINFORMAT_RELOCATION *Rel) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS ElfWriteFile(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Path) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS ElfWriteMemory(IN BINFORMAT_CONTEXT *Ctx, OUT VOID *Buf, IN UINT64 Size, OUT UINT64 *Written) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }

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
