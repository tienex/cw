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
  .RemoveSignature = NULL
};

CONST BINFORMAT_API *
MachoGetApi (
  VOID
  )
{
  return &gMachoApi;
}
