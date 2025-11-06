/** @file
  MMIX C Compiler Lexer Implementation.

  This file implements the lexer for tokenizing C23 source code.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/compiler/MmixLexer.h"

//
// Forward declarations
//
BOOLEAN
TokenLookupKeyword (
  IN  CONST CHAR8  *Name,
  IN  UINT32       Length,
  OUT TOKEN_TYPE   *Type
  );

/**
  Peek at character at offset.

  @param[in]      Lexer         Lexer state.
  @param[in]      Offset        Offset from current position.

  @return  Character at offset, or '\0' if past end.

**/
STATIC
CHAR8
LexerPeekChar (
  IN  LEXER_STATE  *Lexer,
  IN  INT32        Offset
  )
{
  UINT32  Pos = Lexer->Position + Offset;
  if (Pos >= Lexer->SourceLength) {
    return '\0';
  }
  return Lexer->Source[Pos];
}

/**
  Advance to next character.

  @param[in,out]  Lexer         Lexer state.

**/
STATIC
VOID
LexerAdvance (
  IN OUT LEXER_STATE  *Lexer
  )
{
  if (Lexer->Position >= Lexer->SourceLength) {
    Lexer->AtEof = TRUE;
    Lexer->CurrentChar = '\0';
    return;
  }

  if (Lexer->CurrentChar == '\n') {
    Lexer->Line++;
    Lexer->Column = 1;
  } else if (Lexer->CurrentChar == '\t') {
    Lexer->Column += Lexer->Options.TabSize;
  } else {
    Lexer->Column++;
  }

  Lexer->Position++;
  if (Lexer->Position < Lexer->SourceLength) {
    Lexer->CurrentChar = Lexer->Source[Lexer->Position];
  } else {
    Lexer->CurrentChar = '\0';
    Lexer->AtEof = TRUE;
  }
}

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
  )
{
  LEXER_STATE  *Lexer;

  if (Source == NULL) {
    return NULL;
  }

  Lexer = (LEXER_STATE *)calloc (1, sizeof (LEXER_STATE));
  if (Lexer == NULL) {
    return NULL;
  }

  Lexer->Source = Source;
  Lexer->SourceLength = SourceLength;
  Lexer->FileName = FileName ? FileName : "<input>";
  Lexer->Position = 0;
  Lexer->Line = 1;
  Lexer->Column = 1;
  Lexer->CurrentChar = (SourceLength > 0) ? Source[0] : '\0';
  Lexer->AtEof = (SourceLength == 0);

  //
  // Set options
  //
  if (Options != NULL) {
    Lexer->Options = *Options;
  } else {
    //
    // Default options
    //
    Lexer->Options.EnableGnuExtensions = TRUE;
    Lexer->Options.EnableMsvcExtensions = TRUE;
    Lexer->Options.EnableClangExtensions = TRUE;
    Lexer->Options.EnableMetawareExtensions = TRUE;
    Lexer->Options.TabSize = 8;
  }

  return Lexer;
}

/**
  Destroy a lexer state.

  @param[in]      Lexer         Lexer state.

**/
VOID
LexerDestroy (
  IN  LEXER_STATE  *Lexer
  )
{
  if (Lexer == NULL) {
    return;
  }

  //
  // Note: We don't destroy CurrentToken here because it's owned by the Parser
  // The Parser is responsible for destroying all tokens it receives
  //

  free (Lexer);
}

/**
  Get current location.

  @param[in]      Lexer         Lexer state.
  @param[out]     Location      Pointer to receive location.

**/
VOID
LexerGetLocation (
  IN  LEXER_STATE     *Lexer,
  OUT TOKEN_LOCATION  *Location
  )
{
  if (Lexer == NULL || Location == NULL) {
    return;
  }

  Location->FileName = Lexer->FileName;
  Location->Line = Lexer->Line;
  Location->Column = Lexer->Column;
  Location->Offset = Lexer->Position;
}

