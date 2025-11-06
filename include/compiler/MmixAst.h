/** @file
  MMIX C Compiler Abstract Syntax Tree Definitions.

  This file defines the AST node structures for the MMIX C23 compiler.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_AST_H_
#define MMIX_AST_H_

#include "../MmixTypes.h"
#include "MmixToken.h"

//
// Forward declarations
//
typedef struct _AST_NODE         AST_NODE;
typedef struct _AST_TYPE         AST_TYPE;
typedef struct _AST_EXPR         AST_EXPR;
typedef struct _AST_STMT         AST_STMT;
typedef struct _AST_DECL         AST_DECL;

/**
  AST node kinds
**/
typedef enum {
  //
  // Expression nodes
  //
  AST_EXPR_INTEGER,           // Integer literal
  AST_EXPR_FLOAT,             // Floating-point literal
  AST_EXPR_STRING,            // String literal
  AST_EXPR_CHAR,              // Character literal
  AST_EXPR_IDENTIFIER,        // Identifier reference
  AST_EXPR_BINARY,            // Binary operation
  AST_EXPR_UNARY,             // Unary operation
  AST_EXPR_CALL,              // Function call
  AST_EXPR_MEMBER,            // Struct/union member access
  AST_EXPR_INDEX,             // Array subscript
  AST_EXPR_CAST,              // Type cast
  AST_EXPR_SIZEOF,            // sizeof operator
  AST_EXPR_ALIGNOF,           // _Alignof operator
  AST_EXPR_COMPOUND_LITERAL,  // Compound literal (C99)
  AST_EXPR_GENERIC,           // _Generic selection (C11)
  AST_EXPR_STMT_EXPR,         // Statement expression (GNU)
  AST_EXPR_CONDITIONAL,       // Ternary operator (? :)
  AST_EXPR_COMMA,             // Comma expression
  AST_EXPR_BUILTIN,           // Compiler builtin
  AST_EXPR_VA_ARG,            // va_arg
  AST_EXPR_OFFSETOF,          // offsetof
  AST_EXPR_TYPEOF,            // typeof expression (C23/GNU)

  //
  // Statement nodes
  //
  AST_STMT_COMPOUND,          // Compound statement { }
  AST_STMT_EXPR,              // Expression statement
  AST_STMT_IF,                // if statement
  AST_STMT_WHILE,             // while loop
  AST_STMT_DO_WHILE,          // do-while loop
  AST_STMT_FOR,               // for loop
  AST_STMT_SWITCH,            // switch statement
  AST_STMT_CASE,              // case label
  AST_STMT_DEFAULT,           // default label
  AST_STMT_LABEL,             // goto label
  AST_STMT_GOTO,              // goto statement
  AST_STMT_CONTINUE,          // continue statement
  AST_STMT_BREAK,             // break statement
  AST_STMT_RETURN,            // return statement
  AST_STMT_DECL,              // Declaration statement
  AST_STMT_ASM,               // Inline assembly (GNU)
  AST_STMT_TRY,               // __try block (MSVC)
  AST_STMT_EXCEPT,            // __except handler (MSVC)
  AST_STMT_FINALLY,           // __finally block (MSVC)
  AST_STMT_NULL,              // Empty statement

  //
  // Declaration nodes
  //
  AST_DECL_VAR,               // Variable declaration
  AST_DECL_FUNCTION,          // Function declaration/definition
  AST_DECL_TYPEDEF,           // typedef declaration
  AST_DECL_STRUCT,            // struct declaration
  AST_DECL_UNION,             // union declaration
  AST_DECL_ENUM,              // enum declaration
  AST_DECL_FIELD,             // struct/union field
  AST_DECL_ENUMERATOR,        // enum constant

  //
  // Type nodes
  //
  AST_TYPE_VOID,              // void type
  AST_TYPE_BOOL,              // _Bool type
  AST_TYPE_CHAR,              // char type
  AST_TYPE_UCHAR,             // unsigned char type
  AST_TYPE_SHORT,             // short type
  AST_TYPE_USHORT,            // unsigned short type
  AST_TYPE_INT,               // int type
  AST_TYPE_UINT,              // unsigned int type
  AST_TYPE_LONG,              // long type
  AST_TYPE_ULONG,             // unsigned long type
  AST_TYPE_LONG_LONG,         // long long type
  AST_TYPE_ULONG_LONG,        // unsigned long long type
  AST_TYPE_INT8,              // __int8 type (MSVC)
  AST_TYPE_INT16,             // __int16 type (MSVC)
  AST_TYPE_INT32,             // __int32 type (MSVC)
  AST_TYPE_INT64,             // __int64 type (MSVC)
  AST_TYPE_INT128,            // __int128 type (GNU)
  AST_TYPE_FLOAT,             // float type
  AST_TYPE_DOUBLE,            // double type
  AST_TYPE_LONG_DOUBLE,       // long double type
  AST_TYPE_FLOAT128,          // __float128 type (GNU)
  AST_TYPE_COMPLEX,           // _Complex type
  AST_TYPE_FLOAT_COMPLEX,     // float _Complex type
  AST_TYPE_DOUBLE_COMPLEX,    // double _Complex type
  AST_TYPE_BITINT,            // _BitInt(N) type (C23)
  AST_TYPE_QUALIFIED,         // Qualified type (const/volatile/restrict)
  AST_TYPE_POINTER,           // Pointer type
  AST_TYPE_ARRAY,             // Array type
  AST_TYPE_FUNCTION,          // Function type
  AST_TYPE_STRUCT,            // Struct type
  AST_TYPE_UNION,             // Union type
  AST_TYPE_ENUM,              // Enum type
  AST_TYPE_TYPEOF,            // typeof type (C23/GNU)
  AST_TYPE_ATOMIC,            // _Atomic type (C11)
} AST_NODE_KIND;

