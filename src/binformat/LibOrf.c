/** @file
  Obscure and Rare Binary Format Library Implementation.

  This file implements support for obscure and rare binary formats including
  PEF, Amiga HUNK, Atari TOS, VMS, PDP-10, HP SOM, Acorn, and EPOC32/Symbian
  formats following the unified binary format API.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LibOrf.h"

#pragma pack(push, 1)

///
/// PEF Container Header
///
typedef struct {
  UINT32  Magic1;               ///< "Joy!" magic
  UINT32  Magic2;               ///< "peff" or CFM magic
  UINT32  Architecture;         ///< Architecture type
  UINT32  FormatVersion;        ///< Format version
  UINT32  DateTimeStamp;        ///< Date/time stamp
  UINT32  OldDefVersion;        ///< Old definition version
  UINT32  OldImpVersion;        ///< Old implementation version
  UINT32  CurrentVersion;       ///< Current version
  UINT16  SectionCount;         ///< Number of sections
  UINT16  InstSectionCount;     ///< Instantiated section count
  UINT32  Reserved;             ///< Reserved
} PEF_CONTAINER_HEADER;

///
/// Amiga HUNK Header
///
typedef struct {
  UINT32  HunkType;             ///< Hunk type
  UINT32  NumHunks;             ///< Number of hunks
  UINT32  FirstHunk;            ///< First hunk number
  UINT32  LastHunk;             ///< Last hunk number
} AMIGA_HUNK_HEADER;

///
/// Atari TOS Program Header
///
typedef struct {
  UINT16  Magic;                ///< Magic number (0x601A)
  UINT32  TextSize;             ///< Text segment size
  UINT32  DataSize;             ///< Data segment size
  UINT32  BssSize;              ///< BSS size
  UINT32  SymbolSize;           ///< Symbol table size
  UINT32  Reserved1;            ///< Reserved
  UINT32  Flags;                ///< Flags
  UINT16  RelocFlag;            ///< Relocation flag
} ATARI_TOS_HEADER;

///
/// OpenVMS Object Module Header
///
typedef struct {
  UINT16  RecordType;           ///< Record type
  UINT16  RecordSize;           ///< Record size
  UINT8   MajorId;              ///< Major structure ID
  UINT8   MinorId;              ///< Minor structure ID
} VMS_OBJECT_HEADER;

///
/// PDP-10 Save File Header
///
typedef struct {
  UINT8   Magic;                ///< Magic number (0137)
  UINT8   Flags;                ///< Flags
  UINT16  EntryPoint;           ///< Entry point address
  UINT16  HighSegStart;         ///< High segment start
  UINT16  HighSegLength;        ///< High segment length
  UINT16  LowSegStart;          ///< Low segment start (optional)
  UINT16  LowSegLength;         ///< Low segment length (optional)
} PDP10_SAV_HEADER;

///
/// HP SOM Header
///
typedef struct {
  UINT16  SystemId;             ///< System ID
  UINT16  Magic;                ///< Magic number
  UINT32  Version;              ///< Version ID
  UINT32  FileTime;             ///< File timestamp
  UINT32  EntrySpace;           ///< Entry space
  UINT32  EntrySubspace;        ///< Entry subspace
  UINT32  EntryOffset;          ///< Entry offset
  UINT32  AuxHeaderLocation;    ///< Auxiliary header location
  UINT32  AuxHeaderSize;        ///< Auxiliary header size
  UINT32  SomLength;            ///< SOM length
  UINT32  CompilerVersion;      ///< Compiler version
  UINT32  CompilerRevision;     ///< Compiler revision
  UINT32  CpuType;              ///< CPU type
} HP_SOM_HEADER;

///
/// Acorn AIF (ARM Image Format) Header
///
typedef struct {
  UINT32  BranchInstruction;    ///< Branch instruction (0xE1A00000)
  UINT32  SWIExit;              ///< SWI exit instruction
  UINT32  ReadOnlySize;         ///< Read-only size
  UINT32  ReadWriteSize;        ///< Read-write size
  UINT32  DebugSize;            ///< Debug size
  UINT32  ZeroInitSize;         ///< Zero-initialized size
  UINT32  DebugType;            ///< Debug type
  UINT32  ImageBase;            ///< Image base address
  UINT32  WorkSpace;            ///< Work space
  UINT32  AddressMode;          ///< Addressing mode
  UINT32  DataBase;             ///< Data base
  UINT32  Reserved[2];          ///< Reserved
} ACORN_AIF_HEADER;

///
/// EPOC32 E32 Image Header
///
typedef struct {
  UINT32  Uid1;                 ///< UID 1 (0x10000079)
  UINT32  Uid2;                 ///< UID 2 (0x1000007A for E32)
  UINT32  Uid3;                 ///< UID 3 (application specific)
  UINT32  Check;                ///< UID checksum
  UINT8   Signature[4];         ///< Signature "EPOC"
  UINT32  CpuType;              ///< CPU type
  UINT32  CodeChecksum;         ///< Code checksum
  UINT32  DataChecksum;         ///< Data checksum
  UINT8   MajorVersion;         ///< Major version
  UINT8   MinorVersion;         ///< Minor version
  UINT16  Build;                ///< Build number
  UINT32  Timestamp;            ///< Timestamp (seconds since 2000)
  UINT32  Flags;                ///< Flags
  UINT32  CodeSize;             ///< Code size
  UINT32  DataSize;             ///< Data size
  UINT32  HeapSizeMin;          ///< Minimum heap size
  UINT32  HeapSizeMax;          ///< Maximum heap size
  UINT32  StackSize;            ///< Stack size
  UINT32  BssSize;              ///< BSS size
  UINT32  EntryPoint;           ///< Entry point offset
  UINT32  CodeBase;             ///< Code base address
  UINT32  DataBase;             ///< Data base address
  UINT32  DllRefTableCount;     ///< DLL reference table count
  UINT32  ExportDirOffset;      ///< Export directory offset
  UINT32  ExportDirCount;       ///< Export directory count
  UINT32  TextSize;             ///< Text size
  UINT32  CodeOffset;           ///< Code offset
  UINT32  DataOffset;           ///< Data offset
  UINT32  ImportOffset;         ///< Import offset
  UINT32  CodeRelocOffset;      ///< Code relocation offset
  UINT32  DataRelocOffset;      ///< Data relocation offset
} EPOC_E32_HEADER;

#pragma pack(pop)

///
/// ORF Variant Types
///
typedef enum {
  OrfVariantPEF,          ///< Classic Mac PEF
  OrfVariantMacCode,      ///< Classic Mac CODE resource
  OrfVariantAmigaHunk,    ///< Amiga HUNK
  OrfVariantAtariTOS,     ///< Atari TOS PRG
  OrfVariantVMS,          ///< OpenVMS executable/object
  OrfVariantPDP10,        ///< PDP-10 SAV
  OrfVariantHPSOM,        ///< HP SOM
  OrfVariantAcornAIF,     ///< Acorn AIF
  OrfVariantAcornAOF,     ///< Acorn AOF
  OrfVariantEPOC32        ///< EPOC32/Symbian E32
} ORF_VARIANT;

///
/// ORF Context Structure
///
typedef struct _ORF_CONTEXT {
  BOOLEAN      ReadOnly;       ///< Read-only mode
  ORF_VARIANT  Variant;        ///< Format variant
  UINT8        *FileData;      ///< File data buffer
  UINT64       FileSize;       ///< File size
  BOOLEAN      OwnBuffer;      ///< TRUE if we allocated the buffer

  union {
    PEF_CONTAINER_HEADER  *Pef;
    AMIGA_HUNK_HEADER     *AmigaHunk;
    ATARI_TOS_HEADER      *AtariTos;
    VMS_OBJECT_HEADER     *Vms;
    PDP10_SAV_HEADER      *Pdp10;
    HP_SOM_HEADER         *HpSom;
    ACORN_AIF_HEADER      *AcornAif;
    EPOC_E32_HEADER       *Epoc;
  } Header;
} ORF_CONTEXT;

#define ORF_CONTEXT_FROM_BINFORMAT(ctx) ((ORF_CONTEXT *)(ctx))

/**
  Detect ORF variant from file data.

  @param[in]  Data   Pointer to file data.
  @param[in]  Size   Size of data.

  @return ORF variant type, or -1 if not recognized.

**/
STATIC
INT32
DetectOrfVariant (
  IN  UINT8   *Data,
  IN  UINT64  Size
  )
{
  if (Size < 4) {
    return -1;
  }

  UINT32 Magic32 = *(UINT32 *)Data;
  UINT16 Magic16 = *(UINT16 *)Data;

  //
  // Check for PEF
  //
  if (Magic32 == PEF_MAGIC_JOY) {
    return OrfVariantPEF;
  }

  //
  // Check for Amiga HUNK
  //
  if (Magic32 == HUNK_MAGIC || Magic32 == HUNK_UNIT) {
    return OrfVariantAmigaHunk;
  }

  //
  // Check for Atari TOS
  //
  if (Magic16 == TOS_PRG_MAGIC) {
    return OrfVariantAtariTOS;
  }

  //
  // Check for VMS
  //
  if (Magic16 == VMS_EXE_MAGIC || Magic16 == VMS_OBJ_MAGIC) {
    return OrfVariantVMS;
  }

  //
  // Check for PDP-10 SAV
  //
  if (Data[0] == PDP10_SAV_MAGIC) {
    return OrfVariantPDP10;
  }

  //
  // Check for HP SOM
  //
  if (Size >= sizeof(HP_SOM_HEADER)) {
    HP_SOM_HEADER *Som = (HP_SOM_HEADER *)Data;
    if (Som->Magic == SOM_MAGIC_EXEC ||
        Som->Magic == SOM_MAGIC_RELOC ||
        Som->Magic == SOM_MAGIC_DEMAND ||
        Som->Magic == SOM_MAGIC_SHARED ||
        Som->Magic == SOM_MAGIC_DL) {
      return OrfVariantHPSOM;
    }
  }

  //
  // Check for Acorn AIF
  //
  if (Magic32 == ACORN_AIF_MAGIC) {
    return OrfVariantAcornAIF;
  }

  //
  // Check for Acorn AOF
  //
  if (Magic32 == ACORN_AOF_MAGIC) {
    return OrfVariantAcornAOF;
  }

  //
  // Check for EPOC32
  //
  if (Size >= sizeof(EPOC_E32_HEADER)) {
    EPOC_E32_HEADER *Epoc = (EPOC_E32_HEADER *)Data;
    if (Epoc->Uid1 == EPOC_UID1 && Epoc->Uid2 == EPOC_UID_E32) {
      return OrfVariantEPOC32;
    }
  }

  return -1;
}

