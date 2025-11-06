/** @file
  MMIX C Compiler Token Definitions.

  This file defines the token types and structures for the MMIX C23 compiler
  with support for GNU, MSVC, Clang, and MetaWare extensions.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_TOKEN_H_
#define MMIX_TOKEN_H_

#include "../MmixTypes.h"

/**
  Token types
**/
typedef enum {
  //
  // Special tokens
  //
  TOK_EOF,              // End of file
  TOK_ERROR,            // Lexer error
  TOK_NEWLINE,          // Newline (for preprocessor)

  //
  // Literals
  //
  TOK_INTEGER,          // Integer literal (123, 0x456, 0b1010, 1_000_000)
  TOK_FLOAT_LITERAL,    // Float literal (3.14, 1.0e-5, 0x1.2p3)
  TOK_CHAR_LITERAL,     // Character literal ('a', L'b', u'c', U'd')
  TOK_STRING,           // String literal ("hello", L"wide", u8"utf8")
  TOK_RAW_STRING,       // Raw string (MetaWare: r"C:\path")

  //
  // Identifiers and keywords
  //
  TOK_IDENTIFIER,       // Identifier (variable/function names)

  //
  // C23 Keywords
  //
  TOK_AUTO,             // auto
  TOK_BREAK,            // break
  TOK_CASE,             // case
  TOK_CHAR,             // char
  TOK_CONST,            // const
  TOK_CONTINUE,         // continue
  TOK_DEFAULT,          // default
  TOK_DO,               // do
  TOK_DOUBLE,           // double
  TOK_ELSE,             // else
  TOK_ENUM,             // enum
  TOK_EXTERN,           // extern
  TOK_FLOAT,            // float
  TOK_FOR,              // for
  TOK_GOTO,             // goto
  TOK_IF,               // if
  TOK_INLINE,           // inline
  TOK_INT,              // int
  TOK_LONG,             // long
  TOK_REGISTER,         // register
  TOK_RESTRICT,         // restrict
  TOK_RETURN,           // return
  TOK_SHORT,            // short
  TOK_SIGNED,           // signed
  TOK_SIZEOF,           // sizeof
  TOK_STATIC,           // static
  TOK_STRUCT,           // struct
  TOK_SWITCH,           // switch
  TOK_TYPEDEF,          // typedef
  TOK_UNION,            // union
  TOK_UNSIGNED,         // unsigned
  TOK_VOID,             // void
  TOK_VOLATILE,         // volatile
  TOK_WHILE,            // while

  //
  // C99 Keywords
  //
  TOK_BOOL,             // _Bool
  TOK_COMPLEX,          // _Complex
  TOK_IMAGINARY,        // _Imaginary

  //
  // C11 Keywords
  //
  TOK_ALIGNAS,          // _Alignas
  TOK_ALIGNOF,          // _Alignof
  TOK_ATOMIC,           // _Atomic
  TOK_GENERIC,          // _Generic
  TOK_NORETURN,         // _Noreturn
  TOK_STATIC_ASSERT,    // _Static_assert
  TOK_THREAD_LOCAL,     // _Thread_local

  //
  // C23 Keywords
  //
  TOK_TYPEOF,           // typeof
  TOK_TYPEOF_UNQUAL,    // typeof_unqual
  TOK_CONSTEXPR,        // constexpr
  TOK_NULLPTR,          // nullptr
  TOK_BITINT,           // _BitInt

  //
  // GNU Extensions
  //
  TOK_ASM,              // asm / __asm / __asm__
  TOK_TYPEOF_GNU,       // __typeof / __typeof__
  TOK_ATTRIBUTE,        // __attribute / __attribute__
  TOK_EXTENSION,        // __extension__
  TOK_BUILTIN_VA_ARG,   // __builtin_va_arg
  TOK_INT128,           // __int128
  TOK_FLOAT128,         // __float128

  //
  // MSVC Keywords
  //
  TOK_CDECL,            // __cdecl
  TOK_STDCALL,          // __stdcall
  TOK_FASTCALL,         // __fastcall
  TOK_VECTORCALL,       // __vectorcall
  TOK_THISCALL,         // __thiscall
  TOK_DECLSPEC,         // __declspec
  TOK_FORCEINLINE,      // __forceinline
  TOK_INT8,             // __int8
  TOK_INT16,            // __int16
  TOK_INT32,            // __int32
  TOK_INT64,            // __int64
  TOK_PTR32,            // __ptr32
  TOK_PTR64,            // __ptr64
  TOK_UNALIGNED,        // __unaligned
  TOK_TRY,              // __try
  TOK_EXCEPT,           // __except
  TOK_FINALLY,          // __finally

  //
  // MetaWare Extensions
  //
  TOK_ARROW_YIELD,      // -> (for generator yield type)
  TOK_YIELD,            // yield (generator yield)
  TOK_NEAR,             // __near
  TOK_FAR,              // __far
  TOK_HUGE,             // __huge
  TOK_INTERRUPT,        // __interrupt
  TOK_REENTRANT,        // __reentrant
  TOK_TASK,             // __task
  TOK_PORT,             // __port
  TOK_BASED,            // __based
  TOK_SEGNAME,          // __segname
  TOK_STARTUP,          // __startup
  TOK_EXIT,             // __exit
  TOK_PASCAL,           // __pascal (calling convention)
  TOK_SYSCALL,          // __syscall (calling convention)
  TOK_FORTRAN,          // __fortran (calling convention)

  //
  // MetaWare Built-in Functions
  //
  TOK_BUILTIN_ABS,              // __abs
  TOK_BUILTIN_MIN,              // __min
  TOK_BUILTIN_MAX,              // __max
  TOK_BUILTIN_BIT_COUNT,        // __bit_count
  TOK_BUILTIN_LEADING_ZEROS,    // __leading_zeros
  TOK_BUILTIN_TRAILING_ZEROS,   // __trailing_zeros
  TOK_BUILTIN_ROTATE_LEFT,      // __rotate_left
  TOK_BUILTIN_ROTATE_RIGHT,     // __rotate_right
  TOK_BUILTIN_ATOMIC_ADD,       // __atomic_add
  TOK_BUILTIN_ATOMIC_SWAP,      // __atomic_swap
  TOK_BUILTIN_CAS,              // __compare_and_swap
  TOK_BUILTIN_SEGMENT_OF,       // __segment_of
  TOK_BUILTIN_OFFSET_OF,        // __offset_of
  TOK_BUILTIN_MAKE_POINTER,     // __make_pointer

  //
  // Open Watcom Extensions
  //
  TOK_WATCALL,          // __watcall (Watcom calling convention)
  TOK_PACKED,           // _Packed (packed struct/union)
  TOK_FAR16,            // __far16 (16-bit far pointer)
  TOK_SEGMENT,          // __segment (segment type)
  TOK_SELF,             // __self (self-relative pointer)
  TOK_PRAGMA_AUX,       // #pragma aux (auxiliary pragma)
  TOK_DECLSPEC_WATCOM,  // __declspec (Watcom version)
  TOK_INT64_WATCOM,     // __int64 (Watcom 64-bit int)
  TOK_INLINE_WATCOM,    // __inline (Watcom inline)
  TOK_LOADDS,           // __loadds (load DS register)
  TOK_SAVEREGS,         // __saveregs (save all registers)
  TOK_EXPORT,           // __export (export function)
  TOK_PRAGMA_INTRINSIC, // #pragma intrinsic

  //
  // Operators
  //
  TOK_PLUS,             // +
  TOK_MINUS,            // -
  TOK_STAR,             // *
  TOK_SLASH,            // /
  TOK_PERCENT,          // %
  TOK_AMPERSAND,        // &
  TOK_PIPE,             // |
  TOK_CARET,            // ^
  TOK_TILDE,            // ~
  TOK_EXCLAIM,          // !
  TOK_EQUAL,            // =
  TOK_LESS,             // <
  TOK_GREATER,          // >
  TOK_QUESTION,         // ?
  TOK_COLON,            // :

  //
  // Compound operators
  //
  TOK_PLUS_PLUS,        // ++
  TOK_MINUS_MINUS,      // --
  TOK_ARROW,            // ->
  TOK_SHIFT_LEFT,       // <<
  TOK_SHIFT_RIGHT,      // >>
  TOK_LESS_EQUAL,       // <=
  TOK_GREATER_EQUAL,    // >=
  TOK_EQUAL_EQUAL,      // ==
  TOK_NOT_EQUAL,        // !=
  TOK_LOGICAL_AND,      // &&
  TOK_LOGICAL_OR,       // ||
  TOK_PLUS_EQUAL,       // +=
  TOK_MINUS_EQUAL,      // -=
  TOK_STAR_EQUAL,       // *=
  TOK_SLASH_EQUAL,      // /=
  TOK_PERCENT_EQUAL,    // %=
  TOK_AMPERSAND_EQUAL,  // &=
  TOK_PIPE_EQUAL,       // |=
  TOK_CARET_EQUAL,      // ^=
  TOK_SHIFT_LEFT_EQUAL, // <<=
  TOK_SHIFT_RIGHT_EQUAL,// >>=
  TOK_DOT_DOT,          // .. (bit concatenation, MMIX extension)
  TOK_ELLIPSIS,         // ...
  TOK_DOUBLE_COLON,     // :: (C++ / scope)

  //
  // Delimiters
  //
  TOK_LPAREN,           // (
  TOK_RPAREN,           // )
  TOK_LBRACKET,         // [
  TOK_RBRACKET,         // ]
  TOK_LBRACE,           // {
  TOK_RBRACE,           // }
  TOK_COMMA,            // ,
  TOK_SEMICOLON,        // ;
  TOK_DOT,              // .
  TOK_HASH,             // # (preprocessor)
  TOK_DOUBLE_HASH,      // ## (preprocessor)

  //
  // Preprocessor tokens
  //
  TOK_PP_DEFINE,        // #define
  TOK_PP_UNDEF,         // #undef
  TOK_PP_INCLUDE,       // #include
  TOK_PP_IF,            // #if
  TOK_PP_IFDEF,         // #ifdef
  TOK_PP_IFNDEF,        // #ifndef
  TOK_PP_ELIF,          // #elif
  TOK_PP_ELIFDEF,       // #elifdef (C23)
  TOK_PP_ELIFNDEF,      // #elifndef (C23)
  TOK_PP_ELSE,          // #else
  TOK_PP_ENDIF,         // #endif
  TOK_PP_ERROR,         // #error
  TOK_PP_WARNING,       // #warning
  TOK_PP_PRAGMA,        // #pragma
  TOK_PP_LINE,          // #line
  TOK_PP_EMBED,         // #embed (C23)

  TOK_MAX               // Maximum token value
} TOKEN_TYPE;

