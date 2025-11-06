/**
  @file MmixSema.h

  Semantic analyzer for MMIX C23 compiler.
  Performs symbol table management, scope resolution, and type checking.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#ifndef MMIX_SEMA_H_
#define MMIX_SEMA_H_

#include "MmixTypes.h"
#include "MmixAst.h"

/**
  Symbol kinds
**/
typedef enum {
  SYMBOL_VAR,           // Variable
  SYMBOL_FUNCTION,      // Function
  SYMBOL_TYPEDEF,       // Type alias
  SYMBOL_STRUCT,        // Struct type
  SYMBOL_UNION,         // Union type
  SYMBOL_ENUM,          // Enum type
  SYMBOL_ENUMERATOR,    // Enum constant
  SYMBOL_LABEL,         // Label
} SYMBOL_KIND;

/**
  Symbol linkage
**/
typedef enum {
  LINKAGE_NONE,         // No linkage (local)
  LINKAGE_INTERNAL,     // Internal linkage (static)
  LINKAGE_EXTERNAL,     // External linkage
} SYMBOL_LINKAGE;

/**
  Symbol table entry
**/
typedef struct _SYMBOL {
  CHAR8                 *Name;            // Symbol name
  SYMBOL_KIND           Kind;             // Symbol kind
  AST_TYPE              *Type;            // Symbol type
  AST_DECL              *Decl;            // Declaration node
  SYMBOL_LINKAGE        Linkage;          // Symbol linkage
  UINT32                ScopeLevel;       // Scope nesting level
  struct _SYMBOL        *Next;            // Next symbol in chain
} SYMBOL;

/**
  Scope structure
**/
typedef struct _SCOPE {
  SYMBOL                **Symbols;        // Symbol hash table
  UINT32                SymbolCount;      // Number of symbols
  UINT32                Level;            // Nesting level (0 = global)
  struct _SCOPE         *Parent;          // Parent scope
  struct _SCOPE         *FirstChild;      // First child scope
  struct _SCOPE         *NextSibling;     // Next sibling scope
  BOOLEAN               IsFunction;       // Is function scope
  BOOLEAN               IsLoop;           // Is loop scope
  BOOLEAN               IsSwitch;         // Is switch scope
} SCOPE;

/**
  Semantic analyzer context
**/
typedef struct {
  SCOPE                 *GlobalScope;     // Global scope
  SCOPE                 *CurrentScope;    // Current scope
  AST_TRANSLATION_UNIT  *TranslationUnit; // AST root
  UINT32                ErrorCount;       // Number of errors
  UINT32                WarningCount;     // Number of warnings
  AST_TYPE              *CurrentFunction; // Current function return type
  BOOLEAN               InLoop;           // Inside loop
  BOOLEAN               InSwitch;         // Inside switch
} SEMA_CONTEXT;

/**
  Create semantic analyzer context.

  @param[in]      Unit          Translation unit to analyze.

  @return  Pointer to context, or NULL on error.

**/
SEMA_CONTEXT *
SemaCreate (
  IN  AST_TRANSLATION_UNIT  *Unit
  );

/**
  Destroy semantic analyzer context.

  @param[in]      Context       Semantic analyzer context.

**/
VOID
SemaDestroy (
  IN  SEMA_CONTEXT  *Context
  );

/**
  Perform semantic analysis on translation unit.

  @param[in,out]  Context       Semantic analyzer context.

  @return  MMIX_STATUS_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaAnalyze (
  IN OUT SEMA_CONTEXT  *Context
  );

/**
  Create a new scope.

  @param[in]      Parent        Parent scope.
  @param[in]      Level         Nesting level.

  @return  Pointer to scope, or NULL on error.

**/
SCOPE *
ScopeCreate (
  IN  SCOPE   *Parent,
  IN  UINT32  Level
  );

/**
  Destroy a scope.

  @param[in]      Scope         Scope to destroy.

**/
VOID
ScopeDestroy (
  IN  SCOPE  *Scope
  );

/**
  Enter a new scope.

  @param[in,out]  Context       Semantic analyzer context.

  @return  Pointer to new scope, or NULL on error.

**/
SCOPE *
SemaEnterScope (
  IN OUT SEMA_CONTEXT  *Context
  );

