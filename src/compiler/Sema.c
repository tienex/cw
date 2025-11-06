/**
  @file Sema.c

  Semantic analyzer implementation for MMIX C23 compiler.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/MmixSema.h"

#define SYMBOL_TABLE_SIZE  128

/**
  Hash function for symbol names.

  @param[in]      Name          Symbol name.

  @return  Hash value.

**/
STATIC
UINT32
HashSymbolName (
  IN  CONST CHAR8  *Name
  )
{
  UINT32  Hash = 0;

  while (*Name) {
    Hash = Hash * 31 + *Name;
    Name++;
  }

  return Hash % SYMBOL_TABLE_SIZE;
}

/**
  Create semantic analyzer context.

  @param[in]      Unit          Translation unit to analyze.

  @return  Pointer to context, or NULL on error.

**/
SEMA_CONTEXT *
SemaCreate (
  IN  AST_TRANSLATION_UNIT  *Unit
  )
{
  SEMA_CONTEXT  *Context;

  Context = (SEMA_CONTEXT *)calloc (1, sizeof (SEMA_CONTEXT));
  if (Context == NULL) {
    return NULL;
  }

  Context->TranslationUnit = Unit;
  Context->ErrorCount = 0;
  Context->WarningCount = 0;
  Context->CurrentFunction = NULL;
  Context->InLoop = FALSE;
  Context->InSwitch = FALSE;

  //
  // Create global scope
  //
  Context->GlobalScope = ScopeCreate (NULL, 0);
  if (Context->GlobalScope == NULL) {
    free (Context);
    return NULL;
  }

  Context->CurrentScope = Context->GlobalScope;

  return Context;
}

/**
  Destroy semantic analyzer context.

  @param[in]      Context       Semantic analyzer context.

**/
VOID
SemaDestroy (
  IN  SEMA_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return;
  }

  if (Context->GlobalScope != NULL) {
    ScopeDestroy (Context->GlobalScope);
  }

  free (Context);
}

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
  )
{
  SCOPE  *Scope;

  Scope = (SCOPE *)calloc (1, sizeof (SCOPE));
  if (Scope == NULL) {
    return NULL;
  }

  Scope->Symbols = (SYMBOL **)calloc (SYMBOL_TABLE_SIZE, sizeof (SYMBOL *));
  if (Scope->Symbols == NULL) {
    free (Scope);
    return NULL;
  }

  Scope->SymbolCount = 0;
  Scope->Level = Level;
  Scope->Parent = Parent;
  Scope->FirstChild = NULL;
  Scope->NextSibling = NULL;
  Scope->IsFunction = FALSE;
  Scope->IsLoop = FALSE;
  Scope->IsSwitch = FALSE;

  //
  // Add to parent's child list
  //
  if (Parent != NULL) {
    if (Parent->FirstChild == NULL) {
      Parent->FirstChild = Scope;
    } else {
      SCOPE  *Sibling = Parent->FirstChild;
      while (Sibling->NextSibling != NULL) {
        Sibling = Sibling->NextSibling;
      }
      Sibling->NextSibling = Scope;
    }
  }

  return Scope;
}

/**
  Destroy a scope.

  @param[in]      Scope         Scope to destroy.

**/
VOID
ScopeDestroy (
  IN  SCOPE  *Scope
  )
{
  if (Scope == NULL) {
    return;
  }

  //
  // Free all symbols
  //
  if (Scope->Symbols != NULL) {
    for (UINT32 i = 0; i < SYMBOL_TABLE_SIZE; i++) {
      SYMBOL  *Sym = Scope->Symbols[i];
      while (Sym != NULL) {
        SYMBOL  *Next = Sym->Next;
        if (Sym->Name != NULL) {
          free (Sym->Name);
        }
        free (Sym);
        Sym = Next;
      }
    }
    free (Scope->Symbols);
  }

  //
  // Recursively destroy children
  //
  SCOPE  *Child = Scope->FirstChild;
  while (Child != NULL) {
    SCOPE  *Next = Child->NextSibling;
    ScopeDestroy (Child);
    Child = Next;
  }

  free (Scope);
}

/**
  Enter a new scope.

  @param[in,out]  Context       Semantic analyzer context.

  @return  Pointer to new scope, or NULL on error.

**/
SCOPE *
SemaEnterScope (
  IN OUT SEMA_CONTEXT  *Context
  )
{
  SCOPE  *NewScope;

  NewScope = ScopeCreate (Context->CurrentScope, Context->CurrentScope->Level + 1);
  if (NewScope == NULL) {
    return NULL;
  }

  Context->CurrentScope = NewScope;
  return NewScope;
}