/**
  Token location information
**/
typedef struct {
  CONST CHAR8  *FileName;
  UINT32       Line;
  UINT32       Column;
  UINT32       Offset;
} TOKEN_LOCATION;

/**
  Token value union
**/
typedef union {
  INT64        IntValue;
  UINT64       UIntValue;
  double       FloatValue;
  CHAR8        CharValue;
  CHAR8        *StringValue;
} TOKEN_VALUE;

/**
  Token structure
**/
typedef struct {
  TOKEN_TYPE       Type;
  TOKEN_LOCATION   Location;
  TOKEN_VALUE      Value;
  CHAR8            *Text;        // Original text
  UINT32           TextLength;   // Length of original text
  BOOLEAN          HasValue;     // TRUE if Value is valid
  BOOLEAN          SpaceBefore;  // TRUE if whitespace before token
} TOKEN;

/**
  Keyword entry
**/
typedef struct {
  CONST CHAR8   *Name;
  TOKEN_TYPE    Type;
  BOOLEAN       IsExtension;     // TRUE for compiler extensions
} KEYWORD_ENTRY;

/**
  Check if token is a keyword.

  @param[in]      Type          Token type.

  @return  TRUE if keyword, FALSE otherwise.

**/
BOOLEAN
TokenIsKeyword (
  IN  TOKEN_TYPE  Type
  );

/**
  Check if token is an operator.

  @param[in]      Type          Token type.

  @return  TRUE if operator, FALSE otherwise.

**/
BOOLEAN
TokenIsOperator (
  IN  TOKEN_TYPE  Type
  );

/**
  Check if token is a literal.

  @param[in]      Type          Token type.

  @return  TRUE if literal, FALSE otherwise.

**/
BOOLEAN
TokenIsLiteral (
  IN  TOKEN_TYPE  Type
  );

/**
  Get token type name.

  @param[in]      Type          Token type.

  @return  String name of token type.

**/
CONST CHAR8 *
TokenTypeName (
  IN  TOKEN_TYPE  Type
  );

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
  );

/**
  Destroy a token.

  @param[in]      Token         Token to destroy.

**/
VOID
TokenDestroy (
  IN  TOKEN  *Token
  );

#endif // MMIX_TOKEN_H_
