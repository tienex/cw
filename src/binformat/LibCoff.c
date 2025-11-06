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
  .GetRelocations = CoffGetRelocations,
  .AddSection = CoffAddSection,
  .AddSymbol = CoffAddSymbol,
  .AddRelocation = CoffAddRelocation,
  .WriteFile = CoffWriteFile,
  .WriteMemory = CoffWriteMemory,
  .SelectArchitecture = CoffSelectArchitecture
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
