/** @file
  Windows Minidump Binary Format Library Implementation.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include "LibMinidump.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

///
/// Minidump context structure
///
typedef struct {
  BOOLEAN  ReadOnly;
  VOID     *FileData;
  UINT64   FileSize;
  PMINIDUMP_HEADER Header;
  PMINIDUMP_DIRECTORY Directory;
} MINIDUMP_CONTEXT;

//
// Forward declarations
//
STATIC BINFORMAT_STATUS MinidumpInitFile(OUT BINFORMAT_CONTEXT **Context, IN CONST CHAR8 *FilePath, IN BOOLEAN ReadOnly);
STATIC BINFORMAT_STATUS MinidumpInitMemory(OUT BINFORMAT_CONTEXT **Context, IN CONST VOID *Buffer, IN UINT64 Size);
STATIC BINFORMAT_STATUS MinidumpCreate(OUT BINFORMAT_CONTEXT **Context, IN BINFORMAT_FILE_TYPE FileType, IN BINFORMAT_MACHINE Machine, IN BOOLEAN Is64Bit);
STATIC BINFORMAT_STATUS MinidumpClose(IN BINFORMAT_CONTEXT *Context);
STATIC BINFORMAT_STATUS MinidumpGetHeader(IN BINFORMAT_CONTEXT *Context, OUT BINFORMAT_HEADER_INFO *Info);
STATIC BINFORMAT_STATUS MinidumpGetSection(IN BINFORMAT_CONTEXT *Context, IN UINT32 Index, OUT BINFORMAT_SECTION *Section);
STATIC BINFORMAT_STATUS MinidumpGetSectionByName(IN BINFORMAT_CONTEXT *Context, IN CONST CHAR8 *Name, OUT BINFORMAT_SECTION *Section);
STATIC BINFORMAT_STATUS MinidumpGetSegment(IN BINFORMAT_CONTEXT *Context, IN UINT32 Index, OUT BINFORMAT_SEGMENT *Segment);
STATIC BINFORMAT_STATUS MinidumpGetSymbol(IN BINFORMAT_CONTEXT *Context, IN UINT32 Index, OUT BINFORMAT_SYMBOL *Symbol);
STATIC BINFORMAT_STATUS MinidumpGetSymbolByName(IN BINFORMAT_CONTEXT *Context, IN CONST CHAR8 *Name, OUT BINFORMAT_SYMBOL *Symbol);
STATIC BINFORMAT_STATUS MinidumpGetRelocations(IN BINFORMAT_CONTEXT *Context, IN UINT32 SectionIndex, OUT BINFORMAT_RELOCATION **Relocations, OUT UINT32 *Count);
STATIC BINFORMAT_STATUS MinidumpWriteFile(IN BINFORMAT_CONTEXT *Context, IN CONST CHAR8 *FilePath);
STATIC BINFORMAT_STATUS MinidumpWriteMemory(IN BINFORMAT_CONTEXT *Context, OUT VOID *Buffer, IN UINT64 Size, OUT UINT64 *Written);
STATIC BINFORMAT_STATUS MinidumpAddSection(IN BINFORMAT_CONTEXT *Context, IN BINFORMAT_SECTION *Section, OUT UINT32 *Index);
STATIC BINFORMAT_STATUS MinidumpAddSymbol(IN BINFORMAT_CONTEXT *Context, IN BINFORMAT_SYMBOL *Symbol, OUT UINT32 *Index);
STATIC BINFORMAT_STATUS MinidumpAddRelocation(IN BINFORMAT_CONTEXT *Context, IN UINT32 SectionIndex, IN BINFORMAT_RELOCATION *Relocation);
STATIC BINFORMAT_STATUS MinidumpSelectArchitecture(IN BINFORMAT_CONTEXT *Context, IN UINT32 ArchitectureIndex);

///
/// Minidump library API table
///
STATIC CONST BINFORMAT_API  mMinidumpApi = {
  .LibraryName          = "libminidump",
  .Version              = 1,
  .InitFile             = MinidumpInitFile,
  .InitMemory           = MinidumpInitMemory,
  .Create               = MinidumpCreate,
  .Close                = MinidumpClose,
  .GetHeader            = MinidumpGetHeader,
  .GetSection           = MinidumpGetSection,
  .GetSectionByName     = MinidumpGetSectionByName,
  .GetSegment           = MinidumpGetSegment,
  .GetSymbol            = MinidumpGetSymbol,
  .GetSymbolByName      = MinidumpGetSymbolByName,
  .GetRelocations       = MinidumpGetRelocations,
  .WriteFile            = MinidumpWriteFile,
  .WriteMemory          = MinidumpWriteMemory,
  .AddSection           = MinidumpAddSection,
  .AddSymbol            = MinidumpAddSymbol,
  .AddRelocation        = MinidumpAddRelocation,
  .SelectArchitecture   = MinidumpSelectArchitecture
};

/**
  Get the Minidump library API table.

  @return Pointer to Minidump library API table.

**/
CONST BINFORMAT_API *
MinidumpGetApi (
  VOID
  )
{
  return &mMinidumpApi;
}