/**
  Exit current scope.

  @param[in,out]  Context       Semantic analyzer context.

**/
VOID
SemaExitScope (
  IN OUT SEMA_CONTEXT  *Context
  );

/**
  Add symbol to current scope.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in]      Name          Symbol name.
  @param[in]      Kind          Symbol kind.
  @param[in]      Type          Symbol type.
  @param[in]      Decl          Declaration node.

  @return  Pointer to symbol, or NULL on error.

**/
SYMBOL *
SemaAddSymbol (
  IN OUT SEMA_CONTEXT  *Context,
  IN     CONST CHAR8   *Name,
  IN     SYMBOL_KIND   Kind,
  IN     AST_TYPE      *Type,
  IN     AST_DECL      *Decl
  );

/**
  Look up symbol in current scope and parent scopes.

  @param[in]      Context       Semantic analyzer context.
  @param[in]      Name          Symbol name.

  @return  Pointer to symbol, or NULL if not found.

**/
SYMBOL *
SemaLookupSymbol (
  IN  SEMA_CONTEXT  *Context,
  IN  CONST CHAR8   *Name
  );

/**
  Look up symbol in current scope only.

  @param[in]      Scope         Scope to search.
  @param[in]      Name          Symbol name.

  @return  Pointer to symbol, or NULL if not found.

**/
SYMBOL *
ScopeLookupSymbol (
  IN  SCOPE        *Scope,
  IN  CONST CHAR8  *Name
  );

/**
  Check if two types are compatible.

  @param[in]      Type1         First type.
  @param[in]      Type2         Second type.

  @return  TRUE if compatible, FALSE otherwise.

**/
BOOLEAN
SemaTypesCompatible (
  IN  AST_TYPE  *Type1,
  IN  AST_TYPE  *Type2
  );

/**
  Check if type is complete.

  @param[in]      Type          Type to check.

  @return  TRUE if complete, FALSE otherwise.

**/
BOOLEAN
SemaIsCompleteType (
  IN  AST_TYPE  *Type
  );

/**
  Check if type is integer type.

  @param[in]      Type          Type to check.

  @return  TRUE if integer, FALSE otherwise.

**/
BOOLEAN
SemaIsIntegerType (
  IN  AST_TYPE  *Type
  );

/**
  Check if type is arithmetic type.

  @param[in]      Type          Type to check.

  @return  TRUE if arithmetic, FALSE otherwise.

**/
BOOLEAN
SemaIsArithmeticType (
  IN  AST_TYPE  *Type
  );

/**
  Check if type is scalar type.

  @param[in]      Type          Type to check.

  @return  TRUE if scalar, FALSE otherwise.

**/
BOOLEAN
SemaIsScalarType (
  IN  AST_TYPE  *Type
  );

/**
  Perform type checking on expression.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in,out]  Expr          Expression to check.

  @return  MMIX_STATUS_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaCheckExpression (
  IN OUT SEMA_CONTEXT  *Context,
  IN OUT AST_EXPR      *Expr
  );

/**
  Perform type checking on statement.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in,out]  Stmt          Statement to check.

  @return  MMIX_STATUS_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaCheckStatement (
  IN OUT SEMA_CONTEXT  *Context,
  IN OUT AST_STMT      *Stmt
  );

/**
  Perform type checking on declaration.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in,out]  Decl          Declaration to check.

  @return  MMIX_STATUS_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaCheckDeclaration (
  IN OUT SEMA_CONTEXT  *Context,
  IN OUT AST_DECL      *Decl
  );

/**
  Report semantic error.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in]      Location      Source location.
  @param[in]      Message       Error message.

**/
VOID
SemaError (
  IN OUT SEMA_CONTEXT        *Context,
  IN     CONST TOKEN_LOCATION *Location,
  IN     CONST CHAR8          *Message
  );

/**
  Report semantic warning.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in]      Location      Source location.
  @param[in]      Message       Warning message.

**/
VOID
SemaWarning (
  IN OUT SEMA_CONTEXT         *Context,
  IN     CONST TOKEN_LOCATION *Location,
  IN     CONST CHAR8          *Message
  );

#endif // MMIX_SEMA_H_
