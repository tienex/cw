/** @file
  MMIX C Compiler Parser Interface.

  This file defines the parser interface for building AST from tokens.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_PARSER_H_
#define MMIX_PARSER_H_

#include "../MmixTypes.h"
#include "MmixLexer.h"
#include "MmixAst.h"

/**
  Parser state
**/
typedef struct {
  LEXER_STATE           *Lexer;
  TOKEN                 *CurrentToken;
  TOKEN                 *PeekToken;
  UINT32                ErrorCount;
  CHAR8                 ErrorMessage[256];
  AST_TRANSLATION_UNIT  *TranslationUnit;
} PARSER_STATE;

/**
  Create a new parser.

  @param[in]      Lexer         Lexer state.

  @return  Pointer to parser state, or NULL on error.

**/
PARSER_STATE *
ParserCreate (
  IN  LEXER_STATE  *Lexer
  );

/**
  Destroy a parser.

  @param[in]      Parser        Parser state.

**/
VOID
ParserDestroy (
  IN  PARSER_STATE  *Parser
  );

/**
  Parse a translation unit (entire file).

  @param[in,out]  Parser        Parser state.

  @return  Pointer to translation unit AST, or NULL on error.

**/
AST_TRANSLATION_UNIT *
ParserParseTranslationUnit (
  IN OUT PARSER_STATE  *Parser
  );

/**
  Parse an expression.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to expression node, or NULL on error.

**/
AST_EXPR *
ParserParseExpression (
  IN OUT PARSER_STATE  *Parser
  );

/**
  Parse a statement.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to statement node, or NULL on error.

**/
AST_STMT *
ParserParseStatement (
  IN OUT PARSER_STATE  *Parser
  );

/**
  Parse a declaration.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to declaration node, or NULL on error.

**/
AST_DECL *
ParserParseDeclaration (
  IN OUT PARSER_STATE  *Parser
  );

/**
  Parse a type specifier.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to type node, or NULL on error.

**/
AST_TYPE *
ParserParseType (
  IN OUT PARSER_STATE  *Parser
  );

/**
  Report a parser error.

  @param[in,out]  Parser        Parser state.
  @param[in]      Message       Error message.

**/
VOID
ParserError (
  IN OUT PARSER_STATE  *Parser,
  IN     CONST CHAR8   *Message
  );

/**
  Check if current token matches expected type.

  @param[in]      Parser        Parser state.
  @param[in]      Type          Expected token type.

  @return  TRUE if matches, FALSE otherwise.

**/
BOOLEAN
ParserExpect (
  IN  PARSER_STATE  *Parser,
  IN  TOKEN_TYPE    Type
  );

/**
  Consume current token if it matches expected type.

  @param[in,out]  Parser        Parser state.
  @param[in]      Type          Expected token type.

  @return  TRUE if consumed, FALSE otherwise.

**/
BOOLEAN
ParserConsume (
  IN OUT PARSER_STATE  *Parser,
  IN     TOKEN_TYPE    Type
  );

#endif // MMIX_PARSER_H_
