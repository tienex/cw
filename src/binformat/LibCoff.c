/** @file
  COFF Binary Format Library Implementation.

  This file implements comprehensive support for PE/COFF, BigObj, SVR3 COFF,
  XCOFF32/64, ECOFF32/64, and TE binary formats following the unified binary
  format API.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LibCoff.h"

//
// COFF File Header Flags
//
#define IMAGE_FILE_RELOCS_STRIPPED          0x0001
#define IMAGE_FILE_EXECUTABLE_IMAGE         0x0002
#define IMAGE_FILE_LINE_NUMS_STRIPPED       0x0004
#define IMAGE_FILE_LOCAL_SYMS_STRIPPED      0x0008
#define IMAGE_FILE_AGGRESSIVE_WS_TRIM       0x0010
#define IMAGE_FILE_LARGE_ADDRESS_AWARE      0x0020
#define IMAGE_FILE_BYTES_REVERSED_LO        0x0080
#define IMAGE_FILE_32BIT_MACHINE            0x0100
#define IMAGE_FILE_DEBUG_STRIPPED           0x0200
#define IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP  0x0400
#define IMAGE_FILE_NET_RUN_FROM_SWAP        0x0800
#define IMAGE_FILE_SYSTEM                   0x1000
#define IMAGE_FILE_DLL                      0x2000
#define IMAGE_FILE_UP_SYSTEM_ONLY           0x4000
#define IMAGE_FILE_BYTES_REVERSED_HI        0x8000

//
// Section Characteristics
//
#define IMAGE_SCN_CNT_CODE                  0x00000020
#define IMAGE_SCN_CNT_INITIALIZED_DATA      0x00000040
#define IMAGE_SCN_CNT_UNINITIALIZED_DATA    0x00000080
#define IMAGE_SCN_MEM_EXECUTE               0x20000000
#define IMAGE_SCN_MEM_READ                  0x40000000
#define IMAGE_SCN_MEM_WRITE                 0x80000000

//
// x86-64 Relocation Types
//
#define IMAGE_REL_AMD64_ABSOLUTE            0x0000  ///< No relocation
#define IMAGE_REL_AMD64_ADDR64              0x0001  ///< 64-bit address
#define IMAGE_REL_AMD64_ADDR32              0x0002  ///< 32-bit address
#define IMAGE_REL_AMD64_ADDR32NB            0x0003  ///< 32-bit address without base
#define IMAGE_REL_AMD64_REL32               0x0004  ///< 32-bit relative
#define IMAGE_REL_AMD64_REL32_1             0x0005  ///< 32-bit relative +1
#define IMAGE_REL_AMD64_REL32_2             0x0006  ///< 32-bit relative +2
#define IMAGE_REL_AMD64_REL32_3             0x0007  ///< 32-bit relative +3
#define IMAGE_REL_AMD64_REL32_4             0x0008  ///< 32-bit relative +4
#define IMAGE_REL_AMD64_REL32_5             0x0009  ///< 32-bit relative +5
#define IMAGE_REL_AMD64_SECTION             0x000A  ///< Section index
#define IMAGE_REL_AMD64_SECREL              0x000B  ///< 32-bit offset from section base
#define IMAGE_REL_AMD64_SECREL7             0x000C  ///< 7-bit offset from section base
#define IMAGE_REL_AMD64_TOKEN               0x000D  ///< CLR token
#define IMAGE_REL_AMD64_SREL32              0x000E  ///< 32-bit signed span
#define IMAGE_REL_AMD64_PAIR                0x000F  ///< Pair relocation
#define IMAGE_REL_AMD64_SSPAN32             0x0010  ///< 32-bit signed span

//
// i386 Relocation Types
//
#define IMAGE_REL_I386_ABSOLUTE             0x0000  ///< No relocation
#define IMAGE_REL_I386_DIR16                0x0001  ///< 16-bit direct
#define IMAGE_REL_I386_REL16                0x0002  ///< 16-bit PC-relative
#define IMAGE_REL_I386_DIR32                0x0006  ///< 32-bit direct
#define IMAGE_REL_I386_DIR32NB              0x0007  ///< 32-bit direct without base
#define IMAGE_REL_I386_SEG12                0x0009  ///< 16-bit segment
#define IMAGE_REL_I386_SECTION              0x000A  ///< Section index
#define IMAGE_REL_I386_SECREL               0x000B  ///< 32-bit offset from section base
#define IMAGE_REL_I386_TOKEN                0x000C  ///< CLR token
#define IMAGE_REL_I386_SECREL7              0x000D  ///< 7-bit offset from section base
#define IMAGE_REL_I386_REL32                0x0014  ///< 32-bit PC-relative

//
// ARM Relocation Types
//
#define IMAGE_REL_ARM_ABSOLUTE              0x0000  ///< No relocation
#define IMAGE_REL_ARM_ADDR32                0x0001  ///< 32-bit address
#define IMAGE_REL_ARM_ADDR32NB              0x0002  ///< 32-bit address without base
#define IMAGE_REL_ARM_BRANCH24              0x0003  ///< 24-bit relative branch
#define IMAGE_REL_ARM_BRANCH11              0x0004  ///< 11-bit relative branch
#define IMAGE_REL_ARM_SECTION               0x000E  ///< Section index
#define IMAGE_REL_ARM_SECREL                0x000F  ///< 32-bit offset from section base
#define IMAGE_REL_ARM_MOV32                 0x0010  ///< 32-bit immediate (MOVW/MOVT pair)
#define IMAGE_REL_THUMB_MOV32               0x0011  ///< 32-bit immediate (Thumb MOVW/MOVT)
#define IMAGE_REL_THUMB_BRANCH20            0x0012  ///< 20-bit relative branch (Thumb)
#define IMAGE_REL_THUMB_BRANCH24            0x0014  ///< 24-bit relative branch (Thumb)
#define IMAGE_REL_THUMB_BLX23               0x0015  ///< 23-bit BLX (Thumb)

//
// ARM64 Relocation Types
//
#define IMAGE_REL_ARM64_ABSOLUTE            0x0000  ///< No relocation
#define IMAGE_REL_ARM64_ADDR32              0x0001  ///< 32-bit address
#define IMAGE_REL_ARM64_ADDR32NB            0x0002  ///< 32-bit address without base
#define IMAGE_REL_ARM64_BRANCH26            0x0003  ///< 26-bit relative branch
#define IMAGE_REL_ARM64_PAGEBASE_REL21      0x0004  ///< 21-bit page base relative
#define IMAGE_REL_ARM64_REL21               0x0005  ///< 21-bit relative
#define IMAGE_REL_ARM64_PAGEOFFSET_12A      0x0006  ///< 12-bit page offset (ADD)
#define IMAGE_REL_ARM64_PAGEOFFSET_12L      0x0007  ///< 12-bit page offset (LDR)
#define IMAGE_REL_ARM64_SECREL              0x0008  ///< 32-bit offset from section base
#define IMAGE_REL_ARM64_SECREL_LOW12A       0x0009  ///< 12-bit offset from section base (ADD)
#define IMAGE_REL_ARM64_SECREL_HIGH12A      0x000A  ///< 12-bit high offset from section base
#define IMAGE_REL_ARM64_SECREL_LOW12L       0x000B  ///< 12-bit offset from section base (LDR)
#define IMAGE_REL_ARM64_TOKEN               0x000C  ///< CLR token
#define IMAGE_REL_ARM64_SECTION             0x000D  ///< Section index
#define IMAGE_REL_ARM64_ADDR64              0x000E  ///< 64-bit address

//
/// COFF Structures
//
#pragma pack(push, 1)

///
/// Standard COFF File Header
///
typedef struct {
  UINT16  Machine;              ///< Machine type
  UINT16  NumberOfSections;     ///< Number of sections
  UINT32  TimeDateStamp;        ///< Time/date stamp
  UINT32  PointerToSymbolTable; ///< File offset of symbol table
  UINT32  NumberOfSymbols;      ///< Number of symbol table entries
  UINT16  SizeOfOptionalHeader; ///< Size of optional header
  UINT16  Characteristics;      ///< Characteristics flags
} COFF_FILE_HEADER;

///
/// PE Optional Header (PE32)
///
typedef struct {
  UINT16  Magic;                    ///< Magic number
  UINT8   MajorLinkerVersion;       ///< Linker major version
  UINT8   MinorLinkerVersion;       ///< Linker minor version
  UINT32  SizeOfCode;               ///< Size of code section
  UINT32  SizeOfInitializedData;    ///< Size of initialized data
  UINT32  SizeOfUninitializedData;  ///< Size of uninitialized data
  UINT32  AddressOfEntryPoint;      ///< Entry point RVA
  UINT32  BaseOfCode;               ///< Base of code RVA
  UINT32  BaseOfData;               ///< Base of data RVA
  UINT32  ImageBase;                ///< Image base address
  UINT32  SectionAlignment;         ///< Section alignment
  UINT32  FileAlignment;            ///< File alignment
  UINT16  MajorOperatingSystemVersion;
  UINT16  MinorOperatingSystemVersion;
  UINT16  MajorImageVersion;
  UINT16  MinorImageVersion;
  UINT16  MajorSubsystemVersion;
  UINT16  MinorSubsystemVersion;
  UINT32  Win32VersionValue;
  UINT32  SizeOfImage;              ///< Size of image
  UINT32  SizeOfHeaders;            ///< Size of headers
  UINT32  CheckSum;                 ///< Checksum
  UINT16  Subsystem;                ///< Subsystem
  UINT16  DllCharacteristics;       ///< DLL characteristics
  UINT32  SizeOfStackReserve;       ///< Stack reserve size
  UINT32  SizeOfStackCommit;        ///< Stack commit size
  UINT32  SizeOfHeapReserve;        ///< Heap reserve size
  UINT32  SizeOfHeapCommit;         ///< Heap commit size
  UINT32  LoaderFlags;              ///< Loader flags
  UINT32  NumberOfRvaAndSizes;      ///< Number of data directories
} PE32_OPTIONAL_HEADER;

///
/// PE Optional Header (PE32+)
///
typedef struct {
  UINT16  Magic;
  UINT8   MajorLinkerVersion;
  UINT8   MinorLinkerVersion;
  UINT32  SizeOfCode;
  UINT32  SizeOfInitializedData;
  UINT32  SizeOfUninitializedData;
  UINT32  AddressOfEntryPoint;
  UINT32  BaseOfCode;
  UINT64  ImageBase;                ///< Image base address (64-bit)
  UINT32  SectionAlignment;
  UINT32  FileAlignment;
  UINT16  MajorOperatingSystemVersion;
  UINT16  MinorOperatingSystemVersion;
  UINT16  MajorImageVersion;
  UINT16  MinorImageVersion;
  UINT16  MajorSubsystemVersion;
  UINT16  MinorSubsystemVersion;
  UINT32  Win32VersionValue;
  UINT32  SizeOfImage;
  UINT32  SizeOfHeaders;
  UINT32  CheckSum;
  UINT16  Subsystem;
  UINT16  DllCharacteristics;
  UINT64  SizeOfStackReserve;       ///< Stack reserve size (64-bit)
  UINT64  SizeOfStackCommit;
  UINT64  SizeOfHeapReserve;
  UINT64  SizeOfHeapCommit;
  UINT32  LoaderFlags;
  UINT32  NumberOfRvaAndSizes;
} PE32PLUS_OPTIONAL_HEADER;

///
/// COFF Section Header
///
typedef struct {
  UINT8   Name[8];                  ///< Section name
  UINT32  VirtualSize;              ///< Virtual size
  UINT32  VirtualAddress;           ///< Virtual address
  UINT32  SizeOfRawData;            ///< Size of raw data
  UINT32  PointerToRawData;         ///< File pointer to raw data
  UINT32  PointerToRelocations;     ///< File pointer to relocations
  UINT32  PointerToLinenumbers;     ///< File pointer to line numbers
  UINT16  NumberOfRelocations;      ///< Number of relocations
  UINT16  NumberOfLinenumbers;      ///< Number of line numbers
  UINT32  Characteristics;          ///< Section characteristics
} COFF_SECTION_HEADER;

///
/// COFF Relocation Entry
///
typedef struct {
  UINT32  VirtualAddress;       ///< Address of item to be relocated
  UINT32  SymbolTableIndex;     ///< Index into symbol table
  UINT16  Type;                 ///< Relocation type (machine-specific)
} COFF_RELOCATION;

///
/// COFF Symbol Table Entry
///
typedef struct {
  union {
    UINT8   ShortName[8];           ///< Symbol name (if <= 8 chars)
    struct {
      UINT32  Zeros;                ///< Zero if name > 8 chars
      UINT32  Offset;               ///< String table offset
    } LongName;
  } Name;
  UINT32  Value;                    ///< Symbol value
  INT16   SectionNumber;            ///< Section number
  UINT16  Type;                     ///< Symbol type
  UINT8   StorageClass;             ///< Storage class
  UINT8   NumberOfAuxSymbols;       ///< Number of auxiliary entries
} COFF_SYMBOL;

///
/// BigObj Header
///
typedef struct {
  UINT16  Sig1;                     ///< Must be 0xFFFF
  UINT16  Sig2;                     ///< Must be 0xFFFF
  UINT16  Version;                  ///< Version (2)
  UINT16  Machine;                  ///< Machine type
  UINT32  TimeDateStamp;            ///< Time/date stamp
  UINT8   ClassID[16];              ///< Class ID GUID
  UINT32  SizeOfData;               ///< Size of data after header
  UINT32  Flags;                    ///< Flags
  UINT32  MetaDataSize;             ///< Metadata size
  UINT32  MetaDataOffset;           ///< Metadata offset
  UINT32  NumberOfSections;         ///< Number of sections
  UINT32  PointerToSymbolTable;     ///< Symbol table offset
  UINT32  NumberOfSymbols;          ///< Number of symbols
} BIGOBJ_HEADER;

///
/// TE (Terse Executable) Header
///
typedef struct {
  UINT16  Signature;                ///< TE signature (0x5A56)
  UINT16  Machine;                  ///< Machine type
  UINT8   NumberOfSections;         ///< Number of sections
  UINT8   Subsystem;                ///< Subsystem
  UINT16  StrippedSize;             ///< Bytes stripped from header
  UINT32  AddressOfEntryPoint;      ///< Entry point RVA
  UINT32  BaseOfCode;               ///< Base of code
  UINT64  ImageBase;                ///< Image base address
  UINT32  DataDirectoryBaseReloc;   ///< Base relocation table
  UINT32  DataDirectoryDebug;       ///< Debug directory
} TE_HEADER;

///
/// XCOFF32 File Header
///
typedef struct {
  UINT16  Magic;                    ///< Magic number (0x01DF)
  UINT16  NumberOfSections;         ///< Number of sections
  UINT32  TimeDate;                 ///< Time and date
  UINT32  SymbolTablePointer;       ///< Symbol table pointer
  UINT32  NumberOfSymbols;          ///< Number of symbols
  UINT16  OptionalHeaderSize;       ///< Optional header size
  UINT16  Flags;                    ///< Flags
} XCOFF32_FILE_HEADER;

///
/// XCOFF64 File Header
///
typedef struct {
  UINT16  Magic;                    ///< Magic number (0x01F7)
  UINT16  NumberOfSections;         ///< Number of sections
  UINT32  TimeDate;                 ///< Time and date
  UINT64  SymbolTablePointer;       ///< Symbol table pointer (64-bit)
  UINT16  OptionalHeaderSize;       ///< Optional header size
  UINT16  Flags;                    ///< Flags
  UINT32  NumberOfSymbols;          ///< Number of symbols
} XCOFF64_FILE_HEADER;

///
/// ECOFF File Header
///
typedef struct {
  UINT16  Magic;                    ///< Magic number
  UINT16  NumberOfSections;         ///< Number of sections
  UINT32  TimeDate;                 ///< Time and date
  UINT32  SymbolTablePointer;       ///< Symbol table pointer
  UINT32  NumberOfSymbols;          ///< Number of symbols
  UINT16  OptionalHeaderSize;       ///< Optional header size
  UINT16  Flags;                    ///< Flags
} ECOFF_FILE_HEADER;

#pragma pack(pop)

///
/// COFF Variant Types
///
typedef enum {
  CoffVariantStandard,    ///< Standard COFF
  CoffVariantPE,          ///< PE/COFF (Windows)
  CoffVariantBigObj,      ///< BigObj COFF
  CoffVariantSVR3,        ///< SVR3 COFF
  CoffVariantXCOFF32,     ///< XCOFF 32-bit (AIX)
  CoffVariantXCOFF64,     ///< XCOFF 64-bit (AIX)
  CoffVariantECOFF32,     ///< ECOFF 32-bit (MIPS)
  CoffVariantECOFF64,     ///< ECOFF 64-bit (MIPS/Alpha)
  CoffVariantTE           ///< Terse Executable (UEFI)
} COFF_VARIANT;

///
/// COFF Context Structure
///
typedef struct _COFF_CONTEXT {
  BOOLEAN         ReadOnly;       ///< Read-only mode
  COFF_VARIANT    Variant;        ///< COFF variant
  BOOLEAN         Is64Bit;        ///< TRUE for 64-bit formats
  UINT8           *FileData;      ///< File data buffer
  UINT64          FileSize;       ///< File size
  BOOLEAN         OwnBuffer;      ///< TRUE if we allocated the buffer

  ///
  /// Headers
  ///
  union {
    COFF_FILE_HEADER      *Standard;
    BIGOBJ_HEADER         *BigObj;
    XCOFF32_FILE_HEADER   *XCoff32;
    XCOFF64_FILE_HEADER   *XCoff64;
    ECOFF_FILE_HEADER     *ECoff;
    TE_HEADER             *Te;
  } Header;

  ///
  /// Optional header (PE only)
  ///
  union {
    PE32_OPTIONAL_HEADER      *Pe32;
    PE32PLUS_OPTIONAL_HEADER  *Pe32Plus;
  } OptionalHeader;

  ///
  /// Sections
  ///
  COFF_SECTION_HEADER  *Sections;

  ///
  /// Symbols
  ///
  COFF_SYMBOL          *Symbols;
  CHAR8                *StringTable;
} COFF_CONTEXT;

#define COFF_CONTEXT_FROM_BINFORMAT(ctx) ((COFF_CONTEXT *)(ctx))

//
// COFF Relocation Iterator
//
typedef struct {
  COFF_CONTEXT  *CoffContext;
  UINT16        Machine;
  UINT32        SectionIndex;
  UINT32        CurrentIndex;
  UINT32        TotalCount;
  UINT64        RelocOffset;
  UINT8         *RelocData;
} COFF_RELOCATION_ITERATOR;

/**
  Convert COFF machine type to BINFORMAT_MACHINE.

  @param[in]  CoffMachine  COFF machine type.

  @return BINFORMAT_MACHINE type.

**/
STATIC
BINFORMAT_MACHINE
CoffMachineToGeneric (
  IN  UINT16  CoffMachine
  )
{
  switch (CoffMachine) {
    case COFF_MAGIC_I386:   return BinMachineX86;
    case COFF_MAGIC_AMD64:  return BinMachineX64;
    case COFF_MAGIC_ARM:
    case COFF_MAGIC_ARMNT:  return BinMachineARM;
    case COFF_MAGIC_ARM64:  return BinMachineARM64;
    case COFF_MAGIC_IA64:   return BinMachineIA64;
    default:                return BinMachineUnknown;
  }
}

