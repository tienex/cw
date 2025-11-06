/** @file
  MMIX C Compiler Token Implementation.

  This file implements token utilities for the MMIX C23 compiler.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/MmixToken.h"

/**
  Keyword table
**/
STATIC CONST KEYWORD_ENTRY  mKeywordTable[] = {
  // C89/C90 Keywords
  { "auto",          TOK_AUTO,          FALSE },
  { "break",         TOK_BREAK,         FALSE },
  { "case",          TOK_CASE,          FALSE },
  { "char",          TOK_CHAR,          FALSE },
  { "const",         TOK_CONST,         FALSE },
  { "continue",      TOK_CONTINUE,      FALSE },
  { "default",       TOK_DEFAULT,       FALSE },
  { "do",            TOK_DO,            FALSE },
  { "double",        TOK_DOUBLE,        FALSE },
  { "else",          TOK_ELSE,          FALSE },
  { "enum",          TOK_ENUM,          FALSE },
  { "extern",        TOK_EXTERN,        FALSE },
  { "float",         TOK_FLOAT,         FALSE },
  { "for",           TOK_FOR,           FALSE },
  { "goto",          TOK_GOTO,          FALSE },
  { "if",            TOK_IF,            FALSE },
  { "int",           TOK_INT,           FALSE },
  { "long",          TOK_LONG,          FALSE },
  { "register",      TOK_REGISTER,      FALSE },
  { "return",        TOK_RETURN,        FALSE },
  { "short",         TOK_SHORT,         FALSE },
  { "signed",        TOK_SIGNED,        FALSE },
  { "sizeof",        TOK_SIZEOF,        FALSE },
  { "static",        TOK_STATIC,        FALSE },
  { "struct",        TOK_STRUCT,        FALSE },
  { "switch",        TOK_SWITCH,        FALSE },
  { "typedef",       TOK_TYPEDEF,       FALSE },
  { "union",         TOK_UNION,         FALSE },
  { "unsigned",      TOK_UNSIGNED,      FALSE },
  { "void",          TOK_VOID,          FALSE },
  { "volatile",      TOK_VOLATILE,      FALSE },
  { "while",         TOK_WHILE,         FALSE },
  { "inline",        TOK_INLINE,        FALSE },
  { "restrict",      TOK_RESTRICT,      FALSE },

  // C99 Keywords
  { "_Bool",         TOK_BOOL,          FALSE },
  { "_Complex",      TOK_COMPLEX,       FALSE },
  { "_Imaginary",    TOK_IMAGINARY,     FALSE },

  // C11 Keywords
  { "_Alignas",      TOK_ALIGNAS,       FALSE },
  { "_Alignof",      TOK_ALIGNOF,       FALSE },
  { "_Atomic",       TOK_ATOMIC,        FALSE },
  { "_Generic",      TOK_GENERIC,       FALSE },
  { "_Noreturn",     TOK_NORETURN,      FALSE },
  { "_Static_assert", TOK_STATIC_ASSERT, FALSE },
  { "_Thread_local", TOK_THREAD_LOCAL,  FALSE },

  // C23 Keywords
  { "typeof",        TOK_TYPEOF,        FALSE },
  { "typeof_unqual", TOK_TYPEOF_UNQUAL, FALSE },
  { "constexpr",     TOK_CONSTEXPR,     FALSE },
  { "nullptr",       TOK_NULLPTR,       FALSE },
  { "_BitInt",       TOK_BITINT,        FALSE },

  // GNU Extensions
  { "asm",           TOK_ASM,           TRUE  },
  { "__asm",         TOK_ASM,           TRUE  },
  { "__asm__",       TOK_ASM,           TRUE  },
  { "__typeof",      TOK_TYPEOF_GNU,    TRUE  },
  { "__typeof__",    TOK_TYPEOF_GNU,    TRUE  },
  { "__attribute",   TOK_ATTRIBUTE,     TRUE  },
  { "__attribute__", TOK_ATTRIBUTE,     TRUE  },
  { "__extension__", TOK_EXTENSION,     TRUE  },
  { "__int128",      TOK_INT128,        TRUE  },
  { "__float128",    TOK_FLOAT128,      TRUE  },

  // MSVC Extensions
  { "__cdecl",       TOK_CDECL,         TRUE  },
  { "__stdcall",     TOK_STDCALL,       TRUE  },
  { "__fastcall",    TOK_FASTCALL,      TRUE  },
  { "__vectorcall",  TOK_VECTORCALL,    TRUE  },
  { "__thiscall",    TOK_THISCALL,      TRUE  },
  { "__declspec",    TOK_DECLSPEC,      TRUE  },
  { "__forceinline", TOK_FORCEINLINE,   TRUE  },
  { "__int8",        TOK_INT8,          TRUE  },
  { "__int16",       TOK_INT16,         TRUE  },
  { "__int32",       TOK_INT32,         TRUE  },
  { "__int64",       TOK_INT64,         TRUE  },
  { "__ptr32",       TOK_PTR32,         TRUE  },
  { "__ptr64",       TOK_PTR64,         TRUE  },
  { "__unaligned",   TOK_UNALIGNED,     TRUE  },
  { "__try",         TOK_TRY,           TRUE  },
  { "__except",      TOK_EXCEPT,        TRUE  },
  { "__finally",     TOK_FINALLY,       TRUE  },

  // MetaWare Extensions
  { "yield",         TOK_YIELD,         TRUE  },
  { "__near",        TOK_NEAR,          TRUE  },
  { "__far",         TOK_FAR,           TRUE  },
  { "__huge",        TOK_HUGE,          TRUE  },
  { "__interrupt",   TOK_INTERRUPT,     TRUE  },
  { "__reentrant",   TOK_REENTRANT,     TRUE  },
  { "__task",        TOK_TASK,          TRUE  },
  { "__port",        TOK_PORT,          TRUE  },
  { "__based",       TOK_BASED,         TRUE  },
  { "__segname",     TOK_SEGNAME,       TRUE  },
  { "__startup",     TOK_STARTUP,       TRUE  },
  { "__exit",        TOK_EXIT,          TRUE  },
  { "__pascal",      TOK_PASCAL,        TRUE  },
  { "__syscall",     TOK_SYSCALL,       TRUE  },
  { "__fortran",     TOK_FORTRAN,       TRUE  },

  // MetaWare Built-in Functions
  { "__abs",                TOK_BUILTIN_ABS,            TRUE  },
  { "__min",                TOK_BUILTIN_MIN,            TRUE  },
  { "__max",                TOK_BUILTIN_MAX,            TRUE  },
  { "__bit_count",          TOK_BUILTIN_BIT_COUNT,      TRUE  },
  { "__leading_zeros",      TOK_BUILTIN_LEADING_ZEROS,  TRUE  },
  { "__trailing_zeros",     TOK_BUILTIN_TRAILING_ZEROS, TRUE  },
  { "__rotate_left",        TOK_BUILTIN_ROTATE_LEFT,    TRUE  },
  { "__rotate_right",       TOK_BUILTIN_ROTATE_RIGHT,   TRUE  },
  { "__atomic_add",         TOK_BUILTIN_ATOMIC_ADD,     TRUE  },
  { "__atomic_swap",        TOK_BUILTIN_ATOMIC_SWAP,    TRUE  },
  { "__compare_and_swap",   TOK_BUILTIN_CAS,            TRUE  },
  { "__segment_of",         TOK_BUILTIN_SEGMENT_OF,     TRUE  },
  { "__offset_of",          TOK_BUILTIN_OFFSET_OF,      TRUE  },
  { "__make_pointer",       TOK_BUILTIN_MAKE_POINTER,   TRUE  },

  // Open Watcom Extensions
  { "__watcall",       TOK_WATCALL,          TRUE  },
  { "_Packed",         TOK_PACKED,           TRUE  },
  { "__far16",         TOK_FAR16,            TRUE  },
  { "__segment",       TOK_SEGMENT,          TRUE  },
  { "__self",          TOK_SELF,             TRUE  },
  { "__loadds",        TOK_LOADDS,           TRUE  },
  { "__saveregs",      TOK_SAVEREGS,         TRUE  },
  { "__export",        TOK_EXPORT,           TRUE  },

  { NULL,            TOK_EOF,           FALSE }
};