/**
  Exit current scope.

  @param[in,out]  Context       Semantic analyzer context.

**/
VOID
SemaExitScope (
  IN OUT SEMA_CONTEXT  *Context
  )
{
  if (Context->CurrentScope->Parent != NULL) {
    Context->CurrentScope = Context->CurrentScope->Parent;
  }
}

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
  )
{
  UINT32  Hash;
  SYMBOL  *Sym;

  if (Scope == NULL || Name == NULL) {
    return NULL;
  }

  Hash = HashSymbolName (Name);
  Sym = Scope->Symbols[Hash];

  while (Sym != NULL) {
    if (strcmp (Sym->Name, Name) == 0) {
      return Sym;
    }
    Sym = Sym->Next;
  }

  return NULL;
}

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
  )
{
  SCOPE   *Scope;
  SYMBOL  *Sym;

  Scope = Context->CurrentScope;

  //
  // Search from current scope to global scope
  //
  while (Scope != NULL) {
    Sym = ScopeLookupSymbol (Scope, Name);
    if (Sym != NULL) {
      return Sym;
    }
    Scope = Scope->Parent;
  }

  return NULL;
}

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
  )
{
  UINT32  Hash;
  SYMBOL  *Sym;
  SYMBOL  *Existing;

  //
  // Check for redeclaration in current scope
  //
  Existing = ScopeLookupSymbol (Context->CurrentScope, Name);
  if (Existing != NULL) {
    //
    // Allow compatible redeclarations at global scope
    //
    if (Context->CurrentScope->Level == 0) {
      //
      // TODO: Check for compatible redeclarations
      //
    } else {
      SemaError (Context, &Decl->Location, "Redeclaration of symbol");
      return NULL;
    }
  }

  //
  // Create new symbol
  //
  Sym = (SYMBOL *)calloc (1, sizeof (SYMBOL));
  if (Sym == NULL) {
    return NULL;
  }

  Sym->Name = strdup (Name);
  Sym->Kind = Kind;
  Sym->Type = Type;
  Sym->Decl = Decl;
  Sym->ScopeLevel = Context->CurrentScope->Level;

  //
  // Determine linkage
  //
  if (Decl->StorageClass == STORAGE_STATIC) {
    Sym->Linkage = LINKAGE_INTERNAL;
  } else if (Decl->StorageClass == STORAGE_EXTERN || Context->CurrentScope->Level == 0) {
    Sym->Linkage = LINKAGE_EXTERNAL;
  } else {
    Sym->Linkage = LINKAGE_NONE;
  }

  //
  // Add to hash table
  //
  Hash = HashSymbolName (Name);
  Sym->Next = Context->CurrentScope->Symbols[Hash];
  Context->CurrentScope->Symbols[Hash] = Sym;
  Context->CurrentScope->SymbolCount++;

  return Sym;
}

/**
  Check if type is integer type.

  @param[in]      Type          Type to check.

  @return  TRUE if integer, FALSE otherwise.

**/
BOOLEAN
SemaIsIntegerType (
  IN  AST_TYPE  *Type
  )
{
  if (Type == NULL) {
    return FALSE;
  }

  //
  // Unwrap qualified types
  //
  if (Type->Kind == AST_TYPE_QUALIFIED) {
    return SemaIsIntegerType (Type->Qualified.BaseType);
  }

  return (Type->Kind == AST_TYPE_BOOL ||
          Type->Kind == AST_TYPE_CHAR ||
          Type->Kind == AST_TYPE_UCHAR ||
          Type->Kind == AST_TYPE_SHORT ||
          Type->Kind == AST_TYPE_USHORT ||
          Type->Kind == AST_TYPE_INT ||
          Type->Kind == AST_TYPE_UINT ||
          Type->Kind == AST_TYPE_LONG ||
          Type->Kind == AST_TYPE_ULONG ||
          Type->Kind == AST_TYPE_LONG_LONG ||
          Type->Kind == AST_TYPE_ULONG_LONG ||
          Type->Kind == AST_TYPE_INT8 ||
          Type->Kind == AST_TYPE_INT16 ||
          Type->Kind == AST_TYPE_INT32 ||
          Type->Kind == AST_TYPE_INT64 ||
          Type->Kind == AST_TYPE_INT128);
}

