/** @file
  OMF Binary Format Library Implementation.

  This file implements support for OMF, MZ, NE, LE, LX, and Xenix x.out binary
  formats following the unified binary format API.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/binformat/LibOmf.h"

#pragma pack(push, 1)

///
/// MZ (DOS) Header
///
typedef struct {
  UINT16  Magic;              ///< MZ signature
  UINT16  BytesInLastBlock;   ///< Bytes in last block
  UINT16  BlocksInFile;       ///< Blocks in file
  UINT16  NumRelocations;     ///< Number of relocations
  UINT16  HeaderParagraphs;   ///< Header size in paragraphs
  UINT16  MinExtraParagraphs; ///< Min extra paragraphs
  UINT16  MaxExtraParagraphs; ///< Max extra paragraphs
  UINT16  Ss;                 ///< Initial SS
  UINT16  Sp;                 ///< Initial SP
  UINT16  Checksum;           ///< Checksum
  UINT16  Ip;                 ///< Initial IP
  UINT16  Cs;                 ///< Initial CS
  UINT16  RelocTableOffset;   ///< Relocation table offset
  UINT16  OverlayNumber;      ///< Overlay number
  UINT16  Reserved[4];        ///< Reserved
  UINT16  OemId;              ///< OEM identifier
  UINT16  OemInfo;            ///< OEM information
  UINT16  Reserved2[10];      ///< Reserved
  UINT32  NewHeaderOffset;    ///< Offset to NE/LE/LX/PE header
} MZ_HEADER;

///
/// NE (New Executable) Header
///
typedef struct {
  UINT16  Magic;              ///< NE signature
  UINT8   LinkerVersion;      ///< Linker version
  UINT8   LinkerRevision;     ///< Linker revision
  UINT16  EntryTableOffset;   ///< Entry table offset
  UINT16  EntryTableSize;     ///< Entry table size
  UINT32  Checksum;           ///< Checksum
  UINT16  Flags;              ///< Flags
  UINT16  AutoDataSegment;    ///< Automatic data segment
  UINT16  InitialHeapSize;    ///< Initial heap size
  UINT16  InitialStackSize;   ///< Initial stack size
  UINT32  EntryPoint;         ///< Entry point (CS:IP)
  UINT32  InitialStack;       ///< Initial stack (SS:SP)
  UINT16  SegmentCount;       ///< Number of segments
  UINT16  ModuleRefs;         ///< Module reference count
  UINT16  NonResidentSize;    ///< Non-resident names table size
  UINT16  SegmentTableOffset; ///< Segment table offset
  UINT16  ResourceTableOffset; ///< Resource table offset
  UINT16  ResidentTableOffset; ///< Resident names table offset
  UINT16  ModuleRefTableOffset; ///< Module reference table offset
  UINT16  ImportTableOffset;  ///< Import names table offset
  UINT32  NonResidentOffset;  ///< Non-resident names table offset
  UINT16  MovableEntries;     ///< Movable entry points
  UINT16  AlignShift;         ///< Logical sector alignment shift
  UINT16  ResourceSegments;   ///< Resource segments
  UINT8   TargetOS;           ///< Target operating system
  UINT8   AdditionalFlags;    ///< Additional flags
  UINT16  FastLoadOffset;     ///< Fast-load area offset
  UINT16  FastLoadSize;       ///< Fast-load area size
  UINT16  Reserved;           ///< Reserved
  UINT16  ExpectedVersion;    ///< Expected Windows version
} NE_HEADER;

///
/// LE/LX (Linear Executable) Header
///
typedef struct {
  UINT16  Magic;              ///< LE or LX signature
  UINT8   ByteOrder;          ///< Byte order
  UINT8   WordOrder;          ///< Word order
  UINT32  FormatLevel;        ///< Format level
  UINT16  CpuType;            ///< CPU type
  UINT16  TargetOS;           ///< Target OS
  UINT32  ModuleVersion;      ///< Module version
  UINT32  ModuleFlags;        ///< Module flags
  UINT32  ModulePages;        ///< Module page count
  UINT32  EipObject;          ///< EIP object number
  UINT32  Eip;                ///< EIP value
  UINT32  EspObject;          ///< ESP object number
  UINT32  Esp;                ///< ESP value
  UINT32  PageSize;           ///< Page size
  UINT32  PageOffsetShift;    ///< Page offset shift (LX only)
  UINT32  FixupSectionSize;   ///< Fixup section size
  UINT32  FixupSectionChecksum; ///< Fixup section checksum
  UINT32  LoaderSectionSize;  ///< Loader section size
  UINT32  LoaderSectionChecksum; ///< Loader section checksum
  UINT32  ObjectTableOffset;  ///< Object table offset
  UINT32  ObjectCount;        ///< Object table entry count
  UINT32  ObjectPageTableOffset; ///< Object page table offset
  UINT32  ObjectIterPagesOffset; ///< Object iterated pages offset
  UINT32  ResourceTableOffset; ///< Resource table offset
  UINT32  ResourceTableEntries; ///< Resource table entries
  UINT32  ResidentTableOffset; ///< Resident names table offset
  UINT32  EntryTableOffset;   ///< Entry table offset
  UINT32  ModuleDirectivesOffset; ///< Module directives offset
  UINT32  ModuleDirectivesCount; ///< Module directives count
  UINT32  FixupPageTableOffset; ///< Fixup page table offset
  UINT32  FixupRecordTableOffset; ///< Fixup record table offset
  UINT32  ImportModuleTableOffset; ///< Import module table offset
  UINT32  ImportModuleEntries; ///< Import module entries
  UINT32  ImportProcTableOffset; ///< Import procedure table offset
  UINT32  PerPageChecksumOffset; ///< Per-page checksum offset
  UINT32  DataPagesOffset;    ///< Data pages offset
  UINT32  PreloadPageCount;   ///< Preload page count
  UINT32  NonResidentTableOffset; ///< Non-resident names table offset
  UINT32  NonResidentTableSize; ///< Non-resident names table size
  UINT32  NonResidentChecksum; ///< Non-resident names checksum
  UINT32  AutoDataObject;     ///< Auto data object
  UINT32  DebugInfoOffset;    ///< Debug info offset
  UINT32  DebugInfoSize;      ///< Debug info size
  UINT32  PreloadInstancePages; ///< Preload instance pages
  UINT32  DemandInstancePages; ///< Demand instance pages
  UINT32  HeapSize;           ///< Heap size
} LE_HEADER;

#pragma pack(pop)

typedef enum {
  OmfVariantOMF,
  OmfVariantMZ,
  OmfVariantNE,
  OmfVariantLE,
  OmfVariantLX,
  OmfVariantXout
} OMF_VARIANT;

typedef struct _OMF_CONTEXT {
  BOOLEAN      ReadOnly;
  OMF_VARIANT  Variant;
  UINT8        *FileData;
  UINT64       FileSize;
  BOOLEAN      OwnBuffer;

  union {
    MZ_HEADER  *Mz;
    NE_HEADER  *Ne;
    LE_HEADER  *Le;
  } Header;
} OMF_CONTEXT;

#define OMF_CONTEXT_FROM_BINFORMAT(ctx) ((OMF_CONTEXT *)(ctx))

STATIC
BINFORMAT_STATUS
OmfInitFile (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  )
{
  FILE         *File;
  OMF_CONTEXT  *OmfCtx;
  UINT64       FileSize;

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

  OmfCtx = (OMF_CONTEXT *)calloc(1, sizeof(OMF_CONTEXT));
  if (OmfCtx == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  OmfCtx->FileData = (UINT8 *)malloc(FileSize);
  if (OmfCtx->FileData == NULL) {
    free(OmfCtx);
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  if (fread(OmfCtx->FileData, 1, FileSize, File) != FileSize) {
    free(OmfCtx->FileData);
    free(OmfCtx);
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);

  OmfCtx->FileSize = FileSize;
  OmfCtx->OwnBuffer = TRUE;
  OmfCtx->ReadOnly = ReadOnly;

  //
  // Detect variant
  //
  UINT16 Magic = *(UINT16 *)OmfCtx->FileData;

  if (Magic == MZ_MAGIC || Magic == ZM_MAGIC) {
    MZ_HEADER *MzHdr = (MZ_HEADER *)OmfCtx->FileData;
    OmfCtx->Header.Mz = MzHdr;

    if (MzHdr->NewHeaderOffset != 0 && MzHdr->NewHeaderOffset < FileSize - 2) {
      UINT16 NewMagic = *(UINT16 *)(OmfCtx->FileData + MzHdr->NewHeaderOffset);

      if (NewMagic == NE_MAGIC) {
        OmfCtx->Variant = OmfVariantNE;
        OmfCtx->Header.Ne = (NE_HEADER *)(OmfCtx->FileData + MzHdr->NewHeaderOffset);
      } else if (NewMagic == LE_MAGIC) {
        OmfCtx->Variant = OmfVariantLE;
        OmfCtx->Header.Le = (LE_HEADER *)(OmfCtx->FileData + MzHdr->NewHeaderOffset);
      } else if (NewMagic == LX_MAGIC) {
        OmfCtx->Variant = OmfVariantLX;
        OmfCtx->Header.Le = (LE_HEADER *)(OmfCtx->FileData + MzHdr->NewHeaderOffset);
      } else {
        OmfCtx->Variant = OmfVariantMZ;
      }
    } else {
      OmfCtx->Variant = OmfVariantMZ;
    }
  } else if (Magic == XOUT_MAGIC) {
    OmfCtx->Variant = OmfVariantXout;
  } else if (OmfCtx->FileData[0] >= 0x80 && OmfCtx->FileData[0] <= 0xA2) {
    //
    // Likely OMF record
    //
    OmfCtx->Variant = OmfVariantOMF;
  } else {
    free(OmfCtx->FileData);
    free(OmfCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  *Context = (BINFORMAT_CONTEXT *)OmfCtx;
  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS OmfInitMemory(OUT BINFORMAT_CONTEXT **Ctx, IN CONST VOID *Buf, IN UINT64 Size) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfCreate(OUT BINFORMAT_CONTEXT **Ctx, IN BINFORMAT_FILE_TYPE Type, IN BINFORMAT_MACHINE Mach, IN BOOLEAN Is64) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }

STATIC
VOID
OmfClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  OMF_CONTEXT  *OmfCtx;

  if (Context == NULL) {
    return;
  }

  OmfCtx = OMF_CONTEXT_FROM_BINFORMAT(Context);

  if (OmfCtx->OwnBuffer && OmfCtx->FileData != NULL) {
    free(OmfCtx->FileData);
  }

  free(OmfCtx);
}

STATIC
BINFORMAT_STATUS
OmfGetHeader (
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  )
{
  OMF_CONTEXT  *OmfCtx;

  if (Context == NULL || HeaderInfo == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  OmfCtx = OMF_CONTEXT_FROM_BINFORMAT(Context);
  memset(HeaderInfo, 0, sizeof(BINFORMAT_HEADER_INFO));

  HeaderInfo->Is64Bit = FALSE;
  HeaderInfo->FileType = BinFileTypeExecutable;
  HeaderInfo->Machine = BinMachineX86;
  HeaderInfo->Endianness = BinEndianLittle;

  switch (OmfCtx->Variant) {
    case OmfVariantMZ:
      if (OmfCtx->Header.Mz != NULL) {
        HeaderInfo->EntryPoint = ((UINT32)OmfCtx->Header.Mz->Cs << 4) + OmfCtx->Header.Mz->Ip;
      }
      break;

    case OmfVariantNE:
      if (OmfCtx->Header.Ne != NULL) {
        HeaderInfo->SectionCount = OmfCtx->Header.Ne->SegmentCount;
      }
      break;

    case OmfVariantLE:
    case OmfVariantLX:
      if (OmfCtx->Header.Le != NULL) {
        HeaderInfo->EntryPoint = OmfCtx->Header.Le->Eip;
      }
      break;

    default:
      break;
  }

  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS OmfGetSection(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfGetSectionByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfGetSegment(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SEGMENT *Seg) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfGetSymbol(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfGetSymbolByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfGetRelocations(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, OUT BINFORMAT_RELOCATION **Rels, OUT UINT32 *Cnt) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfAddSection(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SECTION *Sec, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfAddSymbol(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SYMBOL *Sym, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfAddRelocation(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, IN BINFORMAT_RELOCATION *Rel) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfWriteFile(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Path) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfWriteMemory(IN BINFORMAT_CONTEXT *Ctx, OUT VOID *Buf, IN UINT64 Size, OUT UINT64 *Written) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OmfSelectArchitecture(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 ArchIdx) { return BINFORMAT_ERROR_UNSUPPORTED; }

STATIC CONST BINFORMAT_API gOmfApi = {
  .LibraryName = "libomf",
  .Version = 1,
  .InitFile = OmfInitFile,
  .InitMemory = OmfInitMemory,
  .Create = OmfCreate,
  .Close = OmfClose,
  .GetHeader = OmfGetHeader,
  .GetSection = OmfGetSection,
  .GetSectionByName = OmfGetSectionByName,
  .GetSegment = OmfGetSegment,
  .GetSymbol = OmfGetSymbol,
  .GetSymbolByName = OmfGetSymbolByName,
  .GetRelocations = OmfGetRelocations,
  .AddSection = OmfAddSection,
  .AddSymbol = OmfAddSymbol,
  .AddRelocation = OmfAddRelocation,
  .WriteFile = OmfWriteFile,
  .WriteMemory = OmfWriteMemory,
  .SelectArchitecture = OmfSelectArchitecture
};

CONST BINFORMAT_API *
OmfGetApi (
  VOID
  )
{
  return &gOmfApi;
}
