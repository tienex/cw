/** @file
  a.out Binary Format Library Implementation.

  This file implements support for a.out, b.out, and Plan 9 a.out binary
  formats following the unified binary format API.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/binformat/LibAout.h"

#pragma pack(push, 1)

///
/// Traditional a.out header
///
typedef struct {
  UINT16  Magic;      ///< Magic number
  UINT16  MachType;   ///< Machine type
  UINT32  TextSize;   ///< Text segment size
  UINT32  DataSize;   ///< Data segment size
  UINT32  BssSize;    ///< BSS segment size
  UINT32  SymSize;    ///< Symbol table size
  UINT32  Entry;      ///< Entry point
  UINT32  TextRelSize; ///< Text relocation size
  UINT32  DataRelSize; ///< Data relocation size
} AOUT_HEADER;

///
/// Plan 9 a.out header
///
typedef struct {
  UINT32  Magic;      ///< Magic number
  UINT32  TextSize;   ///< Text size
  UINT32  DataSize;   ///< Data size
  UINT32  BssSize;    ///< BSS size
  UINT32  SymSize;    ///< Symbol size
  UINT32  Entry;      ///< Entry point
  UINT32  SpsSize;    ///< PC/SP offset table size
  UINT32  PcsSize;    ///< PC/line number table size
} PLAN9_HEADER;

#pragma pack(pop)

typedef enum {
  AoutVariantTraditional,
  AoutVariantBout,
  AoutVariantPlan9
} AOUT_VARIANT;

typedef struct _AOUT_CONTEXT {
  BOOLEAN       ReadOnly;
  AOUT_VARIANT  Variant;
  UINT8         *FileData;
  UINT64        FileSize;
  BOOLEAN       OwnBuffer;

  union {
    AOUT_HEADER   *Aout;
    PLAN9_HEADER  *Plan9;
  } Header;
} AOUT_CONTEXT;

#define AOUT_CONTEXT_FROM_BINFORMAT(ctx) ((AOUT_CONTEXT *)(ctx))

STATIC
BINFORMAT_STATUS
AoutInitFile (
  OUT BINFORMAT_CONTEXT  **Context,
  IN  CONST CHAR8        *FilePath,
  IN  BOOLEAN            ReadOnly
  )
{
  FILE          *File;
  AOUT_CONTEXT  *AoutCtx;
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

  AoutCtx = (AOUT_CONTEXT *)calloc(1, sizeof(AOUT_CONTEXT));
  if (AoutCtx == NULL) {
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  AoutCtx->FileData = (UINT8 *)malloc(FileSize);
  if (AoutCtx->FileData == NULL) {
    free(AoutCtx);
    fclose(File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  if (fread(AoutCtx->FileData, 1, FileSize, File) != FileSize) {
    free(AoutCtx->FileData);
    free(AoutCtx);
    fclose(File);
    return BINFORMAT_ERROR_IO;
  }

  fclose(File);

  AoutCtx->FileSize = FileSize;
  AoutCtx->OwnBuffer = TRUE;
  AoutCtx->ReadOnly = ReadOnly;

  //
  // Detect variant by magic
  //
  UINT32 Magic32 = *(UINT32 *)AoutCtx->FileData;

  if ((Magic32 & 0xFF000000) == 0x80000000) {
    AoutCtx->Variant = AoutVariantPlan9;
    AoutCtx->Header.Plan9 = (PLAN9_HEADER *)AoutCtx->FileData;
  } else {
    AoutCtx->Variant = AoutVariantTraditional;
    AoutCtx->Header.Aout = (AOUT_HEADER *)AoutCtx->FileData;
  }

  *Context = (BINFORMAT_CONTEXT *)AoutCtx;
  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS AoutInitMemory(OUT BINFORMAT_CONTEXT **Ctx, IN CONST VOID *Buf, IN UINT64 Size) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutCreate(OUT BINFORMAT_CONTEXT **Ctx, IN BINFORMAT_FILE_TYPE Type, IN BINFORMAT_MACHINE Mach, IN BOOLEAN Is64) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }

STATIC
VOID
AoutClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  AOUT_CONTEXT  *AoutCtx;

  if (Context == NULL) {
    return;
  }

  AoutCtx = AOUT_CONTEXT_FROM_BINFORMAT(Context);

  if (AoutCtx->OwnBuffer && AoutCtx->FileData != NULL) {
    free(AoutCtx->FileData);
  }

  free(AoutCtx);
}

STATIC
BINFORMAT_STATUS
AoutGetHeader (
  IN  BINFORMAT_CONTEXT      *Context,
  OUT BINFORMAT_HEADER_INFO  *HeaderInfo
  )
{
  AOUT_CONTEXT  *AoutCtx;

  if (Context == NULL || HeaderInfo == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  AoutCtx = AOUT_CONTEXT_FROM_BINFORMAT(Context);
  memset(HeaderInfo, 0, sizeof(BINFORMAT_HEADER_INFO));

  HeaderInfo->Is64Bit = FALSE;
  HeaderInfo->FileType = BinFileTypeExecutable;
  HeaderInfo->Endianness = BinEndianLittle;

  if (AoutCtx->Variant == AoutVariantPlan9) {
    HeaderInfo->EntryPoint = AoutCtx->Header.Plan9->Entry;

    switch (AoutCtx->Header.Plan9->Magic) {
      case PLAN9_MAGIC_386:    HeaderInfo->Machine = BinMachineX86; break;
      case PLAN9_MAGIC_ARM:    HeaderInfo->Machine = BinMachineARM; break;
      case PLAN9_MAGIC_MIPS:   HeaderInfo->Machine = BinMachineMIPS; break;
      case PLAN9_MAGIC_SPARC:  HeaderInfo->Machine = BinMachineSPARC; break;
      case PLAN9_MAGIC_ALPHA:  HeaderInfo->Machine = BinMachineAlpha; break;
      case PLAN9_MAGIC_POWER:  HeaderInfo->Machine = BinMachinePowerPC; break;
      default:                 HeaderInfo->Machine = BinMachineUnknown; break;
    }
  } else {
    HeaderInfo->EntryPoint = AoutCtx->Header.Aout->Entry;
    HeaderInfo->Machine = BinMachineUnknown;
  }

  return BINFORMAT_SUCCESS;
}

STATIC BINFORMAT_STATUS AoutGetSection(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutGetSectionByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SECTION *Sec) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutGetSegment(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SEGMENT *Seg) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutGetSymbol(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 Idx, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutGetSymbolByName(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Name, OUT BINFORMAT_SYMBOL *Sym) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutGetRelocations(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, OUT BINFORMAT_RELOCATION **Rels, OUT UINT32 *Cnt) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutAddSection(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SECTION *Sec, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutAddSymbol(IN BINFORMAT_CONTEXT *Ctx, IN BINFORMAT_SYMBOL *Sym, OUT UINT32 *Idx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutAddRelocation(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 SecIdx, IN BINFORMAT_RELOCATION *Rel) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutWriteFile(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Path) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutWriteMemory(IN BINFORMAT_CONTEXT *Ctx, OUT VOID *Buf, IN UINT64 Size, OUT UINT64 *Written) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS AoutSelectArchitecture(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 ArchIdx) { return BINFORMAT_ERROR_UNSUPPORTED; }

STATIC CONST BINFORMAT_API gAoutApi = {
  .LibraryName = "libaout",
  .Version = 1,
  .InitFile = AoutInitFile,
  .InitMemory = AoutInitMemory,
  .Create = AoutCreate,
  .Close = AoutClose,
  .GetHeader = AoutGetHeader,
  .GetSection = AoutGetSection,
  .GetSectionByName = AoutGetSectionByName,
  .GetSegment = AoutGetSegment,
  .GetSymbol = AoutGetSymbol,
  .GetSymbolByName = AoutGetSymbolByName,
  .GetRelocations = AoutGetRelocations,
  .AddSection = AoutAddSection,
  .AddSymbol = AoutAddSymbol,
  .AddRelocation = AoutAddRelocation,
  .WriteFile = AoutWriteFile,
  .WriteMemory = AoutWriteMemory,
  .SelectArchitecture = AoutSelectArchitecture
};

CONST BINFORMAT_API *
AoutGetApi (
  VOID
  )
{
  return &gAoutApi;
}
