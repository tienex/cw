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
STATIC BINFORMAT_STATUS MachoWriteFile(IN BINFORMAT_CONTEXT *Ctx, IN CONST CHAR8 *Path) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoWriteMemory(IN BINFORMAT_CONTEXT *Ctx, OUT VOID *Buf, IN UINT64 Size, OUT UINT64 *Written) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }
STATIC BINFORMAT_STATUS MachoSelectArchitecture(IN BINFORMAT_CONTEXT *Ctx, IN UINT32 ArchIdx) { return BINFORMAT_ERROR_NOT_IMPLEMENTED; }

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
  .GetRelocations = MachoGetRelocations,
  .AddSection = MachoAddSection,
  .AddSymbol = MachoAddSymbol,
  .AddRelocation = MachoAddRelocation,
  .WriteFile = MachoWriteFile,
  .WriteMemory = MachoWriteMemory,
  .SelectArchitecture = MachoSelectArchitecture
};

CONST BINFORMAT_API *
MachoGetApi (
  VOID
  )
{
  return &gMachoApi;
}