/**
  Detect COFF variant from file data.

  @param[in]  Data   Pointer to file data.
  @param[in]  Size   Size of data.

  @return COFF variant type.

**/
STATIC
COFF_VARIANT
DetectCoffVariant (
  IN  UINT8   *Data,
  IN  UINT64  Size
  )
{
  if (Size < 4) {
    return CoffVariantStandard;
  }

  //
  // Check for TE signature
  //
  TE_HEADER *TeHdr = (TE_HEADER *)Data;
  if (TeHdr->Signature == TE_MAGIC) {
    return CoffVariantTE;
  }

  //
  // Check for PE signature (MZ + PE)
  //
  if (Size > 0x3C && Data[0] == 'M' && Data[1] == 'Z') {
    UINT32 PeOffset = *(UINT32 *)(Data + 0x3C);
    if (PeOffset + 4 <= Size) {
      UINT32 PeSig = *(UINT32 *)(Data + PeOffset);
      if (PeSig == PE_SIGNATURE) {
        return CoffVariantPE;
      }
    }
  }

  //
  // Check for BigObj
  //
  BIGOBJ_HEADER *BigObj = (BIGOBJ_HEADER *)Data;
  if (Size >= sizeof(BIGOBJ_HEADER) &&
      BigObj->Sig1 == BIGOBJ_MAGIC_ANON &&
      BigObj->Sig2 == BIGOBJ_MAGIC_ANON &&
      BigObj->Version == BIGOBJ_VERSION) {
    return CoffVariantBigObj;
  }

  //
  // Check for XCOFF
  //
  UINT16 Magic = *(UINT16 *)Data;
  if (Magic == XCOFF32_MAGIC) {
    return CoffVariantXCOFF32;
  }
  if (Magic == XCOFF64_MAGIC) {
    return CoffVariantXCOFF64;
  }

  //
  // Check for ECOFF
  //
  if (Magic == ECOFF_MAGIC_MIPS) {
    return CoffVariantECOFF32;
  }
  if (Magic == ECOFF_MAGIC_ALPHA) {
    return CoffVariantECOFF64;
  }

  //
  // Default to standard COFF
  //
  return CoffVariantStandard;
}