/**
  Type qualifiers
**/
typedef enum {
  QUAL_NONE       = 0,
  QUAL_CONST      = (1 << 0),
  QUAL_VOLATILE   = (1 << 1),
  QUAL_RESTRICT   = (1 << 2),
  QUAL_ATOMIC     = (1 << 3),
} TYPE_QUALIFIER;

/**
  Storage class specifiers
**/
typedef enum {
  STORAGE_NONE,
  STORAGE_AUTO,
  STORAGE_REGISTER,
  STORAGE_STATIC,
  STORAGE_EXTERN,
  STORAGE_TYPEDEF,
  STORAGE_THREAD_LOCAL,
  STORAGE_CONSTEXPR,          // C23
} STORAGE_CLASS;

/**
  Function specifiers
**/
typedef enum {
  FUNC_SPEC_NONE      = 0,
  FUNC_SPEC_INLINE    = (1 << 0),
  FUNC_SPEC_NORETURN  = (1 << 1),
} FUNCTION_SPECIFIER;

/**
  Binary operators
**/
typedef enum {
  BIN_OP_ADD,             // +
  BIN_OP_SUB,             // -
  BIN_OP_MUL,             // *
  BIN_OP_DIV,             // /
  BIN_OP_MOD,             // %
  BIN_OP_BIT_AND,         // &
  BIN_OP_BIT_OR,          // |
  BIN_OP_BIT_XOR,         // ^
  BIN_OP_SHIFT_LEFT,      // <<
  BIN_OP_SHIFT_RIGHT,     // >>
  BIN_OP_LOGICAL_AND,     // &&
  BIN_OP_LOGICAL_OR,      // ||
  BIN_OP_EQ,              // ==
  BIN_OP_NE,              // !=
  BIN_OP_LT,              // <
  BIN_OP_LE,              // <=
  BIN_OP_GT,              // >
  BIN_OP_GE,              // >=
  BIN_OP_ASSIGN,          // =
  BIN_OP_ADD_ASSIGN,      // +=
  BIN_OP_SUB_ASSIGN,      // -=
  BIN_OP_MUL_ASSIGN,      // *=
  BIN_OP_DIV_ASSIGN,      // /=
  BIN_OP_MOD_ASSIGN,      // %=
  BIN_OP_AND_ASSIGN,      // &=
  BIN_OP_OR_ASSIGN,       // |=
  BIN_OP_XOR_ASSIGN,      // ^=
  BIN_OP_SHL_ASSIGN,      // <<=
  BIN_OP_SHR_ASSIGN,      // >>=
  BIN_OP_COMMA,           // ,
} BINARY_OPERATOR;

/**
  Unary operators
**/
typedef enum {
  UN_OP_PLUS,             // +
  UN_OP_MINUS,            // -
  UN_OP_BIT_NOT,          // ~
  UN_OP_LOGICAL_NOT,      // !
  UN_OP_PRE_INC,          // ++x
  UN_OP_PRE_DEC,          // --x
  UN_OP_POST_INC,         // x++
  UN_OP_POST_DEC,         // x--
  UN_OP_ADDRESS_OF,       // &
  UN_OP_DEREF,            // *
} UNARY_OPERATOR;

/**
  AST type structure
**/
struct _AST_TYPE {
  AST_NODE_KIND      Kind;
  TOKEN_LOCATION     Location;
  TYPE_QUALIFIER     Qualifiers;
  BOOLEAN            IsUnsigned;
  BOOLEAN            IsSigned;