/**
  Check if token is a keyword.

  @param[in]      Type          Token type.

  @return  TRUE if keyword, FALSE otherwise.

**/
BOOLEAN
TokenIsKeyword (
  IN  TOKEN_TYPE  Type
  )
{
  return (Type >= TOK_AUTO && Type <= TOK_EXPORT);
}

/**
  Check if token is an operator.

  @param[in]      Type          Token type.

  @return  TRUE if operator, FALSE otherwise.

**/
BOOLEAN
TokenIsOperator (
  IN  TOKEN_TYPE  Type
  )
{
  return (Type >= TOK_PLUS && Type <= TOK_DOUBLE_COLON);
}

/**
  Check if token is a literal.

  @param[in]      Type          Token type.

  @return  TRUE if literal, FALSE otherwise.

**/
BOOLEAN
TokenIsLiteral (
  IN  TOKEN_TYPE  Type
  )
{
  return (Type == TOK_INTEGER || Type == TOK_FLOAT_LITERAL ||
          Type == TOK_CHAR_LITERAL || Type == TOK_STRING || Type == TOK_RAW_STRING);
}

/**
  Get token type name.

  @param[in]      Type          Token type.

  @return  String name of token type.

**/
CONST CHAR8 *
TokenTypeName (
  IN  TOKEN_TYPE  Type
  )
{
  switch (Type) {
    case TOK_EOF:           return "EOF";
    case TOK_ERROR:         return "ERROR";
    case TOK_IDENTIFIER:    return "IDENTIFIER";
    case TOK_INTEGER:       return "INTEGER";
    case TOK_FLOAT_LITERAL: return "FLOAT";
    case TOK_CHAR_LITERAL:  return "CHAR";
    case TOK_STRING:        return "STRING";
    case TOK_PLUS:          return "+";
    case TOK_MINUS:         return "-";
    case TOK_STAR:          return "*";
    case TOK_SLASH:         return "/";
    default:
      //
      // Search keyword table
      //
      for (UINT32 i = 0; mKeywordTable[i].Name != NULL; i++) {
        if (mKeywordTable[i].Type == Type) {
          return mKeywordTable[i].Name;
        }
      }
      return "UNKNOWN";
  }
}