/**
  Initialize ORF context from file.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FilePath   Path to binary file.
  @param[in]   ReadOnly   TRUE for read-only access.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
OrfInitFile (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  )
{
  FILE         *File;
  ORF_CONTEXT  *OrfCtx;
  UINT64       FileSize;
  INT32        Variant;

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

  OrfCtx = (ORF_CONTEXT *)calloc(1, sizeof(ORF_CONTEXT));
  if (OrfCtx == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  OrfCtx->FileData = (UINT8 *)malloc(FileSize);
  if (OrfCtx->FileData == NULL) {
    free(OrfCtx);
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  if (fread(OrfCtx->FileData, 1, FileSize, File) != FileSize) {
    free(OrfCtx->FileData);
    free(OrfCtx);
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);

  OrfCtx->FileSize = FileSize;
  OrfCtx->OwnBuffer = TRUE;
  OrfCtx->ReadOnly = ReadOnly;

  //
  // Detect variant
  //
  Variant = DetectOrfVariant(OrfCtx->FileData, FileSize);
  if (Variant < 0) {
    free(OrfCtx->FileData);
    free(OrfCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  OrfCtx->Variant = (ORF_VARIANT)Variant;

  //
  // Parse headers based on variant
  //
  switch (OrfCtx->Variant) {
    case OrfVariantPEF:
      OrfCtx->Header.Pef = (PEF_CONTAINER_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantAmigaHunk:
      OrfCtx->Header.AmigaHunk = (AMIGA_HUNK_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantAtariTOS:
      OrfCtx->Header.AtariTos = (ATARI_TOS_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantVMS:
      OrfCtx->Header.Vms = (VMS_OBJECT_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantPDP10:
      OrfCtx->Header.Pdp10 = (PDP10_SAV_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantHPSOM:
      OrfCtx->Header.HpSom = (HP_SOM_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantAcornAIF:
    case OrfVariantAcornAOF:
      OrfCtx->Header.AcornAif = (ACORN_AIF_HEADER *)OrfCtx->FileData;
      break;

    case OrfVariantEPOC32:
      OrfCtx->Header.Epoc = (EPOC_E32_HEADER *)OrfCtx->FileData;
      break;

    default:
      free(OrfCtx->FileData);
      free(OrfCtx);
      return BINFORMAT_ERROR_UNSUPPORTED;
  }

  *Context = (BINFORMAT_CONTEXT *)OrfCtx;
  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS OrfInitMemory(OUT BINFORMAT_CONTEXT **Ctx, IN CONST VOID *Buf, IN UINT64 Size) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfCreate(OUT BINFORMAT_CONTEXT **Ctx, IN BINFORMAT_FILE_TYPE Type, IN BINFORMAT_MACHINE Mach, IN BOOLEAN Is64) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }

/**
  Close and free an ORF context.

  @param[in]  Context    Context to close.

**/
STATIC
VOID
OrfClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  ORF_CONTEXT  *OrfCtx;

  if (Context == NULL) {
    return;
  }

  OrfCtx = ORF_CONTEXT_FROM_BINFORMAT(Context);

  if (OrfCtx->OwnBuffer && OrfCtx->FileData != NULL) {
    free(OrfCtx->FileData);
  }

  free(OrfCtx);
}

