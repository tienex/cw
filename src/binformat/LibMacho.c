/** @file
  Mach-O Binary Format Library Implementation.

  This file implements support for Mach-O 32/64-bit and universal binaries
  following the unified binary format API.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LibMacho.h"

#pragma pack(push, 1)

typedef struct {
  UINT32  Magic;        ///< Magic number
  UINT32  CpuType;      ///< CPU type
  UINT32  CpuSubtype;   ///< CPU subtype
  UINT32  FileType;     ///< File type
  UINT32  NumCmds;      ///< Number of load commands
  UINT32  SizeOfCmds;   ///< Size of load commands
  UINT32  Flags;        ///< Flags
} MACHO_HEADER_32;

typedef struct {
  UINT32  Magic;
  UINT32  CpuType;
  UINT32  CpuSubtype;
  UINT32  FileType;
  UINT32  NumCmds;
  UINT32  SizeOfCmds;
  UINT32  Flags;
  UINT32  Reserved;     ///< Reserved (64-bit)
} MACHO_HEADER_64;

typedef struct {
  UINT32  Magic;        ///< Fat magic
  UINT32  NumArchs;     ///< Number of architectures
} FAT_HEADER;

typedef struct {
  UINT32  CpuType;      ///< CPU type
  UINT32  CpuSubtype;   ///< CPU subtype
  UINT32  Offset;       ///< File offset
  UINT32  Size;         ///< Size
  UINT32  Align;        ///< Alignment
} FAT_ARCH;

#pragma pack(pop)

//
// Mach-O Load Commands
//
#define LC_SEGMENT          0x1   ///< Segment of this file to be mapped
#define LC_SYMTAB           0x2   ///< Symbol table
#define LC_SYMSEG           0x3   ///< Link-edit gdb symbol table info (obsolete)
#define LC_THREAD           0x4   ///< Thread state
#define LC_UNIXTHREAD       0x5   ///< Unix thread (includes a stack)
#define LC_LOADFVMLIB       0x6   ///< Load a specified fixed VM shared library
#define LC_IDFVMLIB         0x7   ///< Fixed VM shared library identification
#define LC_IDENT            0x8   ///< Object identification info (obsolete)
#define LC_FVMFILE          0x9   ///< Fixed VM file inclusion (internal use)
#define LC_PREPAGE          0xa   ///< Prepage command (internal use)
#define LC_DYSYMTAB         0xb   ///< Dynamic link-edit symbol table info
#define LC_LOAD_DYLIB       0xc   ///< Load a dynamically linked shared library
#define LC_ID_DYLIB         0xd   ///< Dynamically linked shared library ID
#define LC_LOAD_DYLINKER    0xe   ///< Load a dynamic linker
#define LC_ID_DYLINKER      0xf   ///< Dynamic linker identification
#define LC_PREBOUND_DYLIB   0x10  ///< Modules prebound for a dynamically linked shared library
#define LC_ROUTINES         0x11  ///< Image routines
#define LC_SUB_FRAMEWORK    0x12  ///< Sub framework
#define LC_SUB_UMBRELLA     0x13  ///< Sub umbrella
#define LC_SUB_CLIENT       0x14  ///< Sub client
#define LC_SUB_LIBRARY      0x15  ///< Sub library
#define LC_TWOLEVEL_HINTS   0x16  ///< Two-level namespace lookup hints
#define LC_PREBIND_CKSUM    0x17  ///< Prebind checksum
#define LC_SEGMENT_64       0x19  ///< 64-bit segment of this file to be mapped
#define LC_ROUTINES_64      0x1a  ///< 64-bit image routines
#define LC_UUID             0x1b  ///< UUID of the binary
#define LC_RPATH            0x1c  ///< Runpath additions
#define LC_CODE_SIGNATURE   0x1d  ///< Local of code signature
#define LC_SEGMENT_SPLIT_INFO 0x1e ///< Local of info to split segments
#define LC_REEXPORT_DYLIB   0x1f  ///< Load and re-export dylib
#define LC_LAZY_LOAD_DYLIB  0x20  ///< Delay load of dylib until first use
#define LC_ENCRYPTION_INFO  0x21  ///< Encrypted segment information
#define LC_DYLD_INFO        0x22  ///< Compressed dyld information
#define LC_DYLD_INFO_ONLY   0x22  ///< Compressed dyld information only
#define LC_LOAD_UPWARD_DYLIB 0x23 ///< Load upward dylib
#define LC_VERSION_MIN_MACOSX 0x24 ///< Build for MacOSX min OS version
#define LC_VERSION_MIN_IPHONEOS 0x25 ///< Build for iPhoneOS min OS version
#define LC_FUNCTION_STARTS  0x26  ///< Compressed table of function start addresses
#define LC_DYLD_ENVIRONMENT 0x27  ///< String for dyld to treat like environment variable

//
// Mach-O Relocation Types - Generic
//
#define GENERIC_RELOC_VANILLA        0  ///< Generic relocation
#define GENERIC_RELOC_PAIR           1  ///< Relocation pair (for scattered)
#define GENERIC_RELOC_SECTDIFF       2  ///< Section difference
#define GENERIC_RELOC_PB_LA_PTR      3  ///< Prebound lazy pointer
#define GENERIC_RELOC_LOCAL_SECTDIFF 4  ///< Local section difference
#define GENERIC_RELOC_TLV            5  ///< Thread local variable

//
// Mach-O Relocation Types - x86-64
//
#define X86_64_RELOC_UNSIGNED   0  ///< Absolute address
#define X86_64_RELOC_SIGNED     1  ///< Signed 32-bit displacement
#define X86_64_RELOC_BRANCH     2  ///< Branch displacement
#define X86_64_RELOC_GOT_LOAD   3  ///< Load from GOT
#define X86_64_RELOC_GOT        4  ///< GOT reference
#define X86_64_RELOC_SUBTRACTOR 5  ///< Subtraction (must be followed by vanilla)
#define X86_64_RELOC_SIGNED_1   6  ///< Signed 32-bit displacement with -1 addend
#define X86_64_RELOC_SIGNED_2   7  ///< Signed 32-bit displacement with -2 addend
#define X86_64_RELOC_SIGNED_4   8  ///< Signed 32-bit displacement with -4 addend
#define X86_64_RELOC_TLV        9  ///< Thread local variable reference

//
// Mach-O Relocation Types - i386
//
#define I386_RELOC_VANILLA      0  ///< Generic relocation
#define I386_RELOC_PAIR         1  ///< Second relocation for sectdiff
#define I386_RELOC_SECTDIFF     2  ///< Section difference
#define I386_RELOC_PB_LA_PTR    3  ///< Prebound lazy pointer
#define I386_RELOC_LOCAL_SECTDIFF 4 ///< Local section difference
#define I386_RELOC_TLV          5  ///< Thread local variable

//
// Mach-O Relocation Types - ARM
//
#define ARM_RELOC_VANILLA       0  ///< Generic relocation
#define ARM_RELOC_PAIR          1  ///< Second relocation for sectdiff
#define ARM_RELOC_SECTDIFF      2  ///< Section difference
#define ARM_RELOC_LOCAL_SECTDIFF 3 ///< Local section difference
#define ARM_RELOC_PB_LA_PTR     4  ///< Prebound lazy pointer
#define ARM_RELOC_BR24          5  ///< 24-bit branch
#define ARM_THUMB_RELOC_BR22    6  ///< 22-bit Thumb branch
#define ARM_THUMB_32BIT_BRANCH  7  ///< 32-bit Thumb branch
#define ARM_RELOC_HALF          8  ///< Half (16-bit) relocation
#define ARM_RELOC_HALF_SECTDIFF 9  ///< Half sectdiff

//
// Mach-O Relocation Types - ARM64
//
#define ARM64_RELOC_UNSIGNED            0  ///< Absolute address
#define ARM64_RELOC_SUBTRACTOR          1  ///< Subtraction (must be followed by unsigned)
#define ARM64_RELOC_BRANCH26            2  ///< 26-bit branch
#define ARM64_RELOC_PAGE21              3  ///< 21-bit page distance (adrp)
#define ARM64_RELOC_PAGEOFF12           4  ///< 12-bit page offset
#define ARM64_RELOC_GOT_LOAD_PAGE21     5  ///< GOT page (adrp)
#define ARM64_RELOC_GOT_LOAD_PAGEOFF12  6  ///< GOT page offset
#define ARM64_RELOC_POINTER_TO_GOT      7  ///< Pointer to GOT entry
#define ARM64_RELOC_TLVP_LOAD_PAGE21    8  ///< TLV page (adrp)
#define ARM64_RELOC_TLVP_LOAD_PAGEOFF12 9  ///< TLV page offset
#define ARM64_RELOC_ADDEND              10 ///< Addend for next relocation

//
// Mach-O Relocation Types - PowerPC
//
#define PPC_RELOC_VANILLA       0  ///< Generic relocation
#define PPC_RELOC_PAIR          1  ///< Second relocation for sectdiff
#define PPC_RELOC_BR14          2  ///< 14-bit branch
#define PPC_RELOC_BR24          3  ///< 24-bit branch
#define PPC_RELOC_HI16          4  ///< High 16 bits
#define PPC_RELOC_LO16          5  ///< Low 16 bits
#define PPC_RELOC_HA16          6  ///< High-adjusted 16 bits
#define PPC_RELOC_LO14          7  ///< Low 14 bits
#define PPC_RELOC_SECTDIFF      8  ///< Section difference
#define PPC_RELOC_PB_LA_PTR     9  ///< Prebound lazy pointer
#define PPC_RELOC_HI16_SECTDIFF 10 ///< High 16 bits sectdiff
#define PPC_RELOC_LO16_SECTDIFF 11 ///< Low 16 bits sectdiff
#define PPC_RELOC_HA16_SECTDIFF 12 ///< High-adjusted 16 bits sectdiff
#define PPC_RELOC_JBSR          13 ///< Jump BSR (obsolete)
#define PPC_RELOC_LO14_SECTDIFF 14 ///< Low 14 bits sectdiff
#define PPC_RELOC_LOCAL_SECTDIFF 15 ///< Local section difference

//
// Mach-O Relocation Entry Structure
//
typedef struct {
  INT32   Address;      ///< Offset from section start
  UINT32  Symbolnum:24; ///< Symbol index or section ordinal
  UINT32  PCRel:1;      ///< PC relative
  UINT32  Length:2;     ///< 0=byte, 1=word, 2=long, 3=quad
  UINT32  Extern:1;     ///< External symbol
  UINT32  Type:4;       ///< Relocation type
} MACHO_RELOCATION_INFO;

//
// Mach-O Scattered Relocation Entry
//
typedef struct {
  UINT32  Address:24;   ///< Offset from section start
  UINT32  Type:4;       ///< Relocation type
  UINT32  Length:2;     ///< 0=byte, 1=word, 2=long, 3=quad
  UINT32  PCRel:1;      ///< PC relative
  UINT32  Scattered:1;  ///< Must be 1
  INT32   Value;        ///< Value for scattered relocation
} MACHO_SCATTERED_RELOCATION_INFO;

#define SCATTERED_RELOC_FLAG 0x80000000

typedef struct _MACHO_CONTEXT {
  BOOLEAN  ReadOnly;
  BOOLEAN  Is64Bit;
  BOOLEAN  IsFat;
  UINT8    *FileData;
  UINT64   FileSize;
  BOOLEAN  OwnBuffer;

  FAT_HEADER  *FatHeader;
  FAT_ARCH    *FatArchs;
  UINT32      SelectedArch;
  UINT64      CurrentOffset;

  union {
    MACHO_HEADER_32  *Macho32;
    MACHO_HEADER_64  *Macho64;
  } Header;
} MACHO_CONTEXT;

#define MACHO_CONTEXT_FROM_BINFORMAT(ctx) ((MACHO_CONTEXT *)(ctx))

//
// Mach-O Relocation Iterator
//
typedef struct {
  MACHO_CONTEXT  *MachoContext;
  UINT32         CpuType;
  UINT32         SectionIndex;
  UINT32         CurrentIndex;
  UINT32         TotalCount;
  UINT64         RelocOffset;
  UINT8          *RelocData;
} MACHO_RELOCATION_ITERATOR;

//
// Map Mach-O relocation type to universal relocation type
//
STATIC
BINFORMAT_RELOC_TYPE
MachoMapRelocType (
  IN  UINT32   CpuType,
  IN  UINT32   RelocType,
  IN  BOOLEAN  IsScattered
  )
{
  //
  // Handle scattered relocations
  //
  if (IsScattered) {
    switch (RelocType) {
      case GENERIC_RELOC_VANILLA:
        return BinRelocAbsolute32;
      case GENERIC_RELOC_PAIR:
        return BinRelocPair;
      case GENERIC_RELOC_SECTDIFF:
        return BinRelocSubtract;
      case GENERIC_RELOC_LOCAL_SECTDIFF:
        return BinRelocSubtract;
      default:
        return BinRelocFormatSpecific;
    }
  }

  //
  // Map based on CPU type
  //
  switch (CpuType) {
    case MACHO_CPU_TYPE_X86_64:
      switch (RelocType) {
        case X86_64_RELOC_UNSIGNED:
          return BinRelocAbsolute64;
        case X86_64_RELOC_SIGNED:
          return BinRelocPCRelative32;
        case X86_64_RELOC_BRANCH:
          return BinRelocBranch32;
        case X86_64_RELOC_GOT_LOAD:
          return BinRelocGOTLoad;
        case X86_64_RELOC_GOT:
          return BinRelocGOTPCRelative32;
        case X86_64_RELOC_SUBTRACTOR:
          return BinRelocSubtract;
        case X86_64_RELOC_SIGNED_1:
          return BinRelocPCRelative32;
        case X86_64_RELOC_SIGNED_2:
          return BinRelocPCRelative32;
        case X86_64_RELOC_SIGNED_4:
          return BinRelocPCRelative32;
        case X86_64_RELOC_TLV:
          return BinRelocTLSOffset;
        default:
          return BinRelocFormatSpecific;
      }

    case MACHO_CPU_TYPE_I386:
    case MACHO_CPU_TYPE_X86:
      switch (RelocType) {
        case I386_RELOC_VANILLA:
          return BinRelocAbsolute32;
        case I386_RELOC_PAIR:
          return BinRelocPair;
        case I386_RELOC_SECTDIFF:
          return BinRelocSubtract;
        case I386_RELOC_LOCAL_SECTDIFF:
          return BinRelocSubtract;
        case I386_RELOC_TLV:
          return BinRelocTLSOffset;
        default:
          return BinRelocFormatSpecific;
      }

    case MACHO_CPU_TYPE_ARM:
      switch (RelocType) {
        case ARM_RELOC_VANILLA:
          return BinRelocAbsolute32;
        case ARM_RELOC_PAIR:
          return BinRelocPair;
        case ARM_RELOC_SECTDIFF:
          return BinRelocSubtract;
        case ARM_RELOC_LOCAL_SECTDIFF:
          return BinRelocSubtract;
        case ARM_RELOC_BR24:
          return BinRelocBranch24;
        case ARM_THUMB_RELOC_BR22:
          return BinRelocBranch24;
        case ARM_THUMB_32BIT_BRANCH:
          return BinRelocBranch32;
        case ARM_RELOC_HALF:
          return BinRelocAbsolute16;
        case ARM_RELOC_HALF_SECTDIFF:
          return BinRelocSubtract;
        default:
          return BinRelocFormatSpecific;
      }

    case MACHO_CPU_TYPE_ARM64:
      switch (RelocType) {
        case ARM64_RELOC_UNSIGNED:
          return BinRelocAbsolute64;
        case ARM64_RELOC_SUBTRACTOR:
          return BinRelocSubtract;
        case ARM64_RELOC_BRANCH26:
          return BinRelocBranch26;
        case ARM64_RELOC_PAGE21:
          return BinRelocPagePCRelative;
        case ARM64_RELOC_PAGEOFF12:
          return BinRelocPageOffset12;
        case ARM64_RELOC_GOT_LOAD_PAGE21:
          return BinRelocGOTPageOffset;
        case ARM64_RELOC_GOT_LOAD_PAGEOFF12:
          return BinRelocGOTPageOffset;
        case ARM64_RELOC_POINTER_TO_GOT:
          return BinRelocGOTPCRelative64;
        case ARM64_RELOC_TLVP_LOAD_PAGE21:
          return BinRelocTLSOffset;
        case ARM64_RELOC_TLVP_LOAD_PAGEOFF12:
          return BinRelocTLSOffset;
        case ARM64_RELOC_ADDEND:
          return BinRelocFormatSpecific;  // Special handling needed
        default:
          return BinRelocFormatSpecific;
      }

    case MACHO_CPU_TYPE_POWERPC:
    case MACHO_CPU_TYPE_POWERPC64:
      switch (RelocType) {
        case PPC_RELOC_VANILLA:
          return BinRelocAbsolute32;
        case PPC_RELOC_PAIR:
          return BinRelocPair;
        case PPC_RELOC_BR14:
          return BinRelocBranch14;
        case PPC_RELOC_BR24:
          return BinRelocBranch24;
        case PPC_RELOC_HI16:
          return BinRelocAbsolute16;
        case PPC_RELOC_LO16:
          return BinRelocAbsolute16;
        case PPC_RELOC_HA16:
          return BinRelocAbsolute16;
        case PPC_RELOC_LO14:
          return BinRelocAbsolute16;
        case PPC_RELOC_SECTDIFF:
          return BinRelocSubtract;
        case PPC_RELOC_LOCAL_SECTDIFF:
          return BinRelocSubtract;
        default:
          return BinRelocFormatSpecific;
      }

    default:
      return BinRelocFormatSpecific;
  }
}

STATIC
BINFORMAT_STATUS
MachoInitFile (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  )
{
  FILE           *File;
  MACHO_CONTEXT  *MachoCtx;
  UINT64         FileSize;

  if (Context == NULL || FilePath == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  File = fopen(FilePath, ReadOnly ? "rb" : "r+b");
  if (File == NULL) {
    return BINFORMAT_ERROR_IO;
  }

  fseek(File, 0, SEEK_END);
  FileSize = ftell(File);
  fseek(File, 0, SEEK_SET);

  MachoCtx = (MACHO_CONTEXT *)calloc(1, sizeof(MACHO_CONTEXT));
  if (MachoCtx == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  MachoCtx->FileData = (UINT8 *)malloc(FileSize);
  if (MachoCtx->FileData == NULL) {
    free(MachoCtx);
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  if (fread(MachoCtx->FileData, 1, FileSize, File) != FileSize) {
    free(MachoCtx->FileData);
    free(MachoCtx);
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);

  MachoCtx->FileSize = FileSize;
  MachoCtx->OwnBuffer = TRUE;
  MachoCtx->ReadOnly = ReadOnly;

  //
  // Check for Fat binary
  //
  UINT32 Magic = *(UINT32 *)MachoCtx->FileData;
  if (Magic == MACHO_FAT_MAGIC || Magic == MACHO_FAT_MAGIC_SWAP) {
    MachoCtx->IsFat = TRUE;
    MachoCtx->FatHeader = (FAT_HEADER *)MachoCtx->FileData;
    MachoCtx->FatArchs = (FAT_ARCH *)(MachoCtx->FileData + sizeof(FAT_HEADER));
    MachoCtx->SelectedArch = 0;

    if (MachoCtx->FatHeader->NumArchs > 0) {
      MachoCtx->CurrentOffset = MachoCtx->FatArchs[0].Offset;
    }

    Magic = *(UINT32 *)(MachoCtx->FileData + MachoCtx->CurrentOffset);
  }

  //
  // Parse Mach-O header
  //
  if (Magic == MACHO_MAGIC_64 || Magic == MACHO_MAGIC_64_SWAP) {
    MachoCtx->Is64Bit = TRUE;
    MachoCtx->Header.Macho64 = (MACHO_HEADER_64 *)(MachoCtx->FileData + MachoCtx->CurrentOffset);
  } else if (Magic == MACHO_MAGIC_32 || Magic == MACHO_MAGIC_32_SWAP) {
    MachoCtx->Is64Bit = FALSE;
    MachoCtx->Header.Macho32 = (MACHO_HEADER_32 *)(MachoCtx->FileData + MachoCtx->CurrentOffset);
  } else {
    free(MachoCtx->FileData);
    free(MachoCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  *Context = (BINFORMAT_CONTEXT *)MachoCtx;
  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS MachoInitMemory(OUT BINFORMAT_CONTEXT **Ctx, IN CONST VOID *Buf, IN UINT64 Size) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoCreate(OUT BINFORMAT_CONTEXT **Ctx, IN BINFORMAT_FILE_TYPE Type, IN BINFORMAT_MACHINE Mach, IN BOOLEAN Is64) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }

STATIC
VOID
MachoClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  MACHO_CONTEXT  *MachoCtx;

  if (Context == NULL) {
    return;
  }

  MachoCtx = MACHO_CONTEXT_FROM_BINFORMAT(Context);

  if (MachoCtx->OwnBuffer && MachoCtx->FileData != NULL) {
    free(MachoCtx->FileData);
  }

  free(MachoCtx);
}

STATIC
BINFORMAT_STATUS
MachoGetHeader (
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  )
{
  MACHO_CONTEXT  *MachoCtx;

  if (Context == NULL || HeaderInfo == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MachoCtx = MACHO_CONTEXT_FROM_BINFORMAT(Context);
  memset(HeaderInfo, 0, sizeof(BINFORMAT_HEADER_INFO));

  HeaderInfo->Is64Bit = MachoCtx->Is64Bit;
  HeaderInfo->Endianness = BinEndianLittle;

  if (MachoCtx->IsFat && MachoCtx->FatHeader != NULL) {
    HeaderInfo->ArchitectureCount = MachoCtx->FatHeader->NumArchs;
    for (UINT32 i = 0; i < MachoCtx->FatHeader->NumArchs && i < BINFORMAT_MAX_ARCHITECTURES; i++) {
      HeaderInfo->Architectures[i].CpuSubtype = MachoCtx->FatArchs[i].CpuSubtype;
      HeaderInfo->Architectures[i].Offset = MachoCtx->FatArchs[i].Offset;
      HeaderInfo->Architectures[i].Size = MachoCtx->FatArchs[i].Size;
      HeaderInfo->Architectures[i].Alignment = MachoCtx->FatArchs[i].Align;
    }
  }

  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS MachoGetSection(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoGetSectionByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoGetSegment(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SEGMENT *Seg) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoGetSymbol(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoGetSymbolByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoGetRelocations(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, OUT BINFORMAT_RELOCATION **Rels, OUT UINT32 *Cnt) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoAddSection(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SECTION *Sec, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoAddSymbol(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SYMBOL *Sym, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoAddRelocation(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, IN BINFORMAT_RELOCATION *Rel) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC
BINFORMAT_STATUS
MachoWriteFile (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  CONST CHAR8        *Path
  )
{
  MACHO_CONTEXT  *MachoCtx;
  FILE           *File;

  if (Context == NULL || Path == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MachoCtx = MACHO_CONTEXT_FROM_BINFORMAT(Context);

  if (MachoCtx->FileData == NULL || MachoCtx->FileSize == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  File = fopen(Path, "wb");
  if (File == NULL) {
    return BINFORMAT_ERROR_IO;
  }

  if (fwrite(MachoCtx->FileData, 1, MachoCtx->FileSize, File) != MachoCtx->FileSize) {
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);
  return BINFORMAT_SUCCESS;
}
STATIC
BINFORMAT_STATUS
MachoWriteMemory (
  IN  BINFORMAT_CONTEXT  *Context,
  OUT VOID               *Buffer,
  IN  UINT64             Size,
  OUT UINT64             *Written
  )
{
  MACHO_CONTEXT  *MachoCtx;

  if (Context == NULL || Buffer == NULL || Written == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MachoCtx = MACHO_CONTEXT_FROM_BINFORMAT(Context);

  if (MachoCtx->FileData == NULL || MachoCtx->FileSize == 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (Size < MachoCtx->FileSize) {
    return BINFORMAT_ERROR_BUFFER_TOO_SMALL;
  }

  memcpy(Buffer, MachoCtx->FileData, MachoCtx->FileSize);
  *Written = MachoCtx->FileSize;

  return BINFORMAT_SUCCESS;
}
STATIC
BINFORMAT_STATUS
MachoSelectArchitecture (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  UINT32             ArchIndex
  )
{
  MACHO_CONTEXT  *MachoCtx;
  UINT32         Magic;

  if (Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MachoCtx = MACHO_CONTEXT_FROM_BINFORMAT(Context);

  if (!MachoCtx->IsFat) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (ArchIndex >= MachoCtx->FatHeader->NumArchs) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MachoCtx->SelectedArch = ArchIndex;
  MachoCtx->CurrentOffset = MachoCtx->FatArchs[ArchIndex].Offset;

  //
  // Update header pointer to selected architecture
  //
  Magic = *(UINT32 *)(MachoCtx->FileData + MachoCtx->CurrentOffset);

  if (Magic == MACHO_MAGIC_64 || Magic == MACHO_MAGIC_64_SWAP) {
    MachoCtx->Is64Bit = TRUE;
    MachoCtx->Header.Macho64 = (MACHO_HEADER_64 *)(MachoCtx->FileData + MachoCtx->CurrentOffset);
  } else if (Magic == MACHO_MAGIC_32 || Magic == MACHO_MAGIC_32_SWAP) {
    MachoCtx->Is64Bit = FALSE;
    MachoCtx->Header.Macho32 = (MACHO_HEADER_32 *)(MachoCtx->FileData + MachoCtx->CurrentOffset);
  } else {
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  return BINFORMAT_SUCCESS;
}

STATIC
BINFORMAT_STATUS
MachoExtractThin (
  IN  BINFORMAT_CONTEXT   *FatContext,
  IN  UINT32              ArchIndex,
  OUT BINFORMAT_CONTEXT   **ThinContext
  )
{
  MACHO_CONTEXT  *FatCtx;
  MACHO_CONTEXT  *ThinCtx;
  UINT32         Offset;
  UINT32         Size;
  UINT32         Magic;

  if (FatContext == NULL || ThinContext == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  FatCtx = MACHO_CONTEXT_FROM_BINFORMAT(FatContext);

  if (!FatCtx->IsFat) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (ArchIndex >= FatCtx->FatHeader->NumArchs) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Get architecture slice info
  //
  Offset = FatCtx->FatArchs[ArchIndex].Offset;
  Size = FatCtx->FatArchs[ArchIndex].Size;

  //
  // Create new context for thin binary
  //
  ThinCtx = (MACHO_CONTEXT *)calloc(1, sizeof(MACHO_CONTEXT));
  if (ThinCtx == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Allocate buffer and copy thin slice
  //
  ThinCtx->FileData = (UINT8 *)malloc(Size);
  if (ThinCtx->FileData == NULL) {
    free(ThinCtx);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  memcpy(ThinCtx->FileData, FatCtx->FileData + Offset, Size);
  ThinCtx->FileSize = Size;
  ThinCtx->OwnBuffer = TRUE;
  ThinCtx->ReadOnly = FALSE;
  ThinCtx->IsFat = FALSE;
  ThinCtx->CurrentOffset = 0;

  //
  // Parse thin binary header
  //
  Magic = *(UINT32 *)ThinCtx->FileData;

  if (Magic == MACHO_MAGIC_64 || Magic == MACHO_MAGIC_64_SWAP) {
    ThinCtx->Is64Bit = TRUE;
    ThinCtx->Header.Macho64 = (MACHO_HEADER_64 *)ThinCtx->FileData;
  } else if (Magic == MACHO_MAGIC_32 || Magic == MACHO_MAGIC_32_SWAP) {
    ThinCtx->Is64Bit = FALSE;
    ThinCtx->Header.Macho32 = (MACHO_HEADER_32 *)ThinCtx->FileData;
  } else {
    free(ThinCtx->FileData);
    free(ThinCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  *ThinContext = (BINFORMAT_CONTEXT *)ThinCtx;
  return BINFORMAT_SUCCESS;
}

STATIC
BINFORMAT_STATUS
MachoCreateFat (
  OUT BINFORMAT_CONTEXT     **Context,
  IN  BINFORMAT_FILE_TYPE   FileType,
  IN  CONST CHAR8           *Format
  )
{
  MACHO_CONTEXT  *MachoCtx;
  FAT_HEADER     *FatHdr;

  if (Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Create empty fat binary
  //
  MachoCtx = (MACHO_CONTEXT *)calloc(1, sizeof(MACHO_CONTEXT));
  if (MachoCtx == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Allocate initial buffer for fat header (will grow as slices are added)
  //
  MachoCtx->FileData = (UINT8 *)malloc(sizeof(FAT_HEADER));
  if (MachoCtx->FileData == NULL) {
    free(MachoCtx);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  MachoCtx->FileSize = sizeof(FAT_HEADER);
  MachoCtx->OwnBuffer = TRUE;
  MachoCtx->ReadOnly = FALSE;
  MachoCtx->IsFat = TRUE;

  //
  // Initialize fat header
  //
  FatHdr = (FAT_HEADER *)MachoCtx->FileData;
  FatHdr->Magic = MACHO_FAT_MAGIC;
  FatHdr->NumArchs = 0;

  MachoCtx->FatHeader = FatHdr;
  MachoCtx->FatArchs = NULL;

  *Context = (BINFORMAT_CONTEXT *)MachoCtx;
  return BINFORMAT_SUCCESS;
}

STATIC
BINFORMAT_STATUS
MachoAddArchSlice (
  IN  BINFORMAT_CONTEXT  *FatContext,
  IN  BINFORMAT_CONTEXT  *ThinContext,
  OUT UINT32             *ArchIndex
  )
{
  MACHO_CONTEXT  *FatCtx;
  MACHO_CONTEXT  *ThinCtx;
  UINT8          *NewData;
  UINT64         NewSize;
  UINT32         NumArchs;
  UINT32         HeaderSize;
  UINT32         DataOffset;
  FAT_ARCH       *NewArchs;
  UINT32         i;

  if (FatContext == NULL || ThinContext == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  FatCtx = MACHO_CONTEXT_FROM_BINFORMAT(FatContext);
  ThinCtx = MACHO_CONTEXT_FROM_BINFORMAT(ThinContext);

  if (!FatCtx->IsFat || ThinCtx->IsFat) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  NumArchs = FatCtx->FatHeader->NumArchs + 1;
  HeaderSize = sizeof(FAT_HEADER) + (NumArchs * sizeof(FAT_ARCH));

  //
  // Calculate total size: header + existing data + new slice (aligned)
  //
  DataOffset = (HeaderSize + 0xFFF) & ~0xFFF;  // Align to 4096
  NewSize = DataOffset + ThinCtx->FileSize;

  //
  // Allocate new buffer
  //
  NewData = (UINT8 *)malloc(NewSize);
  if (NewData == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  memset(NewData, 0, NewSize);

  //
  // Copy fat header
  //
  memcpy(NewData, FatCtx->FileData, sizeof(FAT_HEADER));

  //
  // Copy existing arch entries and adjust offsets
  //
  NewArchs = (FAT_ARCH *)(NewData + sizeof(FAT_HEADER));
  UINT32 CurrentDataOffset = DataOffset;

  for (i = 0; i < FatCtx->FatHeader->NumArchs; i++) {
    NewArchs[i].CpuType = FatCtx->FatArchs[i].CpuType;
    NewArchs[i].CpuSubtype = FatCtx->FatArchs[i].CpuSubtype;
    NewArchs[i].Offset = CurrentDataOffset;
    NewArchs[i].Size = FatCtx->FatArchs[i].Size;
    NewArchs[i].Align = FatCtx->FatArchs[i].Align;

    //
    // Copy existing slice data
    //
    memcpy(NewData + CurrentDataOffset,
           FatCtx->FileData + FatCtx->FatArchs[i].Offset,
           FatCtx->FatArchs[i].Size);

    CurrentDataOffset += (FatCtx->FatArchs[i].Size + 0xFFF) & ~0xFFF;  // Align
  }

  //
  // Add new architecture entry
  //
  NewArchs[NumArchs - 1].CpuType = ThinCtx->Is64Bit ? 0x01000007 : 0x00000007;  // x86_64 or x86
  NewArchs[NumArchs - 1].CpuSubtype = 3;
  NewArchs[NumArchs - 1].Offset = CurrentDataOffset;
  NewArchs[NumArchs - 1].Size = ThinCtx->FileSize;
  NewArchs[NumArchs - 1].Align = 12;  // 2^12 = 4096

  //
  // Copy new slice data
  //
  memcpy(NewData + CurrentDataOffset, ThinCtx->FileData, ThinCtx->FileSize);

  //
  // Update fat header
  //
  ((FAT_HEADER *)NewData)->NumArchs = NumArchs;

  //
  // Replace fat context data
  //
  if (FatCtx->OwnBuffer && FatCtx->FileData != NULL) {
    free(FatCtx->FileData);
  }

  FatCtx->FileData = NewData;
  FatCtx->FileSize = NewSize;
  FatCtx->OwnBuffer = TRUE;
  FatCtx->FatHeader = (FAT_HEADER *)NewData;
  FatCtx->FatArchs = (FAT_ARCH *)(NewData + sizeof(FAT_HEADER));

  if (ArchIndex != NULL) {
    *ArchIndex = NumArchs - 1;
  }

  return BINFORMAT_SUCCESS;
}

STATIC
BINFORMAT_STATUS
MachoRemoveArchSlice (
  IN  BINFORMAT_CONTEXT  *FatContext,
  IN  UINT32             ArchIndex
  )
{
  MACHO_CONTEXT  *FatCtx;
  UINT8          *NewData;
  UINT64         NewSize;
  UINT32         NumArchs;
  UINT32         HeaderSize;
  UINT32         DataOffset;
  FAT_ARCH       *NewArchs;
  UINT32         i, j;

  if (FatContext == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  FatCtx = MACHO_CONTEXT_FROM_BINFORMAT(FatContext);

  if (!FatCtx->IsFat) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (ArchIndex >= FatCtx->FatHeader->NumArchs) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  if (FatCtx->FatHeader->NumArchs == 1) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;  // Can't remove last architecture
  }

  NumArchs = FatCtx->FatHeader->NumArchs - 1;
  HeaderSize = sizeof(FAT_HEADER) + (NumArchs * sizeof(FAT_ARCH));
  DataOffset = (HeaderSize + 0xFFF) & ~0xFFF;

  //
  // Calculate new total size
  //
  NewSize = DataOffset;
  for (i = 0; i < FatCtx->FatHeader->NumArchs; i++) {
    if (i != ArchIndex) {
      NewSize += (FatCtx->FatArchs[i].Size + 0xFFF) & ~0xFFF;
    }
  }

  //
  // Allocate new buffer
  //
  NewData = (UINT8 *)malloc(NewSize);
  if (NewData == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  memset(NewData, 0, NewSize);

  //
  // Copy fat header
  //
  memcpy(NewData, FatCtx->FileData, sizeof(FAT_HEADER));
  ((FAT_HEADER *)NewData)->NumArchs = NumArchs;

  //
  // Copy arch entries except the removed one
  //
  NewArchs = (FAT_ARCH *)(NewData + sizeof(FAT_HEADER));
  UINT32 CurrentDataOffset = DataOffset;

  for (i = 0, j = 0; i < FatCtx->FatHeader->NumArchs; i++) {
    if (i == ArchIndex) {
      continue;  // Skip removed architecture
    }

    NewArchs[j].CpuType = FatCtx->FatArchs[i].CpuType;
    NewArchs[j].CpuSubtype = FatCtx->FatArchs[i].CpuSubtype;
    NewArchs[j].Offset = CurrentDataOffset;
    NewArchs[j].Size = FatCtx->FatArchs[i].Size;
    NewArchs[j].Align = FatCtx->FatArchs[i].Align;

    //
    // Copy slice data
    //
    memcpy(NewData + CurrentDataOffset,
           FatCtx->FileData + FatCtx->FatArchs[i].Offset,
           FatCtx->FatArchs[i].Size);

    CurrentDataOffset += (FatCtx->FatArchs[i].Size + 0xFFF) & ~0xFFF;
    j++;
  }

  //
  // Replace fat context data
  //
  if (FatCtx->OwnBuffer && FatCtx->FileData != NULL) {
    free(FatCtx->FileData);
  }

  FatCtx->FileData = NewData;
  FatCtx->FileSize = NewSize;
  FatCtx->OwnBuffer = TRUE;
  FatCtx->FatHeader = (FAT_HEADER *)NewData;
  FatCtx->FatArchs = (FAT_ARCH *)(NewData + sizeof(FAT_HEADER));

  return BINFORMAT_SUCCESS;
}

STATIC
BINFORMAT_STATUS
MachoReplaceArchSlice (
  IN  BINFORMAT_CONTEXT  *FatContext,
  IN  UINT32             ArchIndex,
  IN  BINFORMAT_CONTEXT  *ThinContext
  )
{
  BINFORMAT_STATUS  Status;

  //
  // Replace is implemented as remove + add
  //
  Status = MachoRemoveArchSlice(FatContext, ArchIndex);
  if (BINFORMAT_IS_ERROR(Status)) {
    return Status;
  }

  return MachoAddArchSlice(FatContext, ThinContext, NULL);
}

//
// Relocation Iterator Functions
//
STATIC
BINFORMAT_STATUS
MachoRelocationIterCreate (
  IN  BINFORMAT_CONTEXT               *Context,
  IN  UINT32                          SectionIndex,
  OUT BINFORMAT_RELOCATION_ITERATOR   **Iterator
  )
{
  MACHO_CONTEXT              *MachoCtx;
  MACHO_RELOCATION_ITERATOR  *Iter;
  UINT8                      *LoadCmdPtr;
  UINT32                     i;
  UINT32                     CmdSize;
  UINT32                     Cmd;
  UINT32                     NumCmds;
  UINT32                     SectionCount;
  UINT32                     CpuType;

  if (Context == NULL || Iterator == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MachoCtx = MACHO_CONTEXT_FROM_BINFORMAT(Context);

  //
  // Get CPU type
  //
  if (MachoCtx->Is64Bit) {
    CpuType = MachoCtx->Header.Macho64->CpuType;
    NumCmds = MachoCtx->Header.Macho64->NumCmds;
    LoadCmdPtr = MachoCtx->FileData + MachoCtx->CurrentOffset + sizeof(MACHO_HEADER_64);
  } else {
    CpuType = MachoCtx->Header.Macho32->CpuType;
    NumCmds = MachoCtx->Header.Macho32->NumCmds;
    LoadCmdPtr = MachoCtx->FileData + MachoCtx->CurrentOffset + sizeof(MACHO_HEADER_32);
  }

  //
  // Walk through load commands to find the section
  // Note: Mach-O stores sections within segments (LC_SEGMENT/LC_SEGMENT_64)
  // For now, return NOT_IMPLEMENTED as full section parsing is not yet complete
  //
  // TODO: Implement full section parsing when MachoGetSection is implemented
  //

  SectionCount = 0;
  for (i = 0; i < NumCmds; i++) {
    Cmd = *(UINT32 *)LoadCmdPtr;
    CmdSize = *((UINT32 *)LoadCmdPtr + 1);

    if (Cmd == LC_SEGMENT || Cmd == LC_SEGMENT_64) {
      //
      // Parse segment command to extract sections and their relocations
      // Each section has: nreloc (count) and reloff (file offset)
      //
      // This requires full segment/section structure parsing
      // which will be implemented when section enumeration is complete
      //
    }

    LoadCmdPtr += CmdSize;
  }

  //
  // For now, return NOT_IMPLEMENTED
  // This will be implemented when MachoGetSection and full section
  // parsing is completed
  //
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

STATIC
BINFORMAT_STATUS
MachoRelocationIterNext (
  IN  BINFORMAT_RELOCATION_ITERATOR  *Iterator,
  OUT BINFORMAT_RELOCATION           *Relocation
  )
{
  MACHO_RELOCATION_ITERATOR       *Iter;
  MACHO_RELOCATION_INFO           *RelocInfo;
  MACHO_SCATTERED_RELOCATION_INFO *ScatteredInfo;
  UINT32                          *RelocWord;
  BOOLEAN                         IsScattered;
  UINT32                          RelocType;
  UINT8                           RelocLength;

  if (Iterator == NULL || Relocation == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  Iter = (MACHO_RELOCATION_ITERATOR *)Iterator;

  if (Iter->CurrentIndex >= Iter->TotalCount) {
    return BINFORMAT_ERROR_END_OF_DATA;
  }

  //
  // Read relocation entry from current position
  //
  RelocWord = (UINT32 *)(Iter->RelocData + (Iter->CurrentIndex * sizeof(MACHO_RELOCATION_INFO)));

  //
  // Check if this is a scattered relocation
  // Scattered relocations have the high bit set in the first word
  //
  IsScattered = ((*RelocWord & SCATTERED_RELOC_FLAG) != 0);

  memset(Relocation, 0, sizeof(BINFORMAT_RELOCATION));

  if (IsScattered) {
    //
    // Scattered relocation
    //
    ScatteredInfo = (MACHO_SCATTERED_RELOCATION_INFO *)RelocWord;

    Relocation->Offset = ScatteredInfo->Address;
    Relocation->IsPcRel = ScatteredInfo->PCRel;
    RelocLength = ScatteredInfo->Length;
    RelocType = ScatteredInfo->Type;
    Relocation->Addend = ScatteredInfo->Value;
    Relocation->IsScattered = TRUE;
    Relocation->IsExtern = FALSE;
    Relocation->SymbolIndex = 0;
  } else {
    //
    // Standard relocation
    //
    RelocInfo = (MACHO_RELOCATION_INFO *)RelocWord;

    Relocation->Offset = RelocInfo->Address;
    Relocation->IsPcRel = RelocInfo->PCRel;
    RelocLength = RelocInfo->Length;
    RelocType = RelocInfo->Type;
    Relocation->IsScattered = FALSE;
    Relocation->IsExtern = RelocInfo->Extern;

    if (RelocInfo->Extern) {
      Relocation->SymbolIndex = RelocInfo->Symbolnum;
    } else {
      Relocation->SectionIndex = RelocInfo->Symbolnum;
    }

    Relocation->Addend = 0;  // Mach-O doesn't have explicit addend in relocation
  }

  //
  // Convert length to size in bytes
  // 0=byte(1), 1=word(2), 2=long(4), 3=quad(8)
  //
  Relocation->Length = (1 << RelocLength);

  //
  // Store native type and map to universal type
  //
  Relocation->NativeType = RelocType;
  Relocation->Type = MachoMapRelocType(Iter->CpuType, RelocType, IsScattered);

  //
  // Move to next relocation
  //
  Iter->CurrentIndex++;

  return BINFORMAT_SUCCESS;
}

STATIC
VOID
MachoRelocationIterFree (
  IN  BINFORMAT_RELOCATION_ITERATOR  *Iterator
  )
{
  if (Iterator == NULL) {
    return;
  }

  free(Iterator);
}

STATIC CONST BINFORMAT_API gMachoApi = {
  .LibraryName = "libmacho",
  .Version = 1,
  .InitFile = MachoInitFile,
  .InitMemory = MachoInitMemory,
  .Create = MachoCreate,
  .Close = MachoClose,
  .GetHeader = MachoGetHeader,
  .GetSection = MachoGetSection,
  .GetSectionByName = MachoGetSectionByName,
  .GetSegment = MachoGetSegment,
  .GetSymbol = MachoGetSymbol,
  .GetSymbolByName = MachoGetSymbolByName,
  .AddSection = MachoAddSection,
  .AddSymbol = MachoAddSymbol,
  .AddRelocation = MachoAddRelocation,
  .WriteFile = MachoWriteFile,
  .WriteMemory = MachoWriteMemory,
  .SelectArchitecture = MachoSelectArchitecture,
  .CreateFat = MachoCreateFat,
  .AddArchSlice = MachoAddArchSlice,
  .RemoveArchSlice = MachoRemoveArchSlice,
  .ReplaceArchSlice = MachoReplaceArchSlice,
  .ExtractThin = MachoExtractThin,
  .GetSignature = NULL,
  .VerifySignature = NULL,
  .SignBinary = NULL,
  .RemoveSignature = NULL,
  .UpdateSection = NULL,
  .UpdateSymbol = NULL,
  .UpdateRelocation = NULL,
  .SectionIterCreate = NULL,
  .SectionIterNext = NULL,
  .SectionIterFree = NULL,
  .SymbolIterCreate = NULL,
  .SymbolIterNext = NULL,
  .SymbolIterFree = NULL,
  .SegmentIterCreate = NULL,
  .SegmentIterNext = NULL,
  .SegmentIterFree = NULL,
  .RelocationIterCreate = MachoRelocationIterCreate,
  .RelocationIterNext = MachoRelocationIterNext,
  .RelocationIterFree = MachoRelocationIterFree,
  .ArchIterCreate = NULL,
  .ArchIterNext = NULL,
  .ArchIterFree = NULL
};

CONST BINFORMAT_API *
MachoGetApi (
  VOID
  )
{
  return &gMachoApi;
}
