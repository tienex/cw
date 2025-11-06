/** @file
  MMIX C Compiler Lexer Interface.

  This file defines the lexer interface for tokenizing C23 source code
  with support for all compiler extensions.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_LEXER_H_
#define MMIX_LEXER_H_

#include "../MmixTypes.h"
#include "MmixToken.h"

#define LEXER_MAX_TOKEN_LENGTH  4096

/**
  Lexer options
**/
typedef struct {
  BOOLEAN  EnableGnuExtensions;       // Enable GNU extensions
  BOOLEAN  EnableMsvcExtensions;      // Enable MSVC extensions
  BOOLEAN  EnableClangExtensions;     // Enable Clang extensions
  BOOLEAN  EnableMetawareExtensions;  // Enable MetaWare extensions
  BOOLEAN  EnableDigraphs;            // Enable digraphs (<%, %>, etc.)
  BOOLEAN  EnableTrigraphs;           // Enable trigraphs (??=, etc.)
  BOOLEAN  EnableDollarInIdentifiers; // Allow $ in identifiers
  BOOLEAN  PreserveWhitespace;        // Track whitespace for formatting
  BOOLEAN  PreserveComments;          // Preserve comments as tokens
  UINT32   TabSize;                   // Tab size for column tracking
} LEXER_OPTIONS;

/**
  Lexer state
**/
typedef struct {
  CONST CHAR8      *Source;           // Source code
  UINT32           SourceLength;      // Source length
  CONST CHAR8      *FileName;         // Source file name
  UINT32           Position;          // Current position
  UINT32           Line;              // Current line (1-based)
  UINT32           Column;            // Current column (1-based)
  LEXER_OPTIONS    Options;           // Lexer options
  CHAR8            CurrentChar;       // Current character
  BOOLEAN          AtEof;             // At end of file
  TOKEN            *CurrentToken;     // Current token
  UINT32           ErrorCount;        // Number of errors
  CHAR8            ErrorMessage[256]; // Last error message
} LEXER_STATE;

/**
  Create a new lexer state.

  @param[in]      Source        Source code.
  @param[in]      SourceLength  Source length.
  @param[in]      FileName      Source file name.
  @param[in]      Options       Lexer options (or NULL for defaults).

  @return  Pointer to lexer state, or NULL on error.

**/
LEXER_STATE *
LexerCreate (
  IN  CONST CHAR8          *Source,
  IN  UINT32               SourceLength,
  IN  CONST CHAR8          *FileName,
  IN  CONST LEXER_OPTIONS  *Options OPTIONAL
  );

/**
  Destroy a lexer state.

  @param[in]      Lexer         Lexer state.

**/
VOID
LexerDestroy (
  IN  LEXER_STATE  *Lexer
  );

/**
  Get the next token from the lexer.

  @param[in,out]  Lexer         Lexer state.

  @return  Pointer to token, or NULL on error.

**/
TOKEN *
LexerNextToken (
  IN OUT LEXER_STATE  *Lexer
  );

/**
  Peek at the next token without consuming it.

  @param[in]      Lexer         Lexer state.

  @return  Pointer to token, or NULL on error.

**/
TOKEN *
LexerPeekToken (
  IN  LEXER_STATE  *Lexer
  );

/**
  Skip whitespace and comments.

  @param[in,out]  Lexer         Lexer state.

**/
VOID
LexerSkipWhitespace (
  IN OUT LEXER_STATE  *Lexer
  );

/**
  Report a lexer error.

  @param[in,out]  Lexer         Lexer state.
  @param[in]      Message       Error message.

**/
VOID
LexerError (
  IN OUT LEXER_STATE  *Lexer,
  IN     CONST CHAR8  *Message
  );

/**
  Get current location.

  @param[in]      Lexer         Lexer state.
  @param[out]     Location      Pointer to receive location.

**/
VOID
LexerGetLocation (
  IN  LEXER_STATE     *Lexer,
  OUT TOKEN_LOCATION  *Location
  );

#endif // MMIX_LEXER_H_
