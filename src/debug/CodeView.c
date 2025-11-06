/** @file
  CodeView debug information implementation.

  This file implements CodeView debug information generation for MMIX.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/MmixCodeView.h"

/**
  Create CodeView debug context.

  @param[out]     Context       Pointer to receive context.

  @retval MMIX_SUCCESS          Context created.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixCodeViewCreate (
  OUT CV_DEBUG_CONTEXT  **Context
  )
{
  CV_DEBUG_CONTEXT  *Ctx;

  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Ctx = (CV_DEBUG_CONTEXT *)calloc (1, sizeof (CV_DEBUG_CONTEXT));
  if (Ctx == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  //
  // Allocate string table
  //
  Ctx->StringTableSize = 4096;
  Ctx->StringTable = (UINT8 *)calloc (1, Ctx->StringTableSize);
  if (Ctx->StringTable == NULL) {
    free (Ctx);
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  *Context = Ctx;
  return MMIX_SUCCESS;
}

/**
  Destroy CodeView debug context.

  @param[in]      Context       Debug context.

**/
VOID
MmixCodeViewDestroy (
  IN  CV_DEBUG_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return;
  }

  //
  // Free symbols
  //
  for (UINT32 i = 0; i < Context->SymbolCount; i++) {
    if (Context->Symbols[i] != NULL) {
      free (Context->Symbols[i]);
    }
  }

  //
  // Free types
  //
  for (UINT32 i = 0; i < Context->TypeCount; i++) {
    if (Context->Types[i] != NULL) {
      free (Context->Types[i]);
    }
  }

  //
  // Free lines
  //
  for (UINT32 i = 0; i < Context->LineCount; i++) {
    if (Context->Lines[i] != NULL) {
      free (Context->Lines[i]);
    }
  }

  //
  // Free files
  //
  for (UINT32 i = 0; i < Context->FileCount; i++) {
    if (Context->Files[i] != NULL) {
      free (Context->Files[i]);
    }
  }

  if (Context->StringTable != NULL) {
    free (Context->StringTable);
  }

  free (Context);
}