/**
  Report a lexer error.

  @param[in,out]  Lexer         Lexer state.
  @param[in]      Message       Error message.

**/
VOID
LexerError (
  IN OUT LEXER_STATE  *Lexer,
  IN     CONST CHAR8  *Message
  )
{
  if (Lexer == NULL || Message == NULL) {
    return;
  }

  snprintf (Lexer->ErrorMessage, sizeof (Lexer->ErrorMessage),
           "%s:%u:%u: error: %s",
           Lexer->FileName, Lexer->Line, Lexer->Column, Message);
  Lexer->ErrorCount++;
}

/**
  Skip whitespace and comments.

  @param[in,out]  Lexer         Lexer state.

**/
VOID
LexerSkipWhitespace (
  IN OUT LEXER_STATE  *Lexer
  )
{
  while (!Lexer->AtEof) {
    //
    // Skip whitespace
    //
    if (isspace (Lexer->CurrentChar)) {
      LexerAdvance (Lexer);
      continue;
    }

    //
    // Skip C++ style comments
    //
    if (Lexer->CurrentChar == '/' && LexerPeekChar (Lexer, 1) == '/') {
      while (!Lexer->AtEof && Lexer->CurrentChar != '\n') {
        LexerAdvance (Lexer);
      }
      continue;
    }

    //
    // Skip C style comments
    //
    if (Lexer->CurrentChar == '/' && LexerPeekChar (Lexer, 1) == '*') {
      LexerAdvance (Lexer);  // Skip '/'
      LexerAdvance (Lexer);  // Skip '*'

      while (!Lexer->AtEof) {
        if (Lexer->CurrentChar == '*' && LexerPeekChar (Lexer, 1) == '/') {
          LexerAdvance (Lexer);  // Skip '*'
          LexerAdvance (Lexer);  // Skip '/'
          break;
        }
        LexerAdvance (Lexer);
      }
      continue;
    }

    break;
  }
}

/**
  Lex identifier or keyword.

  @param[in,out]  Lexer         Lexer state.

  @return  Pointer to token.

**/
STATIC
TOKEN *
LexIdentifier (
  IN OUT LEXER_STATE  *Lexer
  )
{
  TOKEN_LOCATION  Location;
  UINT32          Start;
  UINT32          Length;
  TOKEN_TYPE      Type;
  TOKEN           *Token;

  LexerGetLocation (Lexer, &Location);
  Start = Lexer->Position;

  //
  // Read identifier characters
  //
  while (!Lexer->AtEof &&
         (isalnum (Lexer->CurrentChar) ||
          Lexer->CurrentChar == '_' ||
          (Lexer->Options.EnableDollarInIdentifiers && Lexer->CurrentChar == '$'))) {
    LexerAdvance (Lexer);
  }

  Length = Lexer->Position - Start;

  //
  // Check if it's a keyword
  //
  if (TokenLookupKeyword (&Lexer->Source[Start], Length, &Type)) {
    Token = TokenCreate (Type, &Location, &Lexer->Source[Start], Length);
  } else {
    Token = TokenCreate (TOK_IDENTIFIER, &Location, &Lexer->Source[Start], Length);
  }

  return Token;
}

