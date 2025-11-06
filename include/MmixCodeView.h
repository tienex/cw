/** @file
  CodeView debug information format support.

  This file provides structures and functions for generating CodeView
  debug information in MMIX object files and executables. CodeView is
  Microsoft's native debug format, used in PDB files.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_CODEVIEW_H_
#define MMIX_CODEVIEW_H_

#include "MmixTypes.h"

//
// CodeView signature
//
#define CV_SIGNATURE_C7         0x00000001  // C7 format (Visual C++ 1.0+)
#define CV_SIGNATURE_C11        0x00000002  // C11 format (Visual C++ 5.0+)
#define CV_SIGNATURE_C13        0x00000004  // C13 format (Visual C++ 7.0+)

//
// Symbol record types
//
#define S_END                   0x0006  // End of block
#define S_SKIP                  0x0007  // Skip record
#define S_CVRESERVE             0x0008  // Reserved
#define S_OBJNAME               0x0009  // Object file name
#define S_COMPILE               0x0001  // Compile flags
#define S_REGISTER              0x1001  // Register variable
#define S_CONSTANT              0x1002  // Constant symbol
#define S_UDT                   0x1003  // User-defined type
#define S_SSEARCH               0x1005  // Start search
#define S_COBOLUDT              0x1007  // COBOL user-defined type
#define S_MANYREG               0x1008  // Multiple register variable
#define S_BPREL32               0x1009  // BP-relative
#define S_LDATA32               0x100A  // Local data
#define S_GDATA32               0x100B  // Global data
#define S_PUB32                 0x100C  // Public symbol
#define S_LPROC32               0x100D  // Local procedure
#define S_GPROC32               0x100E  // Global procedure
#define S_REGREL32              0x1111  // Register relative
#define S_LTHREAD32             0x1112  // Local thread storage
#define S_GTHREAD32             0x1113  // Global thread storage

//
// Type record leaf types
//
#define LF_MODIFIER             0x0001  // Type modifier
#define LF_POINTER              0x0002  // Pointer
#define LF_ARRAY                0x0003  // Array
#define LF_CLASS                0x0004  // Class
#define LF_STRUCTURE            0x0005  // Structure
#define LF_UNION                0x0006  // Union
#define LF_ENUM                 0x0007  // Enumeration
#define LF_PROCEDURE            0x0008  // Procedure
#define LF_MFUNCTION            0x0009  // Member function
#define LF_VTSHAPE              0x000A  // Virtual table shape
#define LF_COBOL0               0x000B  // COBOL type
#define LF_COBOL1               0x000C  // COBOL type
#define LF_BARRAY               0x000D  // Basic array
#define LF_LABEL                0x000E  // Label
#define LF_NULL                 0x000F  // Null
#define LF_NOTTRAN              0x0010  // Not translated
#define LF_DIMARRAY             0x0011  // Dimensioned array
#define LF_VFTPATH              0x0012  // Virtual function table path
#define LF_PRECOMP              0x0013  // Precompiled types
#define LF_ENDPRECOMP           0x0014  // End precompiled types
#define LF_OEM                  0x0015  // OEM definable type

//
// Primitive types
//
#define T_VOID                  0x0003  // Void
#define T_CHAR                  0x0010  // 8-bit signed char
#define T_UCHAR                 0x0020  // 8-bit unsigned char
#define T_SHORT                 0x0011  // 16-bit signed short
#define T_USHORT                0x0021  // 16-bit unsigned short
#define T_INT4                  0x0074  // 32-bit signed int
#define T_UINT4                 0x0075  // 32-bit unsigned int
#define T_INT8                  0x0076  // 64-bit signed int
#define T_UINT8                 0x0077  // 64-bit unsigned int
#define T_REAL32                0x0040  // 32-bit float
#define T_REAL64                0x0041  // 64-bit double
#define T_REAL80                0x0042  // 80-bit long double

//
// Debug subsection types
//
#define DEBUG_S_SYMBOLS         0xF1    // Symbols
#define DEBUG_S_LINES           0xF2    // Line numbers
#define DEBUG_S_STRINGTABLE     0xF3    // String table
#define DEBUG_S_FILECHKSMS      0xF4    // File checksums
#define DEBUG_S_FRAMEDATA       0xF5    // Frame data

//
// Maximum sizes
//
#define CV_MAX_SYMBOLS          8192
#define CV_MAX_TYPES            4096
#define CV_MAX_LINES            16384
#define CV_MAX_FILES            256

/**
  CodeView symbol record header
**/
typedef struct {
  UINT16  Length;   // Record length (not including this field)
  UINT16  Type;     // Record type (S_*)
} CV_SYMBOL_HEADER;