  union {
    struct {
      AST_TYPE      *PointeeType;     // Pointer target type
      BOOLEAN       IsRestrict;
    } Pointer;

    struct {
      AST_TYPE      *ElementType;     // Array element type
      AST_EXPR      *Size;            // Array size (NULL for flexible)
      BOOLEAN       IsStatic;
      BOOLEAN       IsVLA;
    } Array;

    struct {
      AST_TYPE      *ReturnType;      // Function return type
      AST_DECL      **Parameters;     // Parameter list
      UINT32        ParameterCount;
      BOOLEAN       IsVariadic;
      BOOLEAN       IsOldStyle;       // K&R style
    } Function;

    struct {
      CHAR8         *Name;            // Struct/union name
      AST_DECL      **Fields;         // Field declarations
      UINT32        FieldCount;
      BOOLEAN       IsComplete;
    } Struct;

    struct {
      CHAR8         *Name;            // Enum name
      AST_DECL      **Enumerators;    // Enum constants
      UINT32        EnumeratorCount;
      AST_TYPE      *UnderlyingType;  // Fixed underlying type (C23)
    } Enum;

    struct {
      UINT32        Width;            // _BitInt width
    } BitInt;

    struct {
      AST_EXPR      *Expr;            // typeof expression
    } Typeof;

    struct {
      AST_TYPE      *BaseType;        // Base type for qualified type
      BOOLEAN       IsConst;
      BOOLEAN       IsVolatile;
      BOOLEAN       IsRestrict;
      BOOLEAN       IsAtomic;
    } Qualified;
  };
};

/**
  AST expression structure
**/
struct _AST_EXPR {
  AST_NODE_KIND      Kind;
  TOKEN_LOCATION     Location;
  AST_TYPE           *Type;           // Expression type (computed)

  union {
    struct {
      UINT64        Value;
      BOOLEAN       IsUnsigned;
    } Integer;

    struct {
      double        Value;
    } Float;

    struct {
      CHAR8         *Value;
      UINT32        Length;
    } String;

    struct {
      CHAR8         Value;
    } Char;

    struct {
      CHAR8         *Name;
      AST_DECL      *Decl;            // Resolved declaration
    } Identifier;

    struct {
      BINARY_OPERATOR  Op;
      AST_EXPR      *Left;
      AST_EXPR      *Right;
    } Binary;

    struct {
      UNARY_OPERATOR   Op;
      AST_EXPR      *Operand;
    } Unary;

    struct {
      AST_EXPR      *Callee;
      AST_EXPR      **Arguments;
      UINT32        ArgumentCount;
    } Call;

    struct {
      AST_EXPR      *Object;
      CHAR8         *MemberName;
      BOOLEAN       IsArrow;          // -> vs .
    } Member;

    struct {
      AST_EXPR      *Array;
      AST_EXPR      *Index;
    } Index;

    struct {
      AST_TYPE      *TargetType;
      AST_EXPR      *Operand;
    } Cast;

    struct {
      AST_TYPE      *TargetType;      // NULL for sizeof(expr)
      AST_EXPR      *Operand;
    } Sizeof;

    struct {
      AST_EXPR      *Condition;
      AST_EXPR      *ThenExpr;
      AST_EXPR      *ElseExpr;
    } Conditional;

    struct {
      AST_STMT      *Body;            // Statement expression body
    } StmtExpr;

    struct {
      AST_TYPE      *ControlType;
      AST_EXPR      **Associations;   // _Generic associations
      UINT32        AssociationCount;
    } Generic;
  };
};

/**
  AST statement structure
**/
struct _AST_STMT {
  AST_NODE_KIND      Kind;
  TOKEN_LOCATION     Location;

  union {
    struct {
      AST_STMT      **Statements;
      UINT32        StatementCount;
    } Compound;

    struct {
      AST_EXPR      *Expression;
    } Expr;

    struct {
      AST_EXPR      *Condition;
      AST_STMT      *ThenBranch;
      AST_STMT      *ElseBranch;
    } If;

    struct {
      AST_EXPR      *Condition;
      AST_STMT      *Body;
    } While;

    struct {
      AST_STMT      *Initializer;
      AST_EXPR      *Condition;
      AST_EXPR      *Increment;
      AST_STMT      *Body;
    } For;

    struct {
      AST_EXPR      *Expression;
      AST_STMT      *Body;
    } Switch;

    struct {
      AST_EXPR      *Value;
      AST_STMT      *Body;
      BOOLEAN       IsDefault;
    } Case;

    struct {
      CHAR8         *Name;
    } Label;

    struct {
      CHAR8         *Target;
      VOID          **LabelAddress;   // For computed goto (GNU)
    } Goto;

    struct {
      AST_EXPR      *Value;
    } Return;