/**
  Lex numeric literal.

  @param[in,out]  Lexer         Lexer state.

  @return  Pointer to token.

**/
STATIC
TOKEN *
LexNumber (
  IN OUT LEXER_STATE  *Lexer
  )
{
  TOKEN_LOCATION  Location;
  UINT32          Start;
  UINT32          Length;
  TOKEN           *Token;
  BOOLEAN         IsFloat = FALSE;

  LexerGetLocation (Lexer, &Location);
  Start = Lexer->Position;

  //
  // Check for binary (0b) or hex (0x) prefix
  //
  if (Lexer->CurrentChar == '0') {
    LexerAdvance (Lexer);

    if (Lexer->CurrentChar == 'x' || Lexer->CurrentChar == 'X') {
      //
      // Hexadecimal
      //
      LexerAdvance (Lexer);
      while (!Lexer->AtEof &&
             (isxdigit (Lexer->CurrentChar) || Lexer->CurrentChar == '_')) {
        if (Lexer->CurrentChar != '_') {
          // Process digit
        }
        LexerAdvance (Lexer);
      }
    } else if (Lexer->CurrentChar == 'b' || Lexer->CurrentChar == 'B') {
      //
      // Binary
      //
      LexerAdvance (Lexer);
      while (!Lexer->AtEof &&
             (Lexer->CurrentChar == '0' || Lexer->CurrentChar == '1' || Lexer->CurrentChar == '_')) {
        LexerAdvance (Lexer);
      }
    }
  }

  //
  // Read decimal digits (with separators)
  //
  while (!Lexer->AtEof && (isdigit (Lexer->CurrentChar) || Lexer->CurrentChar == '_')) {
    LexerAdvance (Lexer);
  }

  //
  // Check for decimal point
  //
  if (Lexer->CurrentChar == '.' && isdigit (LexerPeekChar (Lexer, 1))) {
    IsFloat = TRUE;
    LexerAdvance (Lexer);
    while (!Lexer->AtEof && (isdigit (Lexer->CurrentChar) || Lexer->CurrentChar == '_')) {
      LexerAdvance (Lexer);
    }
  }

  //
  // Check for exponent
  //
  if (Lexer->CurrentChar == 'e' || Lexer->CurrentChar == 'E' ||
      Lexer->CurrentChar == 'p' || Lexer->CurrentChar == 'P') {
    IsFloat = TRUE;
    LexerAdvance (Lexer);
    if (Lexer->CurrentChar == '+' || Lexer->CurrentChar == '-') {
      LexerAdvance (Lexer);
    }
    while (!Lexer->AtEof && (isdigit (Lexer->CurrentChar) || Lexer->CurrentChar == '_')) {
      LexerAdvance (Lexer);
    }
  }

  //
  // Read suffix (f, l, u, etc.)
  //
  while (!Lexer->AtEof && (isalpha (Lexer->CurrentChar))) {
    LexerAdvance (Lexer);
  }

  Length = Lexer->Position - Start;
  Token = TokenCreate (IsFloat ? TOK_FLOAT_LITERAL : TOK_INTEGER,
                      &Location, &Lexer->Source[Start], Length);

  return Token;
}

/**
  Lex string literal.

  @param[in,out]  Lexer         Lexer state.

  @return  Pointer to token.

**/
STATIC
TOKEN *
LexString (
  IN OUT LEXER_STATE  *Lexer
  )
{
  TOKEN_LOCATION  Location;
  UINT32          Start;
  UINT32          Length;
  TOKEN           *Token;
  CHAR8           Quote;
  BOOLEAN         IsRaw = FALSE;

  LexerGetLocation (Lexer, &Location);

  //
  // Check for raw string (MetaWare extension)
  //
  if (Lexer->CurrentChar == 'r' && LexerPeekChar (Lexer, 1) == '"') {
    IsRaw = TRUE;
    LexerAdvance (Lexer);  // Skip 'r'
  }

  Start = Lexer->Position;
  Quote = Lexer->CurrentChar;
  LexerAdvance (Lexer);  // Skip opening quote

  //
  // Read string content
  //
  while (!Lexer->AtEof && Lexer->CurrentChar != Quote) {
    if (!IsRaw && Lexer->CurrentChar == '\\') {
      LexerAdvance (Lexer);  // Skip escape
      if (!Lexer->AtEof) {
        LexerAdvance (Lexer);  // Skip escaped character
      }
    } else {
      LexerAdvance (Lexer);
    }
  }

  if (Lexer->CurrentChar == Quote) {
    LexerAdvance (Lexer);  // Skip closing quote
  } else {
    LexerError (Lexer, "Unterminated string literal");
  }

  Length = Lexer->Position - Start;
  Token = TokenCreate (IsRaw ? TOK_RAW_STRING : TOK_STRING,
                      &Location, &Lexer->Source[Start], Length);

  return Token;
}

