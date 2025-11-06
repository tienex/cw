/** @file
  MMIX C Compiler AST Implementation.

  This file implements AST node creation and management.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/MmixAst.h"

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
  )
{
  AST_EXPR  *Expr;

  Expr = (AST_EXPR *)calloc (1, sizeof (AST_EXPR));
  if (Expr == NULL) {
    return NULL;
  }

  Expr->Kind = Kind;
  if (Location != NULL) {
    Expr->Location = *Location;
  }

  return Expr;
}

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
  )
{
  AST_STMT  *Stmt;

  Stmt = (AST_STMT *)calloc (1, sizeof (AST_STMT));
  if (Stmt == NULL) {
    return NULL;
  }

  Stmt->Kind = Kind;
  if (Location != NULL) {
    Stmt->Location = *Location;
  }

  return Stmt;
}

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
  )
{
  AST_DECL  *Decl;

  Decl = (AST_DECL *)calloc (1, sizeof (AST_DECL));
  if (Decl == NULL) {
    return NULL;
  }

  Decl->Kind = Kind;
  if (Location != NULL) {
    Decl->Location = *Location;
  }

  if (Name != NULL) {
    Decl->Name = strdup (Name);
    if (Decl->Name == NULL) {
      free (Decl);
      return NULL;
    }
  }

  return Decl;
}

/**
  Create a new type node.

  @param[in]      Kind          Type kind.

  @return  Pointer to type node, or NULL on error.

**/
AST_TYPE *
AstTypeCreate (
  IN  AST_NODE_KIND  Kind
  )
{
  AST_TYPE  *Type;

  Type = (AST_TYPE *)calloc (1, sizeof (AST_TYPE));
  if (Type == NULL) {
    return NULL;
  }

  Type->Kind = Kind;
  return Type;
}

/**
  Destroy an expression node (recursive).

  @param[in]      Expr          Expression to destroy.

**/
STATIC
VOID
AstExprDestroy (
  IN  AST_EXPR  *Expr
  )
{
  if (Expr == NULL) {
    return;
  }

  switch (Expr->Kind) {
    case AST_EXPR_STRING:
      if (Expr->String.Value != NULL) {
        free (Expr->String.Value);
      }
      break;

    case AST_EXPR_IDENTIFIER:
      if (Expr->Identifier.Name != NULL) {
        free (Expr->Identifier.Name);
      }
      break;

    case AST_EXPR_BINARY:
      AstExprDestroy (Expr->Binary.Left);
      AstExprDestroy (Expr->Binary.Right);
      break;

    case AST_EXPR_UNARY:
      AstExprDestroy (Expr->Unary.Operand);
      break;

    case AST_EXPR_CALL:
      AstExprDestroy (Expr->Call.Callee);
      for (UINT32 i = 0; i < Expr->Call.ArgumentCount; i++) {
        AstExprDestroy (Expr->Call.Arguments[i]);
      }
      if (Expr->Call.Arguments != NULL) {
        free (Expr->Call.Arguments);
      }
      break;

    case AST_EXPR_CONDITIONAL:
      AstExprDestroy (Expr->Conditional.Condition);
      AstExprDestroy (Expr->Conditional.ThenExpr);
      AstExprDestroy (Expr->Conditional.ElseExpr);
      break;

    default:
      break;
  }

  free (Expr);
}

/**
  Destroy an AST node (recursive).

  @param[in]      Node          AST node to destroy.

**/
VOID
AstNodeDestroy (
  IN  VOID  *Node
  )
{
  if (Node == NULL) {
    return;
  }

  AST_NODE_KIND  *Kind = (AST_NODE_KIND *)Node;

  if (*Kind >= AST_EXPR_INTEGER && *Kind <= AST_EXPR_TYPEOF) {
    AstExprDestroy ((AST_EXPR *)Node);
  } else if (*Kind >= AST_STMT_COMPOUND && *Kind <= AST_STMT_NULL) {
    // TODO: Implement statement destruction
    free (Node);
  } else if (*Kind >= AST_DECL_VAR && *Kind <= AST_DECL_FIELD) {
    // TODO: Implement declaration destruction
    free (Node);
  } else {
    free (Node);
  }
}

/**
  Create integer literal expression.

  @param[in]      Location      Source location.
  @param[in]      Value         Integer value.
  @param[in]      IsUnsigned    TRUE if unsigned.
  @param[in]      BitWidth      Explicit bit width (0 = unspecified).

  @return  Pointer to expression node.

**/
AST_EXPR *
AstExprCreateInteger (
  IN  CONST TOKEN_LOCATION  *Location,
  IN  UINT64                Value,
  IN  BOOLEAN               IsUnsigned,
  IN  UINT32                BitWidth
  )
{
  AST_EXPR  *Expr;

  Expr = AstExprCreate (AST_EXPR_INTEGER, Location);
  if (Expr == NULL) {
    return NULL;
  }

  Expr->Integer.Value = Value;
  Expr->Integer.IsUnsigned = IsUnsigned;
  Expr->Integer.BitWidth = BitWidth;

  return Expr;
}

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
  )
{
  AST_EXPR  *Expr;

  Expr = AstExprCreate (AST_EXPR_BINARY, Location);
  if (Expr == NULL) {
    return NULL;
  }

  Expr->Binary.Op = Op;
  Expr->Binary.Left = Left;
  Expr->Binary.Right = Right;

  return Expr;
}

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
  )
{
  AST_EXPR  *Expr;

  Expr = AstExprCreate (AST_EXPR_IDENTIFIER, Location);
  if (Expr == NULL) {
    return NULL;
  }

  Expr->Identifier.Name = strdup (Name);
  if (Expr->Identifier.Name == NULL) {
    free (Expr);
    return NULL;
  }

  return Expr;
}