/**
  Create a new token.

  @param[in]      Type          Token type.
  @param[in]      Location      Token location.
  @param[in]      Text          Token text.
  @param[in]      TextLength    Text length.

  @return  Pointer to new token, or NULL on error.

**/
TOKEN *
TokenCreate (
  IN  TOKEN_TYPE            Type,
  IN  CONST TOKEN_LOCATION  *Location,
  IN  CONST CHAR8           *Text,
  IN  UINT32                TextLength
  )
{
  TOKEN  *Token;

  Token = (TOKEN *)calloc (1, sizeof (TOKEN));
  if (Token == NULL) {
    return NULL;
  }

  Token->Type = Type;
  if (Location != NULL) {
    Token->Location = *Location;
  }

  if (Text != NULL && TextLength > 0) {
    Token->Text = (CHAR8 *)malloc (TextLength + 1);
    if (Token->Text == NULL) {
      free (Token);
      return NULL;
    }
    memcpy (Token->Text, Text, TextLength);
    Token->Text[TextLength] = '\0';
    Token->TextLength = TextLength;
  }

  return Token;
}

/**
  Destroy a token.

  @param[in]      Token         Token to destroy.

**/
VOID
TokenDestroy (
  IN  TOKEN  *Token
  )
{
  if (Token == NULL) {
    return;
  }

  if (Token->Text != NULL) {
    free (Token->Text);
  }

  if (Token->HasValue && Token->Type == TOK_STRING && Token->Value.StringValue != NULL) {
    free (Token->Value.StringValue);
  }

  free (Token);
}

/**
  Lookup keyword by name.

  @param[in]      Name          Keyword name.
  @param[in]      Length        Name length.
  @param[out]     Type          Pointer to receive token type.

  @return  TRUE if keyword found, FALSE otherwise.

**/
BOOLEAN
TokenLookupKeyword (
  IN  CONST CHAR8  *Name,
  IN  UINT32       Length,
  OUT TOKEN_TYPE   *Type
  )
{
  for (UINT32 i = 0; mKeywordTable[i].Name != NULL; i++) {
    if (strlen (mKeywordTable[i].Name) == Length &&
        strncmp (mKeywordTable[i].Name, Name, Length) == 0) {
      *Type = mKeywordTable[i].Type;
      return TRUE;
    }
  }

  return FALSE;
}