/**
  Get the next token from the lexer.

  @param[in,out]  Lexer         Lexer state.

  @return  Pointer to token, or NULL on error.

**/
TOKEN *
LexerNextToken (
  IN OUT LEXER_STATE  *Lexer
  )
{
  TOKEN_LOCATION  Location;
  TOKEN           *Token;

  if (Lexer == NULL) {
    return NULL;
  }

  //
  // Note: We don't destroy the previous token here because the caller (Parser)
  // owns it and is responsible for destroying it.
  //

  //
  // Skip whitespace
  //
  LexerSkipWhitespace (Lexer);

  if (Lexer->AtEof) {
    LexerGetLocation (Lexer, &Location);
    Lexer->CurrentToken = TokenCreate (TOK_EOF, &Location, NULL, 0);
    return Lexer->CurrentToken;
  }

  LexerGetLocation (Lexer, &Location);

  //
  // Identifier or keyword
  //
  if (isalpha (Lexer->CurrentChar) || Lexer->CurrentChar == '_') {
    Lexer->CurrentToken = LexIdentifier (Lexer);
    return Lexer->CurrentToken;
  }

  //
  // Number
  //
  if (isdigit (Lexer->CurrentChar)) {
    Lexer->CurrentToken = LexNumber (Lexer);
    return Lexer->CurrentToken;
  }

  //
  // String literal
  //
  if (Lexer->CurrentChar == '"' || Lexer->CurrentChar == '\'' ||
      (Lexer->CurrentChar == 'r' && LexerPeekChar (Lexer, 1) == '"')) {
    Lexer->CurrentToken = LexString (Lexer);
    return Lexer->CurrentToken;
  }

  //
  // Single character operators
  //
  CHAR8  Ch = Lexer->CurrentChar;
  LexerAdvance (Lexer);

  switch (Ch) {
    case '+':
      if (Lexer->CurrentChar == '+') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_PLUS_PLUS, &Location, "++", 2);
      } else if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_PLUS_EQUAL, &Location, "+=", 2);
      } else {
        Token = TokenCreate (TOK_PLUS, &Location, "+", 1);
      }
      break;

    case '-':
      if (Lexer->CurrentChar == '-') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_MINUS_MINUS, &Location, "--", 2);
      } else if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_MINUS_EQUAL, &Location, "-=", 2);
      } else if (Lexer->CurrentChar == '>') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_ARROW, &Location, "->", 2);
      } else {
        Token = TokenCreate (TOK_MINUS, &Location, "-", 1);
      }
      break;

    case '(':
      Token = TokenCreate (TOK_LPAREN, &Location, "(", 1);
      break;

    case ')':
      Token = TokenCreate (TOK_RPAREN, &Location, ")", 1);
      break;

    case '{':
      Token = TokenCreate (TOK_LBRACE, &Location, "{", 1);
      break;

    case '}':
      Token = TokenCreate (TOK_RBRACE, &Location, "}", 1);
      break;

    case ';':
      Token = TokenCreate (TOK_SEMICOLON, &Location, ";", 1);
      break;

    case ',':
      Token = TokenCreate (TOK_COMMA, &Location, ",", 1);
      break;

    case '*':
      if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_STAR_EQUAL, &Location, "*=", 2);
      } else {
        Token = TokenCreate (TOK_STAR, &Location, "*", 1);
      }
      break;

    case '/':
      if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_SLASH_EQUAL, &Location, "/=", 2);
      } else {
        Token = TokenCreate (TOK_SLASH, &Location, "/", 1);
      }
      break;

    case '%':
      if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_PERCENT_EQUAL, &Location, "%=", 2);
      } else {
        Token = TokenCreate (TOK_PERCENT, &Location, "%", 1);
      }
      break;

    case '<':
      if (Lexer->CurrentChar == '<') {
        LexerAdvance (Lexer);
        if (Lexer->CurrentChar == '=') {
          LexerAdvance (Lexer);
          Token = TokenCreate (TOK_SHIFT_LEFT_EQUAL, &Location, "<<=", 3);
        } else {
          Token = TokenCreate (TOK_SHIFT_LEFT, &Location, "<<", 2);
        }
      } else if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_LESS_EQUAL, &Location, "<=", 2);
      } else {
        Token = TokenCreate (TOK_LESS, &Location, "<", 1);
      }
      break;

    case '>':
      if (Lexer->CurrentChar == '>') {
        LexerAdvance (Lexer);
        if (Lexer->CurrentChar == '=') {
          LexerAdvance (Lexer);
          Token = TokenCreate (TOK_SHIFT_RIGHT_EQUAL, &Location, ">>=", 3);
        } else {
          Token = TokenCreate (TOK_SHIFT_RIGHT, &Location, ">>", 2);
        }
      } else if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_GREATER_EQUAL, &Location, ">=", 2);
      } else {
        Token = TokenCreate (TOK_GREATER, &Location, ">", 1);
      }
      break;

    case '=':
      if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_EQUAL_EQUAL, &Location, "==", 2);
      } else {
        Token = TokenCreate (TOK_EQUAL, &Location, "=", 1);
      }
      break;

    case '!':
      if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_NOT_EQUAL, &Location, "!=", 2);
      } else {
        Token = TokenCreate (TOK_EXCLAIM, &Location, "!", 1);
      }
      break;

    case '&':
      if (Lexer->CurrentChar == '&') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_LOGICAL_AND, &Location, "&&", 2);
      } else if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_AMPERSAND_EQUAL, &Location, "&=", 2);
      } else {
        Token = TokenCreate (TOK_AMPERSAND, &Location, "&", 1);
      }
      break;

    case '|':
      if (Lexer->CurrentChar == '|') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_LOGICAL_OR, &Location, "||", 2);
      } else if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_PIPE_EQUAL, &Location, "|=", 2);
      } else {
        Token = TokenCreate (TOK_PIPE, &Location, "|", 1);
      }
      break;

    case '^':
      if (Lexer->CurrentChar == '=') {
        LexerAdvance (Lexer);
        Token = TokenCreate (TOK_CARET_EQUAL, &Location, "^=", 2);
      } else {
        Token = TokenCreate (TOK_CARET, &Location, "^", 1);
      }
      break;

    case '~':
      Token = TokenCreate (TOK_TILDE, &Location, "~", 1);
      break;

    case '?':
      Token = TokenCreate (TOK_QUESTION, &Location, "?", 1);
      break;

    case ':':
      Token = TokenCreate (TOK_COLON, &Location, ":", 1);
      break;

    case '.':
      if (Lexer->CurrentChar == '.') {
        if (LexerPeekChar (Lexer, 1) == '.') {
          // ... (ellipsis)
          LexerAdvance (Lexer);
          LexerAdvance (Lexer);
          Token = TokenCreate (TOK_ELLIPSIS, &Location, "...", 3);
        } else {
          // .. (bit concatenation)
          LexerAdvance (Lexer);
          Token = TokenCreate (TOK_DOT_DOT, &Location, "..", 2);
        }
      } else {
        Token = TokenCreate (TOK_DOT, &Location, ".", 1);
      }
      break;

    case '[':
      Token = TokenCreate (TOK_LBRACKET, &Location, "[", 1);
      break;

    case ']':
      Token = TokenCreate (TOK_RBRACKET, &Location, "]", 1);
      break;

    default:
      Token = TokenCreate (TOK_ERROR, &Location, &Ch, 1);
      LexerError (Lexer, "Unexpected character");
      break;
  }

  Lexer->CurrentToken = Token;
  return Token;
}

/**
  Peek at the next token without consuming it.

  @param[in]      Lexer         Lexer state.

  @return  Pointer to token, or NULL on error.

**/
TOKEN *
LexerPeekToken (
  IN  LEXER_STATE  *Lexer
  )
{
  if (Lexer == NULL) {
    return NULL;
  }

  //
  // If we don't have a current token, get one
  //
  if (Lexer->CurrentToken == NULL) {
    return LexerNextToken (Lexer);
  }

  return Lexer->CurrentToken;
}