/**
  Initialize Minidump context from file.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FilePath   Path to minidump file.
  @param[in]   ReadOnly   TRUE for read-only access.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
MinidumpInitFile (
  OUT  BINFORMAT_CONTEXT  **Context,
  IN   CONST CHAR8        *FilePath,
  IN   BOOLEAN            ReadOnly
  )
{
  FILE              *File;
  MINIDUMP_CONTEXT  *MdmpCtx;
  UINT64            FileSize;

  if ((Context == NULL) || (FilePath == NULL)) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  File = fopen (FilePath, ReadOnly ? "rb" : "r+b");
  if (File == NULL) {
    return BINFORMAT_ERROR_NOT_FOUND;
  }

  //
  // Get file size
  //
  fseek (File, 0, SEEK_END);
  FileSize = ftell (File);
  fseek (File, 0, SEEK_SET);

  //
  // Allocate context
  //
  MdmpCtx = (MINIDUMP_CONTEXT *)malloc (sizeof (MINIDUMP_CONTEXT));
  if (MdmpCtx == NULL) {
    fclose (File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Allocate file buffer
  //
  MdmpCtx->FileData = malloc (FileSize);
  if (MdmpCtx->FileData == NULL) {
    free (MdmpCtx);
    fclose (File);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Read entire file
  //
  if (fread (MdmpCtx->FileData, 1, FileSize, File) != FileSize) {
    free (MdmpCtx->FileData);
    free (MdmpCtx);
    fclose (File);
    return BINFORMAT_ERROR_IO;
  }

  fclose (File);

  //
  // Initialize context
  //
  MdmpCtx->ReadOnly = ReadOnly;
  MdmpCtx->FileSize = FileSize;
  MdmpCtx->Header   = (PMINIDUMP_HEADER)MdmpCtx->FileData;

  //
  // Validate header
  //
  if ((MdmpCtx->Header->Signature != MINIDUMP_SIGNATURE) ||
      (MdmpCtx->Header->Version != MINIDUMP_VERSION)) {
    free (MdmpCtx->FileData);
    free (MdmpCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  //
  // Get directory
  //
  MdmpCtx->Directory = (PMINIDUMP_DIRECTORY)((UINT8 *)MdmpCtx->FileData + MdmpCtx->Header->StreamDirectoryRva);

  *Context = (BINFORMAT_CONTEXT *)MdmpCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Initialize Minidump context from memory buffer.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   Buffer     Pointer to minidump data.
  @param[in]   Size       Size of buffer in bytes.

  @retval BINFORMAT_SUCCESS       Context initialized successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
MinidumpInitMemory (
  OUT  BINFORMAT_CONTEXT  **Context,
  IN   CONST VOID         *Buffer,
  IN   UINT64             Size
  )
{
  MINIDUMP_CONTEXT  *MdmpCtx;

  if ((Context == NULL) || (Buffer == NULL) || (Size < sizeof (MINIDUMP_HEADER))) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate context
  //
  MdmpCtx = (MINIDUMP_CONTEXT *)malloc (sizeof (MINIDUMP_CONTEXT));
  if (MdmpCtx == NULL) {
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  //
  // Allocate and copy buffer
  //
  MdmpCtx->FileData = malloc (Size);
  if (MdmpCtx->FileData == NULL) {
    free (MdmpCtx);
    return BINFORMAT_ERROR_OUT_OF_MEMORY;
  }

  memcpy (MdmpCtx->FileData, Buffer, Size);

  //
  // Initialize context
  //
  MdmpCtx->ReadOnly = TRUE;
  MdmpCtx->FileSize = Size;
  MdmpCtx->Header   = (PMINIDUMP_HEADER)MdmpCtx->FileData;

  //
  // Validate header
  //
  if ((MdmpCtx->Header->Signature != MINIDUMP_SIGNATURE) ||
      (MdmpCtx->Header->Version != MINIDUMP_VERSION)) {
    free (MdmpCtx->FileData);
    free (MdmpCtx);
    return BINFORMAT_ERROR_INVALID_FORMAT;
  }

  //
  // Get directory
  //
  MdmpCtx->Directory = (PMINIDUMP_DIRECTORY)((UINT8 *)MdmpCtx->FileData + MdmpCtx->Header->StreamDirectoryRva);

  *Context = (BINFORMAT_CONTEXT *)MdmpCtx;
  return BINFORMAT_SUCCESS;
}

/**
  Create a new minidump context.

  @param[out]  Context    Pointer to receive context handle.
  @param[in]   FileType   Type of file to create.
  @param[in]   Machine    Target machine architecture.
  @param[in]   Is64Bit    TRUE for 64-bit, FALSE for 32-bit.

  @retval BINFORMAT_SUCCESS       Context created successfully.
  @retval BINFORMAT_ERROR_*       Error occurred.

**/
STATIC
BINFORMAT_STATUS
MinidumpCreate (
  OUT  BINFORMAT_CONTEXT   **Context,
  IN   BINFORMAT_FILE_TYPE FileType,
  IN   BINFORMAT_MACHINE   Machine,
  IN   BOOLEAN             Is64Bit
  )
{
  // TODO: Implement minidump creation
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Close minidump context and free resources.

  @param[in]  Context  Context handle.

  @retval BINFORMAT_SUCCESS  Context closed successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpClose (
  IN  BINFORMAT_CONTEXT  *Context
  )
{
  MINIDUMP_CONTEXT  *MdmpCtx;

  if (Context == NULL) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  MdmpCtx = (MINIDUMP_CONTEXT *)Context;

  if (MdmpCtx->FileData != NULL) {
    free (MdmpCtx->FileData);
  }

  free (MdmpCtx);
  return BINFORMAT_SUCCESS;
}

/**
  Get minidump header information.

  @param[in]   Context  Context handle.
  @param[out]  Info     Pointer to receive header information.

  @retval BINFORMAT_SUCCESS  Information retrieved successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetHeader (
  IN   BINFORMAT_CONTEXT      *Context,
  OUT  BINFORMAT_HEADER_INFO  *Info
  )
{
  // TODO: Extract system info from minidump
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Get section (stream) from minidump by index.

  @param[in]   Context  Context handle.
  @param[in]   Index    Section index.
  @param[out]  Section  Pointer to receive section.

  @retval BINFORMAT_SUCCESS  Section retrieved successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetSection (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   UINT32             Index,
  OUT  BINFORMAT_SECTION  *Section
  )
{
  // TODO: Convert minidump stream to section
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Get section (stream) from minidump by name.

  @param[in]   Context  Context handle.
  @param[in]   Name     Section name.
  @param[out]  Section  Pointer to receive section.

  @retval BINFORMAT_SUCCESS  Section retrieved successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetSectionByName (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   CONST CHAR8        *Name,
  OUT  BINFORMAT_SECTION  *Section
  )
{
  // TODO: Convert minidump stream to section
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Get segment from minidump (memory regions).

  @param[in]   Context  Context handle.
  @param[in]   Index    Segment index.
  @param[out]  Segment  Pointer to receive segment.

  @retval BINFORMAT_SUCCESS  Segment retrieved successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetSegment (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   UINT32             Index,
  OUT  BINFORMAT_SEGMENT  *Segment
  )
{
  // TODO: Convert memory descriptor to segment
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Get symbol from minidump (module) by index.

  @param[in]   Context  Context handle.
  @param[in]   Index    Symbol index.
  @param[out]  Symbol   Pointer to receive symbol.

  @retval BINFORMAT_SUCCESS  Symbol retrieved successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetSymbol (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   UINT32             Index,
  OUT  BINFORMAT_SYMBOL   *Symbol
  )
{
  // TODO: Convert module to symbol
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Get symbol from minidump (module) by name.

  @param[in]   Context  Context handle.
  @param[in]   Name     Symbol name.
  @param[out]  Symbol   Pointer to receive symbol.

  @retval BINFORMAT_SUCCESS  Symbol retrieved successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetSymbolByName (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   CONST CHAR8        *Name,
  OUT  BINFORMAT_SYMBOL   *Symbol
  )
{
  // TODO: Convert module to symbol
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Get relocations (not applicable to minidumps).

  @retval BINFORMAT_ERROR_UNSUPPORTED  Operation not supported.

**/
STATIC
BINFORMAT_STATUS
MinidumpGetRelocations (
  IN   BINFORMAT_CONTEXT      *Context,
  IN   UINT32                 SectionIndex,
  OUT  BINFORMAT_RELOCATION   **Relocations,
  OUT  UINT32                 *Count
  )
{
  return BINFORMAT_ERROR_UNSUPPORTED;
}

/**
  Write minidump to file.

  @param[in]  Context   Context handle.
  @param[in]  FilePath  Output file path.

  @retval BINFORMAT_SUCCESS  File written successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpWriteFile (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  CONST CHAR8        *FilePath
  )
{
  // TODO: Implement minidump writing
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Write minidump to memory buffer.

  @param[in]   Context  Context handle.
  @param[out]  Buffer   Buffer to write to.
  @param[in]   Size     Size of buffer.
  @param[out]  Written  Bytes written.

  @retval BINFORMAT_SUCCESS  Buffer written successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpWriteMemory (
  IN   BINFORMAT_CONTEXT  *Context,
  OUT  VOID               *Buffer,
  IN   UINT64             Size,
  OUT  UINT64             *Written
  )
{
  // TODO: Implement minidump writing
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Add section (stream) to minidump.

  @param[in]   Context  Context handle.
  @param[in]   Section  Section to add.
  @param[out]  Index    Index of added section.

  @retval BINFORMAT_SUCCESS  Section added successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpAddSection (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   BINFORMAT_SECTION  *Section,
  OUT  UINT32             *Index
  )
{
  // TODO: Add stream to minidump
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Add symbol (module) to minidump.

  @param[in]   Context  Context handle.
  @param[in]   Symbol   Symbol to add.
  @param[out]  Index    Index of added symbol.

  @retval BINFORMAT_SUCCESS  Symbol added successfully.

**/
STATIC
BINFORMAT_STATUS
MinidumpAddSymbol (
  IN   BINFORMAT_CONTEXT  *Context,
  IN   BINFORMAT_SYMBOL   *Symbol,
  OUT  UINT32             *Index
  )
{
  // TODO: Add module to minidump
  return BINFORMAT_ERROR_NOT_IMPLEMENTED;
}

/**
  Add relocation (not applicable to minidumps).

  @retval BINFORMAT_ERROR_UNSUPPORTED  Operation not supported.

**/
STATIC
BINFORMAT_STATUS
MinidumpAddRelocation (
  IN  BINFORMAT_CONTEXT     *Context,
  IN  UINT32                SectionIndex,
  IN  BINFORMAT_RELOCATION  *Relocation
  )
{
  return BINFORMAT_ERROR_UNSUPPORTED;
}

/**
  Select architecture (not applicable to minidumps).

  @retval BINFORMAT_ERROR_UNSUPPORTED  Operation not supported.

**/
STATIC
BINFORMAT_STATUS
MinidumpSelectArchitecture (
  IN  BINFORMAT_CONTEXT  *Context,
  IN  UINT32             ArchitectureIndex
  )
{
  return BINFORMAT_ERROR_UNSUPPORTED;
}
