/** @file
  MMIX Linker interface.

  This file provides the linker interface for combining MMIX
  object files into executables.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_LINKER_H_
#define MMIX_LINKER_H_

#include "MmixTypes.h"
#include "MmixAsm.h"

#define MMIX_MAX_INPUT_FILES  256

/**
  Linker output format
**/
typedef enum {
  MmixOutputRaw,
  MmixOutputElf,
  MmixOutputMmo
} MMIX_OUTPUT_FORMAT;

/**
  Linker context
**/
typedef struct {
  CHAR8                *InputFiles[MMIX_MAX_INPUT_FILES];
  UINT32               InputFileCount;
  CHAR8                *OutputFile;
  MMIX_OUTPUT_FORMAT   OutputFormat;
  UINT64               BaseAddress;
  UINT64               EntryPoint;
  BOOLEAN              Relocatable;
  MMIX_ASM_CONTEXT     *Objects[MMIX_MAX_INPUT_FILES];
  UINT32               ObjectCount;
  MMIX_SYMBOL          GlobalSymbols[MMIX_MAX_SYMBOLS];
  UINT32               GlobalSymbolCount;
} MMIX_LINKER_CONTEXT;

/**
  Create a new linker context.

  @param[out]     Context       Pointer to receive context.

  @retval MMIX_SUCCESS          Context created successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerCreate (
  OUT MMIX_LINKER_CONTEXT  **Context
  );

/**
  Destroy a linker context.

  @param[in]      Context       Linker context.

**/
VOID
MmixLinkerDestroy (
  IN  MMIX_LINKER_CONTEXT  *Context
  );

/**
  Add an input file to the linker.

  @param[in,out]  Context       Linker context.
  @param[in]      FilePath      Path to input file.

  @retval MMIX_SUCCESS          File added successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerAddInput (
  IN OUT MMIX_LINKER_CONTEXT  *Context,
  IN     CONST CHAR8          *FilePath
  );

/**
  Set linker output options.

  @param[in,out]  Context       Linker context.
  @param[in]      OutputFile    Output file path.
  @param[in]      Format        Output format.
  @param[in]      BaseAddress   Base address for output.

  @retval MMIX_SUCCESS          Options set successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerSetOutput (
  IN OUT MMIX_LINKER_CONTEXT  *Context,
  IN     CONST CHAR8          *OutputFile,
  IN     MMIX_OUTPUT_FORMAT   Format,
  IN     UINT64               BaseAddress
  );

/**
  Perform the link operation.

  @param[in,out]  Context       Linker context.

  @retval MMIX_SUCCESS          Link successful.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLinkerLink (
  IN OUT MMIX_LINKER_CONTEXT  *Context
  );

#endif // MMIX_LINKER_H_