/**
  Check if type is arithmetic type.

  @param[in]      Type          Type to check.

  @return  TRUE if arithmetic, FALSE otherwise.

**/
BOOLEAN
SemaIsArithmeticType (
  IN  AST_TYPE  *Type
  )
{
  if (Type == NULL) {
    return FALSE;
  }

  //
  // Unwrap qualified types
  //
  if (Type->Kind == AST_TYPE_QUALIFIED) {
    return SemaIsArithmeticType (Type->Qualified.BaseType);
  }

  return (SemaIsIntegerType (Type) ||
          Type->Kind == AST_TYPE_FLOAT ||
          Type->Kind == AST_TYPE_DOUBLE ||
          Type->Kind == AST_TYPE_LONG_DOUBLE ||
          Type->Kind == AST_TYPE_FLOAT128 ||
          Type->Kind == AST_TYPE_COMPLEX ||
          Type->Kind == AST_TYPE_FLOAT_COMPLEX ||
          Type->Kind == AST_TYPE_DOUBLE_COMPLEX);
}

/**
  Check if type is scalar type.

  @param[in]      Type          Type to check.

  @return  TRUE if scalar, FALSE otherwise.

**/
BOOLEAN
SemaIsScalarType (
  IN  AST_TYPE  *Type
  )
{
  if (Type == NULL) {
    return FALSE;
  }

  //
  // Unwrap qualified types
  //
  if (Type->Kind == AST_TYPE_QUALIFIED) {
    return SemaIsScalarType (Type->Qualified.BaseType);
  }

  return (SemaIsArithmeticType (Type) ||
          Type->Kind == AST_TYPE_POINTER ||
          Type->Kind == AST_TYPE_ENUM);
}

/**
  Check if type is complete.

  @param[in]      Type          Type to check.

  @return  TRUE if complete, FALSE otherwise.

**/
BOOLEAN
SemaIsCompleteType (
  IN  AST_TYPE  *Type
  )
{
  if (Type == NULL) {
    return FALSE;
  }

  //
  // Unwrap qualified types
  //
  if (Type->Kind == AST_TYPE_QUALIFIED) {
    return SemaIsCompleteType (Type->Qualified.BaseType);
  }

  //
  // Void is incomplete
  //
  if (Type->Kind == AST_TYPE_VOID) {
    return FALSE;
  }

  //
  // Arrays with unknown size are incomplete
  //
  if (Type->Kind == AST_TYPE_ARRAY) {
    if (Type->Array.Size == NULL) {
      return FALSE;
    }
    return SemaIsCompleteType (Type->Array.ElementType);
  }

  //
  // Structs/unions must be complete
  //
  if (Type->Kind == AST_TYPE_STRUCT || Type->Kind == AST_TYPE_UNION) {
    return Type->Struct.IsComplete;
  }

  return TRUE;
}

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
  )
{
  if (Type1 == NULL || Type2 == NULL) {
    return FALSE;
  }

  //
  // Unwrap qualified types
  //
  if (Type1->Kind == AST_TYPE_QUALIFIED) {
    Type1 = Type1->Qualified.BaseType;
  }
  if (Type2->Kind == AST_TYPE_QUALIFIED) {
    Type2 = Type2->Qualified.BaseType;
  }

  //
  // Same type kind
  //
  if (Type1->Kind != Type2->Kind) {
    return FALSE;
  }

  //
  // Check specific type compatibility
  //
  switch (Type1->Kind) {
    case AST_TYPE_POINTER:
      return SemaTypesCompatible (Type1->Pointer.PointeeType, Type2->Pointer.PointeeType);

    case AST_TYPE_ARRAY:
      return SemaTypesCompatible (Type1->Array.ElementType, Type2->Array.ElementType);

    case AST_TYPE_FUNCTION:
      //
      // TODO: Check function type compatibility
      //
      return TRUE;

    case AST_TYPE_STRUCT:
    case AST_TYPE_UNION:
    case AST_TYPE_ENUM:
      //
      // Check by name
      //
      if (Type1->Struct.Name != NULL && Type2->Struct.Name != NULL) {
        return strcmp (Type1->Struct.Name, Type2->Struct.Name) == 0;
      }
      return Type1 == Type2;

    default:
      return TRUE;
  }
}