/**
  Initialize COFF context from file.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FilePath   Path to COFF file.
  @param[in]   ReadOnly   TRUE for read-only access.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
CoffInitFile (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  )
{
  FILE          *File;
  COFF_CONTEXT  *CoffCtx;
  UINT64        FileSize;

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

  CoffCtx = (COFF_CONTEXT *)calloc(1, sizeof(COFF_CONTEXT));
  if (CoffCtx == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  CoffCtx->FileData = (UINT8 *)malloc(FileSize);
  if (CoffCtx->FileData == NULL) {
    free(CoffCtx);
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  if (fread(CoffCtx->FileData, 1, FileSize, File) != FileSize) {
    free(CoffCtx->FileData);
    free(CoffCtx);
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);

  CoffCtx->FileSize = FileSize;
  CoffCtx->OwnBuffer = TRUE;
  CoffCtx->ReadOnly = ReadOnly;

  //
  // Detect variant
  //
  CoffCtx->Variant = DetectCoffVariant(CoffCtx->FileData, FileSize);

  //
  // Parse headers based on variant
  //
  switch (CoffCtx->Variant) {
    case CoffVariantPE: {
      UINT32 PeOffset = *(UINT32 *)(CoffCtx->FileData + 0x3C);
      CoffCtx->Header.Standard = (COFF_FILE_HEADER *)(CoffCtx->FileData + PeOffset + 4);

      if (CoffCtx->Header.Standard->SizeOfOptionalHeader > 0) {
        UINT8 *OptHdr = (UINT8 *)(CoffCtx->Header.Standard + 1);
        UINT16 Magic = *(UINT16 *)OptHdr;
        CoffCtx->Is64Bit = (Magic == 0x020B); // PE32+

        if (CoffCtx->Is64Bit) {
          CoffCtx->OptionalHeader.Pe32Plus = (PE32PLUS_OPTIONAL_HEADER *)OptHdr;
        } else {
          CoffCtx->OptionalHeader.Pe32 = (PE32_OPTIONAL_HEADER *)OptHdr;
        }

        CoffCtx->Sections = (COFF_SECTION_HEADER *)(OptHdr +
          CoffCtx->Header.Standard->SizeOfOptionalHeader);
      }
      break;
    }

    case CoffVariantBigObj:
      CoffCtx->Header.BigObj = (BIGOBJ_HEADER *)CoffCtx->FileData;
      CoffCtx->Sections = (COFF_SECTION_HEADER *)(CoffCtx->FileData + sizeof(BIGOBJ_HEADER));
      break;

    case CoffVariantTE:
      CoffCtx->Header.Te = (TE_HEADER *)CoffCtx->FileData;
      CoffCtx->Sections = (COFF_SECTION_HEADER *)(CoffCtx->FileData + sizeof(TE_HEADER));
      break;

    case CoffVariantXCOFF32:
      CoffCtx->Header.XCoff32 = (XCOFF32_FILE_HEADER *)CoffCtx->FileData;
      CoffCtx->Is64Bit = FALSE;
      break;

    case CoffVariantXCOFF64:
      CoffCtx->Header.XCoff64 = (XCOFF64_FILE_HEADER *)CoffCtx->FileData;
      CoffCtx->Is64Bit = TRUE;
      break;

    case CoffVariantECOFF32:
    case CoffVariantECOFF64:
      CoffCtx->Header.ECoff = (ECOFF_FILE_HEADER *)CoffCtx->FileData;
      CoffCtx->Is64Bit = (CoffCtx->Variant == CoffVariantECOFF64);
      break;

    default:
      CoffCtx->Header.Standard = (COFF_FILE_HEADER *)CoffCtx->FileData;
      CoffCtx->Sections = (COFF_SECTION_HEADER *)(CoffCtx->FileData +
        sizeof(COFF_FILE_HEADER) + CoffCtx->Header.Standard->SizeOfOptionalHeader);
      break;
  }

  *Context = (BINFORMAT_CONTEXT *)CoffCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Initialize COFF context from memory buffer.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   Buffer     Pointer to COFF data.
  @param[in]   Size       Size of COFF data.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
CoffInitMemory (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST VOID         *Buffer,
  IN  UINT64             Size
  )
{
  COFF_CONTEXT  *CoffCtx;

  if (Context == NULL || Buffer == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  CoffCtx = (COFF_CONTEXT *)calloc(1, sizeof(COFF_CONTEXT));
  if (CoffCtx == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  CoffCtx->FileData = (UINT8 *)Buffer;
  CoffCtx->FileSize = Size;
  CoffCtx->OwnBuffer = FALSE;
  CoffCtx->ReadOnly = TRUE;

  CoffCtx->Variant = DetectCoffVariant(CoffCtx->FileData, Size);

  //
  // Parse based on variant (same logic as InitFile)
  //
  switch (CoffCtx->Variant) {
    case CoffVariantPE: {
      UINT32 PeOffset = *(UINT32 *)(CoffCtx->FileData + 0x3C);
      CoffCtx->Header.Standard = (COFF_FILE_HEADER *)(CoffCtx->FileData + PeOffset + 4);
      break;
    }
    case CoffVariantTE:
      CoffCtx->Header.Te = (TE_HEADER *)CoffCtx->FileData;
      break;
    default:
      CoffCtx->Header.Standard = (COFF_FILE_HEADER *)CoffCtx->FileData;
      break;
  }

  *Context = (BINFORMAT_CONTEXT *)CoffCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Create a new COFF context for writing.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FileType   Type of binary to create.
  @param[in]   Machine    Target machine.
  @param[in]   Is64Bit    TRUE for 64-bit format.

  @retval BINFORMAT_SUCCESS       Context created successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
CoffCreate (
  OUT BINFORMAT_CONTEXT   **Context,
  IN  BINFORMAT_FILE_TYPE FileType,
  IN  BINFORMAT_MACHINE   Machine,
  IN  BOOLEAN             Is64Bit
  )
{
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Close and free a COFF context.

  @param[in]  Context    Context to close.

**/
STATIC
VOID
CoffClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  COFF_CONTEXT  *CoffCtx;

  if (Context == NULL) {
    return;
  }

  CoffCtx = COFF_CONTEXT_FROM_BINFORMAT(Context);

  if (CoffCtx->OwnBuffer && CoffCtx->FileData != NULL) {
    free(CoffCtx->FileData);
  }

  free(CoffCtx);
}

