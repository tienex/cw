/** @file
  MMIX Assembler interface.

  This file provides the assembler interface for converting MMIX
  assembly source code into binary object files.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_ASM_H_
#define MMIX_ASM_H_

#include "MmixTypes.h"

//
// Maximum symbol table size
//
#define MMIX_MAX_SYMBOLS     4096
#define MMIX_MAX_RELOCATIONS 4096
#define MMIX_MAX_SECTIONS    64

/**
  Symbol types
**/
typedef enum {
  MmixSymbolLocal,
  MmixSymbolGlobal,
  MmixSymbolWeak,
  MmixSymbolExternal
} MMIX_SYMBOL_TYPE;

/**
  Symbol binding
**/
typedef enum {
  MmixBindLocal,
  MmixBindGlobal,
  MmixBindWeak
} MMIX_SYMBOL_BIND;

/**
  Relocation types
**/
typedef enum {
  MmixRelocNone,
  MmixRelocAbsolute64,
  MmixRelocAbsolute32,
  MmixRelocPcRel32,
  MmixRelocPcRel16,
  MmixRelocHigh16,
  MmixRelocLow16
} MMIX_RELOC_TYPE;

/**
  Section types
**/
typedef enum {
  MmixSectionText,
  MmixSectionData,
  MmixSectionBss,
  MmixSectionRodata
} MMIX_SECTION_TYPE;

/**
  Symbol table entry
**/
typedef struct {
  CHAR8               Name[256];
  UINT64              Value;
  UINT32              Size;
  MMIX_SYMBOL_TYPE    Type;
  MMIX_SYMBOL_BIND    Bind;
  UINT32              Section;
  BOOLEAN             Defined;
} MMIX_SYMBOL;

/**
  Relocation entry
**/
typedef struct {
  UINT64            Offset;
  MMIX_RELOC_TYPE   Type;
  UINT32            Symbol;
  INT64             Addend;
  UINT32            Section;
} MMIX_RELOCATION;

/**
  Section entry
**/
typedef struct {
  CHAR8               Name[256];
  MMIX_SECTION_TYPE   Type;
  UINT64              Address;
  UINT64              Size;
  UINT64              Alignment;
  UINT8               *Data;
  UINT32              DataCapacity;
} MMIX_SECTION;

/**
  Assembler context
**/
typedef struct {
  MMIX_SYMBOL        Symbols[MMIX_MAX_SYMBOLS];
  UINT32             SymbolCount;
  MMIX_RELOCATION    Relocations[MMIX_MAX_RELOCATIONS];
  UINT32             RelocationCount;
  MMIX_SECTION       Sections[MMIX_MAX_SECTIONS];
  UINT32             SectionCount;
  UINT32             CurrentSection;
  UINT64             CurrentAddress;
  UINT32             LineNumber;
  CHAR8              CurrentFile[256];
} MMIX_ASM_CONTEXT;

/**
  Create a new assembler context.

  @param[out]     Context       Pointer to receive context.

  @retval MMIX_SUCCESS          Context created successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmCreate (
  OUT MMIX_ASM_CONTEXT  **Context
  );

/**
  Destroy an assembler context.

  @param[in]      Context       Assembler context.

**/
VOID
MmixAsmDestroy (
  IN  MMIX_ASM_CONTEXT  *Context
  );

/**
  Assemble a source file.

  @param[in,out]  Context       Assembler context.
  @param[in]      FilePath      Path to source file.

  @retval MMIX_SUCCESS          File assembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmAssembleFile (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     CONST CHAR8       *FilePath
  );

/**
  Assemble a single line of assembly.

  @param[in,out]  Context       Assembler context.
  @param[in]      Line          Assembly line.

  @retval MMIX_SUCCESS          Line assembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmAssembleLine (
  IN OUT MMIX_ASM_CONTEXT  *Context,
  IN     CONST CHAR8       *Line
  );

/**
  Write object file.

  @param[in]      Context       Assembler context.
  @param[in]      FilePath      Output file path.

  @retval MMIX_SUCCESS          Object file written successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmWriteObject (
  IN  MMIX_ASM_CONTEXT  *Context,
  IN  CONST CHAR8       *FilePath
  );

/**
  Resolve all symbols and relocations.

  @param[in,out]  Context       Assembler context.

  @retval MMIX_SUCCESS          Resolution successful.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixAsmResolve (
  IN OUT MMIX_ASM_CONTEXT  *Context
  );

#endif // MMIX_ASM_H_