/**
  Get ORF header information.

  @param[in]   Context      ORF context.
  @param[out]  HeaderInfo   Pointer to receive header information.

  @retval BINFORMAT_SUCCESS       Header information retrieved.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
OrfGetHeader (
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  )
{
  ORF_CONTEXT  *OrfCtx;

  if (Context == NULL || HeaderInfo == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  OrfCtx = ORF_CONTEXT_FROM_BINFORMAT(Context);
  memset(HeaderInfo, 0, sizeof(BINFORMAT_HEADER_INFO));

  HeaderInfo->FileType = BinFileTypeExecutable;
  HeaderInfo->Endianness = BinEndianBig;  // Most of these are big-endian

  switch (OrfCtx->Variant) {
    case OrfVariantPEF:
      HeaderInfo->Machine = BinMachinePowerPC;
      if (OrfCtx->Header.Pef != NULL) {
        HeaderInfo->SectionCount = OrfCtx->Header.Pef->SectionCount;
      }
      break;

    case OrfVariantAmigaHunk:
      HeaderInfo->Machine = BinMachineM68K;
      if (OrfCtx->Header.AmigaHunk != NULL) {
        HeaderInfo->SectionCount = OrfCtx->Header.AmigaHunk->NumHunks;
      }
      break;

    case OrfVariantAtariTOS:
      HeaderInfo->Machine = BinMachineM68K;
      HeaderInfo->Is64Bit = FALSE;
      break;

    case OrfVariantVMS:
      HeaderInfo->Machine = BinMachineVAX;
      HeaderInfo->Endianness = BinEndianLittle;
      break;

    case OrfVariantPDP10:
      HeaderInfo->Machine = BinMachinePDP11;  // Close enough
      if (OrfCtx->Header.Pdp10 != NULL) {
        HeaderInfo->EntryPoint = OrfCtx->Header.Pdp10->EntryPoint;
      }
      break;

    case OrfVariantHPSOM:
      HeaderInfo->Machine = BinMachineUnknown;  // HP PA-RISC
      if (OrfCtx->Header.HpSom != NULL) {
        HeaderInfo->EntryPoint = OrfCtx->Header.HpSom->EntryOffset;
      }
      break;

    case OrfVariantAcornAIF:
    case OrfVariantAcornAOF:
      HeaderInfo->Machine = BinMachineARM;
      HeaderInfo->Endianness = BinEndianLittle;
      if (OrfCtx->Header.AcornAif != NULL) {
        HeaderInfo->EntryPoint = OrfCtx->Header.AcornAif->ImageBase;
      }
      break;

    case OrfVariantEPOC32:
      HeaderInfo->Machine = BinMachineARM;
      HeaderInfo->Endianness = BinEndianLittle;
      if (OrfCtx->Header.Epoc != NULL) {
        HeaderInfo->EntryPoint = OrfCtx->Header.Epoc->EntryPoint;
      }
      break;

    default:
      break;
  }

  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS OrfGetSection(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SECTION *Sec) { (VOID)Ctx; (VOID)Idx; (VOID)Sec; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfGetSectionByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SECTION *Sec) { (VOID)Ctx; (VOID)Name; (VOID)Sec; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfGetSegment(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SEGMENT *Seg) { (VOID)Ctx; (VOID)Idx; (VOID)Seg; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfGetSymbol(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SYMBOL *Sym) { (VOID)Ctx; (VOID)Idx; (VOID)Sym; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfGetSymbolByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SYMBOL *Sym) { (VOID)Ctx; (VOID)Name; (VOID)Sym; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfGetRelocations(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, OUT BINFORMAT_RELOCATION **Rels, OUT UINT32 *Cnt) { (VOID)Ctx; (VOID)SecIdx; (VOID)Rels; (VOID)Cnt; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfAddSection(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SECTION *Sec, OUT UINT32 *Idx) { (VOID)Ctx; (VOID)Sec; (VOID)Idx; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfAddSymbol(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SYMBOL *Sym, OUT UINT32 *Idx) { (VOID)Ctx; (VOID)Sym; (VOID)Idx; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfAddRelocation(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, IN BINFORMAT_RELOCATION *Rel) { (VOID)Ctx; (VOID)SecIdx; (VOID)Rel; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfWriteFile(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Path) { (VOID)Ctx; (VOID)Path; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfWriteMemory(IN BINFORMAT_CONTEXT *Ctx, OUT VOID *Buf, IN UINT64 Size, OUT UINT64 *Written) { (VOID)Ctx; (VOID)Buf; (VOID)Size; (VOID)Written; return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS OrfSelectArchitecture(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 ArchIdx) { (VOID)Ctx; (VOID)ArchIdx; return BINFORMAT_ERROR_UNSUPPORTED; }

STATIC CONST BINFORMAT_API gOrfApi = {
  .LibraryName = "liborf",
  .Version = 1,
  .InitFile = OrfInitFile,
  .InitMemory = OrfInitMemory,
  .Create = OrfCreate,
  .Close = OrfClose,
  .GetHeader = OrfGetHeader,
  .GetSection = OrfGetSection,
  .GetSectionByName = OrfGetSectionByName,
  .GetSegment = OrfGetSegment,
  .GetSymbol = OrfGetSymbol,
  .GetSymbolByName = OrfGetSymbolByName,
  .AddSection = OrfAddSection,
  .AddSymbol = OrfAddSymbol,
  .AddRelocation = OrfAddRelocation,
  .WriteFile = OrfWriteFile,
  .WriteMemory = OrfWriteMemory,
  .SelectArchitecture = OrfSelectArchitecture
};

/**
  Get the ORF library API table.

  @return Pointer to ORF library API table.

**/
CONST BINFORMAT_API *
OrfGetApi (
  VOID
  )
{
  return &gOrfApi;
}