/**
  Get COFF header information.

  @param[in]   Context      COFF context.
  @param[out]  HeaderInfo   Pointer to receive header information.

  @retval BINFORMAT_SUCCESS       Header information retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
CoffGetHeader (
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  )
{
  COFF_CONTEXT  *CoffCtx;

  if (Context == NULL || HeaderInfo == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  CoffCtx = COFF_CONTEXT_FROM_BINFORMAT(Context);
  memset(HeaderInfo, 0, sizeof(BINFORMAT_HEADER_INFO));

  HeaderInfo->Is64Bit = CoffCtx->Is64Bit;

  switch (CoffCtx->Variant) {
    case CoffVariantPE:
      HeaderInfo->Machine = CoffMachineToGeneric(CoffCtx->Header.Standard->Machine);
      HeaderInfo->SectionCount = CoffCtx->Header.Standard->NumberOfSections;
      HeaderInfo->SymbolCount = CoffCtx->Header.Standard->NumberOfSymbols;
      HeaderInfo->Flags = CoffCtx->Header.Standard->Characteristics;

      if (CoffCtx->Header.Standard->Characteristics & IMAGE_FILE_DLL) {
        HeaderInfo->FileType = BinFileTypeSharedLibrary;
      } else if (CoffCtx->Header.Standard->Characteristics & IMAGE_FILE_EXECUTABLE_IMAGE) {
        HeaderInfo->FileType = BinFileTypeExecutable;
      } else {
        HeaderInfo->FileType = BinFileTypeRelocatable;
      }

      if (CoffCtx->Is64Bit && CoffCtx->OptionalHeader.Pe32Plus != NULL) {
        HeaderInfo->EntryPoint = CoffCtx->OptionalHeader.Pe32Plus->AddressOfEntryPoint;
      } else if (!CoffCtx->Is64Bit && CoffCtx->OptionalHeader.Pe32 != NULL) {
        HeaderInfo->EntryPoint = CoffCtx->OptionalHeader.Pe32->AddressOfEntryPoint;
      }
      break;

    case CoffVariantTE:
      HeaderInfo->Machine = CoffMachineToGeneric(CoffCtx->Header.Te->Machine);
      HeaderInfo->SectionCount = CoffCtx->Header.Te->NumberOfSections;
      HeaderInfo->EntryPoint = CoffCtx->Header.Te->AddressOfEntryPoint;
      HeaderInfo->FileType = BinFileTypeExecutable;
      break;

    default:
      if (CoffCtx->Header.Standard != NULL) {
        HeaderInfo->Machine = CoffMachineToGeneric(CoffCtx->Header.Standard->Machine);
        HeaderInfo->SectionCount = CoffCtx->Header.Standard->NumberOfSections;
      }
      break;
  }

  return BINFORMAT_SUCCESS;
}

//
// Relocation Type Mapping
//

/**
  Map COFF relocation type to universal BINFORMAT_RELOC_TYPE.

  @param[in]  Machine     Machine type
  @param[in]  RelocType   COFF relocation type

  @return Universal relocation type
**/
STATIC
BINFORMAT_RELOC_TYPE
CoffMapRelocType (
  IN  UINT16  Machine,
  IN  UINT16  RelocType
  )
{
  switch (Machine) {
    case COFF_MAGIC_AMD64:
      switch (RelocType) {
        case IMAGE_REL_AMD64_ABSOLUTE:
          return BinRelocNone;
        case IMAGE_REL_AMD64_ADDR64:
          return BinRelocAbsolute64;
        case IMAGE_REL_AMD64_ADDR32:
        case IMAGE_REL_AMD64_ADDR32NB:
          return BinRelocAbsolute32;
        case IMAGE_REL_AMD64_REL32:
        case IMAGE_REL_AMD64_REL32_1:
        case IMAGE_REL_AMD64_REL32_2:
        case IMAGE_REL_AMD64_REL32_3:
        case IMAGE_REL_AMD64_REL32_4:
        case IMAGE_REL_AMD64_REL32_5:
          return BinRelocPCRelative32;
        case IMAGE_REL_AMD64_SECREL:
          return BinRelocRelative32;
        default:
          return BinRelocFormatSpecific;
      }

    case COFF_MAGIC_I386:
      switch (RelocType) {
        case IMAGE_REL_I386_ABSOLUTE:
          return BinRelocNone;
        case IMAGE_REL_I386_DIR16:
          return BinRelocAbsolute16;
        case IMAGE_REL_I386_DIR32:
        case IMAGE_REL_I386_DIR32NB:
          return BinRelocAbsolute32;
        case IMAGE_REL_I386_REL16:
          return BinRelocPCRelative16;
        case IMAGE_REL_I386_REL32:
          return BinRelocPCRelative32;
        case IMAGE_REL_I386_SECREL:
          return BinRelocRelative32;
        default:
          return BinRelocFormatSpecific;
      }

    case COFF_MAGIC_ARM:
    case COFF_MAGIC_ARMNT:
      switch (RelocType) {
        case IMAGE_REL_ARM_ABSOLUTE:
          return BinRelocNone;
        case IMAGE_REL_ARM_ADDR32:
        case IMAGE_REL_ARM_ADDR32NB:
          return BinRelocAbsolute32;
        case IMAGE_REL_ARM_BRANCH24:
        case IMAGE_REL_ARM_BRANCH11:
        case IMAGE_REL_THUMB_BRANCH20:
        case IMAGE_REL_THUMB_BRANCH24:
        case IMAGE_REL_THUMB_BLX23:
          return BinRelocPCRelative32;
        case IMAGE_REL_ARM_SECREL:
          return BinRelocRelative32;
        default:
          return BinRelocFormatSpecific;
      }

    case COFF_MAGIC_ARM64:
      switch (RelocType) {
        case IMAGE_REL_ARM64_ABSOLUTE:
          return BinRelocNone;
        case IMAGE_REL_ARM64_ADDR32:
        case IMAGE_REL_ARM64_ADDR32NB:
          return BinRelocAbsolute32;
        case IMAGE_REL_ARM64_ADDR64:
          return BinRelocAbsolute64;
        case IMAGE_REL_ARM64_BRANCH26:
        case IMAGE_REL_ARM64_PAGEBASE_REL21:
        case IMAGE_REL_ARM64_REL21:
          return BinRelocPCRelative32;
        case IMAGE_REL_ARM64_SECREL:
          return BinRelocRelative32;
        default:
          return BinRelocFormatSpecific;
      }

    default:
      return BinRelocFormatSpecific;
  }
}