    struct {
      AST_DECL      **Declarations;
      UINT32        DeclarationCount;
    } Decl;

    struct {
      CHAR8         *AsmString;
      AST_EXPR      **Outputs;
      AST_EXPR      **Inputs;
      CHAR8         **Clobbers;
      UINT32        OutputCount;
      UINT32        InputCount;
      UINT32        ClobberCount;
      BOOLEAN       IsVolatile;
    } Asm;

    struct {
      AST_STMT      *TryBlock;
      AST_STMT      *Handler;
      AST_EXPR      *FilterExpr;
    } Try;
  };
};

/**
  AST declaration structure
**/
struct _AST_DECL {
  AST_NODE_KIND      Kind;
  TOKEN_LOCATION     Location;
  CHAR8              *Name;
  AST_TYPE           *Type;
  STORAGE_CLASS      StorageClass;
  FUNCTION_SPECIFIER FunctionSpecifiers;

  union {
    struct {
      AST_EXPR      *Initializer;
      BOOLEAN       IsTentative;
    } Var;

    struct {
      AST_STMT      *Body;
      AST_DECL      **Parameters;
      UINT32        ParameterCount;
      BOOLEAN       IsDefinition;
      BOOLEAN       IsInline;
      BOOLEAN       IsVariadic;
      CHAR8         **AttributeNames;     // GNU attributes
      UINT32        AttributeCount;
    } Function;

    struct {
      AST_TYPE      *AliasedType;
    } Typedef;

    struct {
      AST_EXPR      *Value;
    } Enumerator;

    struct {
      AST_EXPR      *BitWidth;            // Bit-field width
    } Field;
  };

  AST_DECL           *NextInScope;        // Next declaration in scope
};

/**
  AST translation unit (top level)
**/
typedef struct {
  AST_DECL      **Declarations;
  UINT32        DeclarationCount;
  CHAR8         *SourceFile;
} AST_TRANSLATION_UNIT;

/**
  Create a new expression node.

  @param[in]      Kind          Expression kind.
  @param[in]      Location      Source location.

  @return  Pointer to expression node, or NULL on error.

**/
AST_EXPR *
AstExprCreate (
  IN  AST_NODE_KIND            Kind,
  IN  CONST TOKEN_LOCATION     *Location
  );

/**
  Create a new statement node.

  @param[in]      Kind          Statement kind.
  @param[in]      Location      Source location.

  @return  Pointer to statement node, or NULL on error.

**/
AST_STMT *
AstStmtCreate (
  IN  AST_NODE_KIND            Kind,
  IN  CONST TOKEN_LOCATION     *Location
  );

/**
  Create a new declaration node.

  @param[in]      Kind          Declaration kind.
  @param[in]      Location      Source location.
  @param[in]      Name          Declaration name.

  @return  Pointer to declaration node, or NULL on error.

**/
AST_DECL *
AstDeclCreate (
  IN  AST_NODE_KIND            Kind,
  IN  CONST TOKEN_LOCATION     *Location,
  IN  CONST CHAR8              *Name
  );

/**
  Create a new type node.

  @param[in]      Kind          Type kind.

  @return  Pointer to type node, or NULL on error.

**/
AST_TYPE *
AstTypeCreate (
  IN  AST_NODE_KIND  Kind
  );

/**
  Destroy an AST node (recursive).

  @param[in]      Node          AST node to destroy.

**/
VOID
AstNodeDestroy (
  IN  VOID  *Node
  );

/**
  Create integer literal expression.

  @param[in]      Location      Source location.
  @param[in]      Value         Integer value.
  @param[in]      IsUnsigned    TRUE if unsigned.

  @return  Pointer to expression node.

**/
AST_EXPR *
AstExprCreateInteger (
  IN  CONST TOKEN_LOCATION  *Location,
  IN  UINT64                Value,
  IN  BOOLEAN               IsUnsigned
  );

/**
  Create binary expression.

  @param[in]      Location      Source location.
  @param[in]      Op            Binary operator.
  @param[in]      Left          Left operand.
  @param[in]      Right         Right operand.

  @return  Pointer to expression node.

**/
AST_EXPR *
AstExprCreateBinary (
  IN  CONST TOKEN_LOCATION  *Location,
  IN  BINARY_OPERATOR       Op,
  IN  AST_EXPR              *Left,
  IN  AST_EXPR              *Right
  );

/**
  Create identifier expression.

  @param[in]      Location      Source location.
  @param[in]      Name          Identifier name.

  @return  Pointer to expression node.

**/
AST_EXPR *
AstExprCreateIdentifier (
  IN  CONST TOKEN_LOCATION  *Location,
  IN  CONST CHAR8           *Name
  );

#endif // MMIX_AST_H_