/**
  Report semantic error.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in]      Location      Source location.
  @param[in]      Message       Error message.

**/
VOID
SemaError (
  IN OUT SEMA_CONTEXT         *Context,
  IN     CONST TOKEN_LOCATION *Location,
  IN     CONST CHAR8          *Message
  )
{
  if (Location != NULL) {
    fprintf (stderr, "%s:%u:%u: error: %s\n",
             Location->FileName, Location->Line, Location->Column, Message);
  } else {
    fprintf (stderr, "error: %s\n", Message);
  }
  Context->ErrorCount++;
}

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
  )
{
  if (Location != NULL) {
    fprintf (stderr, "%s:%u:%u: warning: %s\n",
             Location->FileName, Location->Line, Location->Column, Message);
  } else {
    fprintf (stderr, "warning: %s\n", Message);
  }
  Context->WarningCount++;
}

/**
  Perform type checking on declaration.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in,out]  Decl          Declaration to check.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaCheckDeclaration (
  IN OUT SEMA_CONTEXT  *Context,
  IN OUT AST_DECL      *Decl
  )
{
  SYMBOL_KIND  SymbolKind;

  if (Decl == NULL) {
    return MMIX_SUCCESS;
  }

  //
  // Determine symbol kind
  //
  switch (Decl->Kind) {
    case AST_DECL_VAR:
      SymbolKind = SYMBOL_VAR;
      break;
    case AST_DECL_FUNCTION:
      SymbolKind = SYMBOL_FUNCTION;
      break;
    case AST_DECL_TYPEDEF:
      SymbolKind = SYMBOL_TYPEDEF;
      break;
    case AST_DECL_STRUCT:
      SymbolKind = SYMBOL_STRUCT;
      break;
    case AST_DECL_UNION:
      SymbolKind = SYMBOL_UNION;
      break;
    case AST_DECL_ENUM:
      SymbolKind = SYMBOL_ENUM;
      break;
    case AST_DECL_ENUMERATOR:
      SymbolKind = SYMBOL_ENUMERATOR;
      break;
    default:
      return MMIX_SUCCESS;
  }

  //
  // Add symbol to current scope
  //
  if (Decl->Name != NULL) {
    SYMBOL  *Sym = SemaAddSymbol (Context, Decl->Name, SymbolKind, Decl->Type, Decl);
    if (Sym == NULL && Context->ErrorCount > 0) {
      return MMIX_ERROR_INVALID_PARAMETER;
    }
  }

  //
  // Check function body
  //
  if (Decl->Kind == AST_DECL_FUNCTION && Decl->Function.Body != NULL) {
    //
    // Enter function scope
    //
    SCOPE  *FuncScope = SemaEnterScope (Context);
    if (FuncScope == NULL) {
      return MMIX_ERROR_INVALID_PARAMETER;
    }
    FuncScope->IsFunction = TRUE;

    //
    // Add parameters to scope
    //
    if (Decl->Type != NULL && Decl->Type->Kind == AST_TYPE_FUNCTION) {
      for (UINT32 i = 0; i < Decl->Type->Function.ParameterCount; i++) {
        AST_DECL  *Param = Decl->Type->Function.Parameters[i];
        if (Param != NULL && Param->Name != NULL) {
          SemaAddSymbol (Context, Param->Name, SYMBOL_VAR, Param->Type, Param);
        }
      }
    }

    //
    // Set current function
    //
    Context->CurrentFunction = Decl->Type;

    //
    // Check function body
    //
    SemaCheckStatement (Context, Decl->Function.Body);

    Context->CurrentFunction = NULL;

    //
    // Exit function scope
    //
    SemaExitScope (Context);
  }

  return MMIX_SUCCESS;
}

/**
  Perform type checking on statement.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in,out]  Stmt          Statement to check.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaCheckStatement (
  IN OUT SEMA_CONTEXT  *Context,
  IN OUT AST_STMT      *Stmt
  )
{
  if (Stmt == NULL) {
    return MMIX_SUCCESS;
  }

  switch (Stmt->Kind) {
    case AST_STMT_COMPOUND:
      //
      // Enter new scope for compound statement
      //
      SemaEnterScope (Context);
      for (UINT32 i = 0; i < Stmt->Compound.StatementCount; i++) {
        SemaCheckStatement (Context, Stmt->Compound.Statements[i]);
      }
      SemaExitScope (Context);
      break;

    case AST_STMT_EXPR:
      if (Stmt->Expr.Expression != NULL) {
        SemaCheckExpression (Context, Stmt->Expr.Expression);
      }
      break;

    case AST_STMT_IF:
      SemaCheckExpression (Context, Stmt->If.Condition);
      SemaCheckStatement (Context, Stmt->If.ThenBranch);
      if (Stmt->If.ElseBranch != NULL) {
        SemaCheckStatement (Context, Stmt->If.ElseBranch);
      }
      break;

    case AST_STMT_WHILE:
      Context->InLoop = TRUE;
      SemaCheckExpression (Context, Stmt->While.Condition);
      SemaCheckStatement (Context, Stmt->While.Body);
      Context->InLoop = FALSE;
      break;

    case AST_STMT_FOR:
      SemaEnterScope (Context);
      Context->InLoop = TRUE;
      if (Stmt->For.Initializer != NULL) {
        SemaCheckStatement (Context, Stmt->For.Initializer);
      }
      if (Stmt->For.Condition != NULL) {
        SemaCheckExpression (Context, Stmt->For.Condition);
      }
      if (Stmt->For.Increment != NULL) {
        SemaCheckExpression (Context, Stmt->For.Increment);
      }
      SemaCheckStatement (Context, Stmt->For.Body);
      Context->InLoop = FALSE;
      SemaExitScope (Context);
      break;

    case AST_STMT_RETURN:
      if (Stmt->Return.Value != NULL) {
        SemaCheckExpression (Context, Stmt->Return.Value);
        //
        // TODO: Check return type compatibility
        //
      }
      break;

    case AST_STMT_BREAK:
      if (!Context->InLoop && !Context->InSwitch) {
        SemaError (Context, &Stmt->Location, "break statement not within loop or switch");
      }
      break;

    case AST_STMT_CONTINUE:
      if (!Context->InLoop) {
        SemaError (Context, &Stmt->Location, "continue statement not within loop");
      }
      break;

    default:
      break;
  }

  return MMIX_SUCCESS;
}

/**
  Perform type checking on expression.

  @param[in,out]  Context       Semantic analyzer context.
  @param[in,out]  Expr          Expression to check.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaCheckExpression (
  IN OUT SEMA_CONTEXT  *Context,
  IN OUT AST_EXPR      *Expr
  )
{
  if (Expr == NULL) {
    return MMIX_SUCCESS;
  }

  switch (Expr->Kind) {
    case AST_EXPR_IDENTIFIER:
      {
        SYMBOL  *Sym = SemaLookupSymbol (Context, Expr->Identifier.Name);
        if (Sym == NULL) {
          SemaError (Context, &Expr->Location, "Undeclared identifier");
          return MMIX_ERROR_INVALID_PARAMETER;
        }
        Expr->Type = Sym->Type;
        Expr->Identifier.Decl = Sym->Decl;
      }
      break;

    case AST_EXPR_BINARY:
      SemaCheckExpression (Context, Expr->Binary.Left);
      SemaCheckExpression (Context, Expr->Binary.Right);
      //
      // TODO: Determine result type based on operator
      //
      break;

    case AST_EXPR_UNARY:
      SemaCheckExpression (Context, Expr->Unary.Operand);
      //
      // TODO: Determine result type based on operator
      //
      break;

    case AST_EXPR_CALL:
      SemaCheckExpression (Context, Expr->Call.Callee);
      for (UINT32 i = 0; i < Expr->Call.ArgumentCount; i++) {
        SemaCheckExpression (Context, Expr->Call.Arguments[i]);
      }
      //
      // TODO: Check function type and argument compatibility
      //
      break;

    case AST_EXPR_INDEX:
      SemaCheckExpression (Context, Expr->Index.Array);
      SemaCheckExpression (Context, Expr->Index.Index);
      break;

    case AST_EXPR_MEMBER:
      SemaCheckExpression (Context, Expr->Member.Object);
      //
      // TODO: Look up member in struct/union type
      //
      break;

    default:
      break;
  }

  return MMIX_SUCCESS;
}

/**
  Perform semantic analysis on translation unit.

  @param[in,out]  Context       Semantic analyzer context.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
SemaAnalyze (
  IN OUT SEMA_CONTEXT  *Context
  )
{
  if (Context == NULL || Context->TranslationUnit == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Analyze each top-level declaration
  //
  for (UINT32 i = 0; i < Context->TranslationUnit->DeclarationCount; i++) {
    SemaCheckDeclaration (Context, Context->TranslationUnit->Declarations[i]);
  }

  if (Context->ErrorCount > 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  return MMIX_SUCCESS;
}