/**
  Add compile symbol.

  @param[in,out]  Context       Debug context.
  @param[in]      Machine       Target machine.
  @param[in]      Language      Source language.
  @param[in]      Version       Compiler version.

  @retval MMIX_SUCCESS          Symbol added.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixCodeViewAddCompile (
  IN OUT CV_DEBUG_CONTEXT  *Context,
  IN     UINT8             Machine,
  IN     UINT8             Language,
  IN     CONST CHAR8       *Version
  )
{
  CV_COMPILE_SYMBOL  *Symbol;
  UINT32             VersionLen;
  UINT32             TotalSize;

  if (Context == NULL || Version == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->SymbolCount >= CV_MAX_SYMBOLS) {
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  VersionLen = strlen (Version) + 1;
  TotalSize = sizeof (CV_COMPILE_SYMBOL) + VersionLen;

  Symbol = (CV_COMPILE_SYMBOL *)malloc (TotalSize);
  if (Symbol == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Symbol->Header.Length = (UINT16)(TotalSize - sizeof (UINT16));
  Symbol->Header.Type = S_COMPILE;
  Symbol->Machine = Machine;
  Symbol->Language = Language;
  Symbol->Flags = 0;
  memcpy (Symbol->Version, Version, VersionLen);

  Context->Symbols[Context->SymbolCount++] = (CV_SYMBOL_HEADER *)Symbol;
  return MMIX_SUCCESS;
}

/**
  Add public symbol.

  @param[in,out]  Context       Debug context.
  @param[in]      Name          Symbol name.
  @param[in]      Offset        Symbol offset.
  @param[in]      Segment       Symbol segment.

  @retval MMIX_SUCCESS          Symbol added.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixCodeViewAddPublic (
  IN OUT CV_DEBUG_CONTEXT  *Context,
  IN     CONST CHAR8       *Name,
  IN     UINT32            Offset,
  IN     UINT16            Segment
  )
{
  CV_PUBLIC_SYMBOL  *Symbol;
  UINT32            NameLen;
  UINT32            TotalSize;

  if (Context == NULL || Name == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->SymbolCount >= CV_MAX_SYMBOLS) {
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  NameLen = strlen (Name) + 1;
  TotalSize = sizeof (CV_PUBLIC_SYMBOL) + NameLen;

  Symbol = (CV_PUBLIC_SYMBOL *)malloc (TotalSize);
  if (Symbol == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Symbol->Header.Length = (UINT16)(TotalSize - sizeof (UINT16));
  Symbol->Header.Type = S_PUB32;
  Symbol->Offset = Offset;
  Symbol->Segment = Segment;
  Symbol->Type = T_VOID;
  memcpy (Symbol->Name, Name, NameLen);

  Context->Symbols[Context->SymbolCount++] = (CV_SYMBOL_HEADER *)Symbol;
  return MMIX_SUCCESS;
}

/**
  Add line number information.

  @param[in,out]  Context       Debug context.
  @param[in]      File          Source file name.
  @param[in]      Line          Line number.
  @param[in]      Offset        Code offset.

  @retval MMIX_SUCCESS          Line info added.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixCodeViewAddLine (
  IN OUT CV_DEBUG_CONTEXT  *Context,
  IN     CONST CHAR8       *File,
  IN     UINT32            Line,
  IN     UINT32            Offset
  )
{
  CV_LINE_ENTRY  *LineEntry;
  BOOLEAN        FileExists;

  if (Context == NULL || File == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->LineCount >= CV_MAX_LINES) {
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  //
  // Check if file already registered
  //
  FileExists = FALSE;
  for (UINT32 i = 0; i < Context->FileCount; i++) {
    if (strcmp (Context->Files[i], File) == 0) {
      FileExists = TRUE;
      break;
    }
  }

  //
  // Add file if new
  //
  if (!FileExists) {
    if (Context->FileCount >= CV_MAX_FILES) {
      return MMIX_ERROR_OUT_OF_RESOURCES;
    }
    Context->Files[Context->FileCount] = strdup (File);
    if (Context->Files[Context->FileCount] == NULL) {
      return MMIX_ERROR_OUT_OF_MEMORY;
    }
    Context->FileCount++;
  }

  //
  // Create line entry
  //
  LineEntry = (CV_LINE_ENTRY *)malloc (sizeof (CV_LINE_ENTRY));
  if (LineEntry == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  LineEntry->Offset = Offset;
  LineEntry->LineStart = Line & 0xFFFFFF;
  LineEntry->DeltaEnd = 0;
  LineEntry->Statement = 1;

  Context->Lines[Context->LineCount++] = LineEntry;
  return MMIX_SUCCESS;
}

/**
  Write CodeView debug information to file.

  @param[in]      Context       Debug context.
  @param[in]      FilePath      Output file path.

  @retval MMIX_SUCCESS          Debug info written.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixCodeViewWrite (
  IN  CV_DEBUG_CONTEXT  *Context,
  IN  CONST CHAR8       *FilePath
  )
{
  FILE    *File;
  UINT32  Signature;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  File = fopen (FilePath, "wb");
  if (File == NULL) {
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Write signature
  //
  Signature = CV_SIGNATURE_C13;
  fwrite (&Signature, sizeof (UINT32), 1, File);

  //
  // Write symbols
  //
  for (UINT32 i = 0; i < Context->SymbolCount; i++) {
    CV_SYMBOL_HEADER  *Sym = Context->Symbols[i];
    fwrite (Sym, Sym->Length + sizeof (UINT16), 1, File);
  }

  //
  // Write line numbers
  //
  if (Context->LineCount > 0) {
    UINT32  SubsectionType = DEBUG_S_LINES;
    UINT32  SubsectionLength = Context->LineCount * sizeof (CV_LINE_ENTRY);

    fwrite (&SubsectionType, sizeof (UINT32), 1, File);
    fwrite (&SubsectionLength, sizeof (UINT32), 1, File);

    for (UINT32 i = 0; i < Context->LineCount; i++) {
      fwrite (Context->Lines[i], sizeof (CV_LINE_ENTRY), 1, File);
    }
  }

  fclose (File);
  return MMIX_SUCCESS;
}