/**
  CodeView type record header
**/
typedef struct {
  UINT16  Length;   // Record length (not including this field)
  UINT16  Leaf;     // Leaf type (LF_*)
} CV_TYPE_HEADER;

/**
  CodeView compile flags symbol (S_COMPILE)
**/
typedef struct {
  CV_SYMBOL_HEADER  Header;
  UINT8             Machine;        // Target machine
  UINT8             Language;       // Source language
  UINT16            Flags;          // Compile flags
  CHAR8             Version[1];     // Compiler version string (variable length)
} CV_COMPILE_SYMBOL;

/**
  CodeView public symbol (S_PUB32)
**/
typedef struct {
  CV_SYMBOL_HEADER  Header;
  UINT32            Offset;         // Symbol offset
  UINT16            Segment;        // Symbol segment
  UINT16            Type;           // Symbol type index
  CHAR8             Name[1];        // Symbol name (variable length)
} CV_PUBLIC_SYMBOL;

/**
  CodeView procedure symbol (S_GPROC32/S_LPROC32)
**/
typedef struct {
  CV_SYMBOL_HEADER  Header;
  UINT32            Parent;         // Parent scope
  UINT32            End;            // End of scope
  UINT32            Next;           // Next symbol
  UINT32            Length;         // Procedure length
  UINT32            DebugStart;     // Debug start offset
  UINT32            DebugEnd;       // Debug end offset
  UINT32            Type;           // Procedure type index
  UINT32            Offset;         // Offset in segment
  UINT16            Segment;        // Segment
  UINT8             Flags;          // Procedure flags
  CHAR8             Name[1];        // Procedure name (variable length)
} CV_PROC_SYMBOL;

/**
  CodeView data symbol (S_GDATA32/S_LDATA32)
**/
typedef struct {
  CV_SYMBOL_HEADER  Header;
  UINT32            Type;           // Data type index
  UINT32            Offset;         // Offset in segment
  UINT16            Segment;        // Segment
  CHAR8             Name[1];        // Data name (variable length)
} CV_DATA_SYMBOL;

/**
  CodeView line number subsection header
**/
typedef struct {
  UINT32  Offset;           // Offset of code
  UINT16  Segment;          // Segment of code
  UINT16  Flags;            // Flags
  UINT32  Length;           // Length of code
} CV_LINES_HEADER;

/**
  CodeView file checksum entry
**/
typedef struct {
  UINT32  NameOffset;       // Offset in string table
  UINT8   ChecksumSize;     // Size of checksum
  UINT8   ChecksumKind;     // Checksum algorithm (0=none, 1=MD5, 2=SHA1)
  UINT8   Checksum[1];      // Checksum data (variable length)
} CV_FILE_CHECKSUM;

/**
  CodeView line number entry
**/
typedef struct {
  UINT32  Offset;           // Offset from start of function
  UINT32  LineStart : 24;   // Starting line number
  UINT32  DeltaEnd : 7;     // Line count
  UINT32  Statement : 1;    // Statement flag
} CV_LINE_ENTRY;

/**
  CodeView debug context
**/
typedef struct {
  CV_SYMBOL_HEADER  *Symbols[CV_MAX_SYMBOLS];
  UINT32            SymbolCount;
  CV_TYPE_HEADER    *Types[CV_MAX_TYPES];
  UINT32            TypeCount;
  CV_LINE_ENTRY     *Lines[CV_MAX_LINES];
  UINT32            LineCount;
  CHAR8             *Files[CV_MAX_FILES];
  UINT32            FileCount;
  UINT8             *StringTable;
  UINT32            StringTableSize;
} CV_DEBUG_CONTEXT;

/**
  Create CodeView debug context.

  @param[out]     Context       Pointer to receive context.

  @retval MMIX_SUCCESS          Context created.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixCodeViewCreate (
  OUT CV_DEBUG_CONTEXT  **Context
  );

/**
  Destroy CodeView debug context.

  @param[in]      Context       Debug context.

**/
VOID
MmixCodeViewDestroy (
  IN  CV_DEBUG_CONTEXT  *Context
  );

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
  );

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
  );

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
  );

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
  );

#endif // MMIX_CODEVIEW_H_