//
// COFF Relocation Iterator Functions
//

/**
  Create relocation iterator for a COFF section.

  @param[in]  Context       Binary format context
  @param[in]  SectionIndex  Section index
  @param[out] Iterator      Pointer to receive iterator

  @retval BINFORMAT_SUCCESS         Iterator created
  @retval BINFORMAT_ERROR_*         Error occurred
**/
STATIC
BINFORMAT_STATUS
CoffRelocationIterCreate (
  IN  BINFORMAT_CONTEXT               *Context,
  IN  UINT32                          SectionIndex,
  OUT BINFORMAT_RELOCATION_ITERATOR   **Iterator
  )
{
  COFF_CONTEXT              *CoffCtx;
  COFF_RELOCATION_ITERATOR  *Iter;
  COFF_SECTION_HEADER       *Section;
  UINT16                    Machine;
  UINT32                    NumSections;

  if (Context == NULL || Iterator == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  *Iterator = NULL;
  CoffCtx = COFF_CONTEXT_FROM_BINFORMAT(Context);

  //
  // Get machine type and section count
  //
  if (CoffCtx->Header.Standard != NULL) {
    Machine = CoffCtx->Header.Standard->Machine;
    NumSections = CoffCtx->Header.Standard->NumberOfSections;
  } else {
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  //
  // Validate section index
  //
  if (SectionIndex >= NumSections) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  //
  // Get section header
  //
  Section = &CoffCtx->Sections[SectionIndex];

  //
  // If no relocations, return success with NULL iterator
  //
  if (Section->NumberOfRelocations == 0) {
    return BINFORMAT_SUCCESS;
  }

  //
  // Allocate iterator
  //
  Iter = (COFF_RELOCATION_ITERATOR *)malloc(sizeof(COFF_RELOCATION_ITERATOR));
  if (Iter == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Initialize iterator
  //
  Iter->CoffContext = CoffCtx;
  Iter->Machine = Machine;
  Iter->SectionIndex = SectionIndex;
  Iter->CurrentIndex = 0;
  Iter->TotalCount = Section->NumberOfRelocations;
  Iter->RelocOffset = Section->PointerToRelocations;
  Iter->RelocData = CoffCtx->FileData + Iter->RelocOffset;

  *Iterator = (BINFORMAT_RELOCATION_ITERATOR *)Iter;
  return BINFORMAT_SUCCESS;
}

/**
  Get next relocation from iterator.

  @param[in]  Iterator    Relocation iterator
  @param[out] Relocation  Pointer to receive relocation info

  @retval BINFORMAT_SUCCESS         Relocation retrieved
  @retval BINFORMAT_ERROR_NOT_FOUND No more relocations
  @retval BINFORMAT_ERROR_*         Error occurred
**/
STATIC
BINFORMAT_STATUS
CoffRelocationIterNext (
  IN  BINFORMAT_RELOCATION_ITERATOR  *Iterator,
  OUT BINFORMAT_RELOCATION           *Relocation
  )
{
  COFF_RELOCATION_ITERATOR  *Iter;
  COFF_RELOCATION           *CoffReloc;

  if (Iterator == NULL || Relocation == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  Iter = (COFF_RELOCATION_ITERATOR *)Iterator;

  if (Iter->CurrentIndex >= Iter->TotalCount) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  //
  // Read relocation entry from current position
  //
  CoffReloc = (COFF_RELOCATION *)(Iter->RelocData +
                                   (Iter->CurrentIndex * sizeof(COFF_RELOCATION)));

  //
  // Fill in relocation information
  //
  memset(Relocation, 0, sizeof(BINFORMAT_RELOCATION));

  Relocation->Offset = CoffReloc->VirtualAddress;
  Relocation->Type = CoffMapRelocType(Iter->Machine, CoffReloc->Type);
  Relocation->SymbolIndex = CoffReloc->SymbolTableIndex;
  Relocation->Addend = 0;  // COFF doesn't use addends
  Relocation->NativeType = CoffReloc->Type;
  Relocation->SectionIndex = Iter->SectionIndex;
  Relocation->IsScattered = FALSE;
  Relocation->IsExtern = (CoffReloc->SymbolTableIndex != 0);
  Relocation->IsPcRel = (Relocation->Type >= BinRelocPCRelative8 &&
                         Relocation->Type <= BinRelocPCRelative64);
  Relocation->Length = 4;  // Default to 4 bytes

  //
  // Adjust length based on relocation type
  //
  if (Relocation->Type == BinRelocAbsolute64 || Relocation->Type == BinRelocPCRelative64) {
    Relocation->Length = 8;
  } else if (Relocation->Type == BinRelocAbsolute16 || Relocation->Type == BinRelocPCRelative16) {
    Relocation->Length = 2;
  } else if (Relocation->Type == BinRelocAbsolute8 || Relocation->Type == BinRelocPCRelative8) {
    Relocation->Length = 1;
  }

  Iter->CurrentIndex++;
  return BINFORMAT_SUCCESS;
}

/**
  Free relocation iterator.

  @param[in]  Iterator    Relocation iterator to free
**/
STATIC
VOID
CoffRelocationIterFree (
  IN  BINFORMAT_RELOCATION_ITERATOR  *Iterator
  )
{
  if (Iterator != NULL) {
    free(Iterator);
  }
}

//
// Stub implementations
//
STATIC BINFORMAT_STATUS CoffGetSection(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffGetSectionByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffGetSegment(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SEGMENT *Seg) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffGetSymbol(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffGetSymbolByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffGetRelocations(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, OUT BINFORMAT_RELOCATION **Rels, OUT UINT32 *Cnt) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffAddSection(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SECTION *Sec, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffAddSymbol(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SYMBOL *Sym, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffAddRelocation(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, IN BINFORMAT_RELOCATION *Rel) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffWriteFile(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Path) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffWriteMemory(IN BINFORMAT_CONTEXT *Ctx, OUT VOID *Buf, IN UINT64 Size, OUT UINT64 *Written) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS CoffSelectArchitecture(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 ArchIdx) { return BINFORMAT_ERROR_UNSUPPORTED; }

//
// COFF API Table
//
STATIC CONST BINFORMAT_API gCoffApi = {
  .LibraryName = "libcoff",
  .Version = 1,
  .InitFile = CoffInitFile,
  .InitMemory = CoffInitMemory,
  .Create = CoffCreate,
  .Close = CoffClose,
  .GetHeader = CoffGetHeader,
  .GetSection = CoffGetSection,
  .GetSectionByName = CoffGetSectionByName,
  .GetSegment = CoffGetSegment,
  .GetSymbol = CoffGetSymbol,
  .GetSymbolByName = CoffGetSymbolByName,
  .AddSection = CoffAddSection,
  .AddSymbol = CoffAddSymbol,
  .AddRelocation = CoffAddRelocation,
  .WriteFile = CoffWriteFile,
  .WriteMemory = CoffWriteMemory,
  .SelectArchitecture = CoffSelectArchitecture,
  .SectionIterCreate = NULL,
  .SectionIterNext = NULL,
  .SectionIterFree = NULL,
  .SymbolIterCreate = NULL,
  .SymbolIterNext = NULL,
  .SymbolIterFree = NULL,
  .SegmentIterCreate = NULL,
  .SegmentIterNext = NULL,
  .SegmentIterFree = NULL,
  .RelocationIterCreate = CoffRelocationIterCreate,
  .RelocationIterNext = CoffRelocationIterNext,
  .RelocationIterFree = CoffRelocationIterFree,
  .ArchIterCreate = NULL,
  .ArchIterNext = NULL,
  .ArchIterFree = NULL
};

/**
  Get the COFF library API table.

  @return Pointer to COFF library API table.

**/
CONST BINFORMAT_API *
CoffGetApi (
  VOID
  )
{
  return &gCoffApi;
}
