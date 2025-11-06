/** @file
  MMIX C Compiler Parser Implementation.

  This file implements recursive descent parsing for C23.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/MmixParser.h"

/**
  Advance to next token.

  @param[in,out]  Parser        Parser state.

**/
STATIC
VOID
ParserAdvance (
  IN OUT PARSER_STATE  *Parser
  )
{
  if (Parser->PeekToken != NULL) {
    TokenDestroy (Parser->CurrentToken);
    Parser->CurrentToken = Parser->PeekToken;
    Parser->PeekToken = NULL;
  } else {
    TokenDestroy (Parser->CurrentToken);
    Parser->CurrentToken = LexerNextToken (Parser->Lexer);
  }
}

/**
  Create a new parser.

  @param[in]      Lexer         Lexer state.

  @return  Pointer to parser state, or NULL on error.

**/
PARSER_STATE *
ParserCreate (
  IN  LEXER_STATE  *Lexer
  )
{
  PARSER_STATE  *Parser;

  if (Lexer == NULL) {
    return NULL;
  }

  Parser = (PARSER_STATE *)calloc (1, sizeof (PARSER_STATE));
  if (Parser == NULL) {
    return NULL;
  }

  Parser->Lexer = Lexer;
  Parser->CurrentToken = LexerNextToken (Lexer);

  return Parser;
}

/**
  Destroy a parser.

  @param[in]      Parser        Parser state.

**/
VOID
ParserDestroy (
  IN  PARSER_STATE  *Parser
  )
{
  if (Parser == NULL) {
    return;
  }

  if (Parser->CurrentToken != NULL) {
    TokenDestroy (Parser->CurrentToken);
  }

  if (Parser->PeekToken != NULL) {
    TokenDestroy (Parser->PeekToken);
  }

  free (Parser);
}

/**
  Report a parser error.

  @param[in,out]  Parser        Parser state.
  @param[in]      Message       Error message.

**/
VOID
ParserError (
  IN OUT PARSER_STATE  *Parser,
  IN     CONST CHAR8   *Message
  )
{
  if (Parser == NULL || Message == NULL) {
    return;
  }

  TOKEN_LOCATION  *Loc = &Parser->CurrentToken->Location;
  snprintf (Parser->ErrorMessage, sizeof (Parser->ErrorMessage),
           "%s:%u:%u: error: %s",
           Loc->FileName, Loc->Line, Loc->Column, Message);
  Parser->ErrorCount++;

  fprintf (stderr, "%s\n", Parser->ErrorMessage);
}

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
  )
{
  return (Parser->CurrentToken != NULL && Parser->CurrentToken->Type == Type);
}

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
  )
{
  if (!ParserExpect (Parser, Type)) {
    CHAR8  ErrorMsg[128];
    snprintf (ErrorMsg, sizeof (ErrorMsg), "Expected '%s' but got '%s'",
             TokenTypeName (Type),
             TokenTypeName (Parser->CurrentToken->Type));
    ParserError (Parser, ErrorMsg);
    return FALSE;
  }

  ParserAdvance (Parser);
  return TRUE;
}

//
// Forward declarations for recursive parsing
//
STATIC AST_EXPR *ParsePrimaryExpression (IN OUT PARSER_STATE *Parser);
STATIC AST_EXPR *ParsePostfixExpression (IN OUT PARSER_STATE *Parser);
STATIC AST_EXPR *ParseUnaryExpression (IN OUT PARSER_STATE *Parser);
STATIC AST_EXPR *ParseBinaryExpression (IN OUT PARSER_STATE *Parser, INT32 MinPrecedence);
STATIC AST_EXPR *ParseAssignmentExpression (IN OUT PARSER_STATE *Parser);

/**
  Get operator precedence.

  @param[in]      Type          Token type.

  @return  Precedence level (higher = tighter binding).

**/
STATIC
INT32
GetOperatorPrecedence (
  IN  TOKEN_TYPE  Type
  )
{
  switch (Type) {
    case TOK_STAR:
    case TOK_SLASH:
    case TOK_PERCENT:
      return 13;

    case TOK_PLUS:
    case TOK_MINUS:
      return 12;

    case TOK_SHIFT_LEFT:
    case TOK_SHIFT_RIGHT:
      return 11;

    case TOK_LESS:
    case TOK_LESS_EQUAL:
    case TOK_GREATER:
    case TOK_GREATER_EQUAL:
      return 10;

    case TOK_EQUAL_EQUAL:
    case TOK_NOT_EQUAL:
      return 9;

    case TOK_AMPERSAND:
      return 8;

    case TOK_CARET:
      return 7;

    case TOK_PIPE:
      return 6;

    case TOK_LOGICAL_AND:
      return 5;

    case TOK_LOGICAL_OR:
      return 4;

    default:
      // Return negative precedence for non-operators
      // so they don't get parsed as binary operators
      return -1;
  }
}

/**
  Convert token type to binary operator.

  @param[in]      Type          Token type.

  @return  Binary operator.

**/
STATIC
BINARY_OPERATOR
TokenToBinaryOp (
  IN  TOKEN_TYPE  Type
  )
{
  switch (Type) {
    case TOK_PLUS:              return BIN_OP_ADD;
    case TOK_MINUS:             return BIN_OP_SUB;
    case TOK_STAR:              return BIN_OP_MUL;
    case TOK_SLASH:             return BIN_OP_DIV;
    case TOK_PERCENT:           return BIN_OP_MOD;
    case TOK_AMPERSAND:         return BIN_OP_BIT_AND;
    case TOK_PIPE:              return BIN_OP_BIT_OR;
    case TOK_CARET:             return BIN_OP_BIT_XOR;
    case TOK_SHIFT_LEFT:        return BIN_OP_SHIFT_LEFT;
    case TOK_SHIFT_RIGHT:       return BIN_OP_SHIFT_RIGHT;
    case TOK_LOGICAL_AND:       return BIN_OP_LOGICAL_AND;
    case TOK_LOGICAL_OR:        return BIN_OP_LOGICAL_OR;
    case TOK_EQUAL_EQUAL:       return BIN_OP_EQ;
    case TOK_NOT_EQUAL:         return BIN_OP_NE;
    case TOK_LESS:              return BIN_OP_LT;
    case TOK_LESS_EQUAL:        return BIN_OP_LE;
    case TOK_GREATER:           return BIN_OP_GT;
    case TOK_GREATER_EQUAL:     return BIN_OP_GE;
    case TOK_EQUAL:             return BIN_OP_ASSIGN;
    case TOK_PLUS_EQUAL:        return BIN_OP_ADD_ASSIGN;
    case TOK_MINUS_EQUAL:       return BIN_OP_SUB_ASSIGN;
    case TOK_STAR_EQUAL:        return BIN_OP_MUL_ASSIGN;
    case TOK_SLASH_EQUAL:       return BIN_OP_DIV_ASSIGN;
    case TOK_COMMA:             return BIN_OP_COMMA;
    default:                    return BIN_OP_ADD;  // Default
  }
}

/**
  Parse primary expression.

  @param[in,out]  Parser        Parser state.

  @return  Expression node, or NULL on error.

**/
STATIC
AST_EXPR *
ParsePrimaryExpression (
  IN OUT PARSER_STATE  *Parser
  )
{
  TOKEN  *Tok = Parser->CurrentToken;


  //
  // Integer literal
  //
  if (Tok->Type == TOK_INTEGER) {
    AST_EXPR  *Expr = AstExprCreateInteger (&Tok->Location, 0, FALSE);
    // TODO: Parse actual integer value from token text
    ParserAdvance (Parser);
    return Expr;
  }

  //
  // String literal
  //
  if (Tok->Type == TOK_STRING || Tok->Type == TOK_RAW_STRING) {
    AST_EXPR  *Expr = AstExprCreate (AST_EXPR_STRING, &Tok->Location);
    if (Expr != NULL) {
      Expr->String.Value = strdup (Tok->Text);
      Expr->String.Length = Tok->TextLength;
    }
    ParserAdvance (Parser);
    return Expr;
  }

  //
  // Identifier
  //
  if (Tok->Type == TOK_IDENTIFIER) {
    AST_EXPR  *Expr = AstExprCreateIdentifier (&Tok->Location, Tok->Text);
    ParserAdvance (Parser);
    return Expr;
  }

  //
  // Parenthesized expression
  //
  if (Tok->Type == TOK_LPAREN) {
    ParserAdvance (Parser);
    AST_EXPR  *Expr = ParserParseExpression (Parser);
    ParserConsume (Parser, TOK_RPAREN);
    return Expr;
  }

  //
  // Statement expression (GNU extension)
  //
  if (Tok->Type == TOK_LPAREN && Parser->Lexer->Options.EnableGnuExtensions) {
    // Check for ({ ... })
    // TODO: Implement statement expressions
  }

  ParserError (Parser, "Expected expression");
  return NULL;
}

/**
  Parse postfix expression.

  @param[in,out]  Parser        Parser state.

  @return  Expression node, or NULL on error.

**/
STATIC
AST_EXPR *
ParsePostfixExpression (
  IN OUT PARSER_STATE  *Parser
  )
{
  AST_EXPR  *Expr = ParsePrimaryExpression (Parser);
  if (Expr == NULL) {
    return NULL;
  }

  while (TRUE) {
    TOKEN  *Tok = Parser->CurrentToken;

    //
    // Array subscript: expr[index]
    //
    if (Tok->Type == TOK_LBRACKET) {
      ParserAdvance (Parser);
      AST_EXPR  *Index = ParserParseExpression (Parser);
      ParserConsume (Parser, TOK_RBRACKET);

      AST_EXPR  *NewExpr = AstExprCreate (AST_EXPR_INDEX, &Tok->Location);
      NewExpr->Index.Array = Expr;
      NewExpr->Index.Index = Index;
      Expr = NewExpr;
      continue;
    }

    //
    // Function call: expr(args)
    //
    if (Tok->Type == TOK_LPAREN) {
      ParserAdvance (Parser);

      AST_EXPR  *NewExpr = AstExprCreate (AST_EXPR_CALL, &Tok->Location);
      NewExpr->Call.Callee = Expr;

      // Parse arguments
      UINT32  ArgCapacity = 8;
      NewExpr->Call.Arguments = (AST_EXPR **)malloc (ArgCapacity * sizeof (AST_EXPR *));
      NewExpr->Call.ArgumentCount = 0;

      if (!ParserExpect (Parser, TOK_RPAREN)) {
        while (TRUE) {
          AST_EXPR  *Arg = ParseAssignmentExpression (Parser);
          if (Arg == NULL) {
            break;
          }

          if (NewExpr->Call.ArgumentCount >= ArgCapacity) {
            ArgCapacity *= 2;
            NewExpr->Call.Arguments = (AST_EXPR **)realloc (
              NewExpr->Call.Arguments,
              ArgCapacity * sizeof (AST_EXPR *)
            );
          }

          NewExpr->Call.Arguments[NewExpr->Call.ArgumentCount++] = Arg;

          if (!ParserExpect (Parser, TOK_COMMA)) {
            break;
          }
          ParserAdvance (Parser);
        }
      }

      ParserConsume (Parser, TOK_RPAREN);
      Expr = NewExpr;
      continue;
    }

    //
    // Member access: expr.member or expr->member
    //
    if (Tok->Type == TOK_DOT || Tok->Type == TOK_ARROW) {
      BOOLEAN  IsArrow = (Tok->Type == TOK_ARROW);
      ParserAdvance (Parser);

      if (!ParserExpect (Parser, TOK_IDENTIFIER)) {
        ParserError (Parser, "Expected member name");
        return NULL;
      }

      AST_EXPR  *NewExpr = AstExprCreate (AST_EXPR_MEMBER, &Tok->Location);
      NewExpr->Member.Object = Expr;
      NewExpr->Member.MemberName = strdup (Parser->CurrentToken->Text);
      NewExpr->Member.IsArrow = IsArrow;
      ParserAdvance (Parser);

      Expr = NewExpr;
      continue;
    }

    //
    // Postfix increment/decrement
    //
    if (Tok->Type == TOK_PLUS_PLUS || Tok->Type == TOK_MINUS_MINUS) {
      AST_EXPR  *NewExpr = AstExprCreate (AST_EXPR_UNARY, &Tok->Location);
      NewExpr->Unary.Op = (Tok->Type == TOK_PLUS_PLUS) ? UN_OP_POST_INC : UN_OP_POST_DEC;
      NewExpr->Unary.Operand = Expr;
      ParserAdvance (Parser);
      Expr = NewExpr;
      continue;
    }

    break;
  }

  return Expr;
}

/**
  Parse unary expression.

  @param[in,out]  Parser        Parser state.

  @return  Expression node, or NULL on error.

**/
STATIC
AST_EXPR *
ParseUnaryExpression (
  IN OUT PARSER_STATE  *Parser
  )
{
  TOKEN  *Tok = Parser->CurrentToken;

  //
  // Prefix increment/decrement
  //
  if (Tok->Type == TOK_PLUS_PLUS || Tok->Type == TOK_MINUS_MINUS) {
    ParserAdvance (Parser);
    AST_EXPR  *Operand = ParseUnaryExpression (Parser);
    AST_EXPR  *Expr = AstExprCreate (AST_EXPR_UNARY, &Tok->Location);
    Expr->Unary.Op = (Tok->Type == TOK_PLUS_PLUS) ? UN_OP_PRE_INC : UN_OP_PRE_DEC;
    Expr->Unary.Operand = Operand;
    return Expr;
  }

  //
  // Unary operators
  //
  if (Tok->Type == TOK_AMPERSAND || Tok->Type == TOK_STAR ||
      Tok->Type == TOK_PLUS || Tok->Type == TOK_MINUS ||
      Tok->Type == TOK_TILDE || Tok->Type == TOK_EXCLAIM) {
    ParserAdvance (Parser);
    AST_EXPR  *Operand = ParseUnaryExpression (Parser);
    AST_EXPR  *Expr = AstExprCreate (AST_EXPR_UNARY, &Tok->Location);

    if (Tok->Type == TOK_AMPERSAND) {
      Expr->Unary.Op = UN_OP_ADDRESS_OF;
    } else if (Tok->Type == TOK_STAR) {
      Expr->Unary.Op = UN_OP_DEREF;
    } else if (Tok->Type == TOK_PLUS) {
      Expr->Unary.Op = UN_OP_PLUS;
    } else if (Tok->Type == TOK_MINUS) {
      Expr->Unary.Op = UN_OP_MINUS;
    } else if (Tok->Type == TOK_TILDE) {
      Expr->Unary.Op = UN_OP_BIT_NOT;
    } else if (Tok->Type == TOK_EXCLAIM) {
      Expr->Unary.Op = UN_OP_LOGICAL_NOT;
    }

    Expr->Unary.Operand = Operand;
    return Expr;
  }

  //
  // sizeof operator
  //
  if (Tok->Type == TOK_SIZEOF) {
    ParserAdvance (Parser);
    AST_EXPR  *Expr = AstExprCreate (AST_EXPR_SIZEOF, &Tok->Location);
    // TODO: Parse sizeof operand (type or expression)
    return Expr;
  }

  //
  // _Alignof operator
  //
  if (Tok->Type == TOK_ALIGNOF) {
    ParserAdvance (Parser);
    AST_EXPR  *Expr = AstExprCreate (AST_EXPR_ALIGNOF, &Tok->Location);
    // TODO: Parse alignof operand
    return Expr;
  }

  return ParsePostfixExpression (Parser);
}

/**
  Parse binary expression with precedence climbing.

  @param[in,out]  Parser        Parser state.
  @param[in]      MinPrecedence Minimum precedence level.

  @return  Expression node, or NULL on error.

**/
STATIC
AST_EXPR *
ParseBinaryExpression (
  IN OUT PARSER_STATE  *Parser,
  IN     INT32         MinPrecedence
  )
{
  AST_EXPR  *Left = ParseUnaryExpression (Parser);
  if (Left == NULL) {
    return NULL;
  }

  while (TRUE) {
    TOKEN_TYPE  OpType = Parser->CurrentToken->Type;
    INT32       Precedence = GetOperatorPrecedence (OpType);

    if (Precedence < MinPrecedence) {
      break;
    }

    TOKEN_LOCATION  OpLoc = Parser->CurrentToken->Location;
    ParserAdvance (Parser);

    AST_EXPR  *Right = ParseBinaryExpression (Parser, Precedence + 1);
    if (Right == NULL) {
      return NULL;
    }

    BINARY_OPERATOR  Op = TokenToBinaryOp (OpType);
    Left = AstExprCreateBinary (&OpLoc, Op, Left, Right);
  }

  return Left;
}

/**
  Parse assignment expression.

  @param[in,out]  Parser        Parser state.

  @return  Expression node, or NULL on error.

**/
STATIC
AST_EXPR *
ParseAssignmentExpression (
  IN OUT PARSER_STATE  *Parser
  )
{
  AST_EXPR  *Expr = ParseBinaryExpression (Parser, 0);

  //
  // Check for ternary conditional operator
  //
  if (ParserExpect (Parser, TOK_QUESTION)) {
    TOKEN_LOCATION  Loc = Parser->CurrentToken->Location;
    ParserAdvance (Parser);

    AST_EXPR  *ThenExpr = ParserParseExpression (Parser);
    ParserConsume (Parser, TOK_COLON);
    AST_EXPR  *ElseExpr = ParseAssignmentExpression (Parser);

    AST_EXPR  *CondExpr = AstExprCreate (AST_EXPR_CONDITIONAL, &Loc);
    CondExpr->Conditional.Condition = Expr;
    CondExpr->Conditional.ThenExpr = ThenExpr;
    CondExpr->Conditional.ElseExpr = ElseExpr;

    return CondExpr;
  }

  return Expr;
}

/**
  Parse an expression.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to expression node, or NULL on error.

**/
AST_EXPR *
ParserParseExpression (
  IN OUT PARSER_STATE  *Parser
  )
{
  return ParseAssignmentExpression (Parser);
}

/**
  Parse a statement.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to statement node, or NULL on error.

**/
AST_STMT *
ParserParseStatement (
  IN OUT PARSER_STATE  *Parser
  )
{
  TOKEN  *Tok = Parser->CurrentToken;

  //
  // Compound statement
  //
  if (Tok->Type == TOK_LBRACE) {
    ParserAdvance (Parser);
    AST_STMT  *Stmt = AstStmtCreate (AST_STMT_COMPOUND, &Tok->Location);

    // Parse statements
    UINT32  Capacity = 16;
    Stmt->Compound.Statements = (AST_STMT **)malloc (Capacity * sizeof (AST_STMT *));
    Stmt->Compound.StatementCount = 0;

    if (Stmt->Compound.Statements == NULL) {
      return NULL;
    }

    while (!ParserExpect (Parser, TOK_RBRACE) && !ParserExpect (Parser, TOK_EOF)) {
      AST_STMT  *SubStmt = ParserParseStatement (Parser);
      if (SubStmt != NULL) {
        // Grow array if needed
        if (Stmt->Compound.StatementCount >= Capacity) {
          Capacity *= 2;
          AST_STMT  **NewStmts = (AST_STMT **)realloc (
            Stmt->Compound.Statements,
            Capacity * sizeof (AST_STMT *)
          );
          if (NewStmts == NULL) {
            free (Stmt->Compound.Statements);
            return NULL;
          }
          Stmt->Compound.Statements = NewStmts;
        }

        // Add statement to array
        Stmt->Compound.Statements[Stmt->Compound.StatementCount++] = SubStmt;
      } else {
        break;
      }
    }

    ParserConsume (Parser, TOK_RBRACE);
    return Stmt;
  }

  //
  // Return statement
  //
  if (Tok->Type == TOK_RETURN) {
    ParserAdvance (Parser);
    AST_STMT  *Stmt = AstStmtCreate (AST_STMT_RETURN, &Tok->Location);

    if (!ParserExpect (Parser, TOK_SEMICOLON)) {
      Stmt->Return.Value = ParserParseExpression (Parser);
    }

    ParserConsume (Parser, TOK_SEMICOLON);
    return Stmt;
  }

  //
  // If statement
  //
  if (Tok->Type == TOK_IF) {
    ParserAdvance (Parser);
    AST_STMT  *Stmt = AstStmtCreate (AST_STMT_IF, &Tok->Location);

    ParserConsume (Parser, TOK_LPAREN);
    Stmt->If.Condition = ParserParseExpression (Parser);
    ParserConsume (Parser, TOK_RPAREN);

    Stmt->If.ThenBranch = ParserParseStatement (Parser);

    if (ParserExpect (Parser, TOK_ELSE)) {
      ParserAdvance (Parser);
      Stmt->If.ElseBranch = ParserParseStatement (Parser);
    }

    return Stmt;
  }

  //
  // While loop
  //
  if (Tok->Type == TOK_WHILE) {
    ParserAdvance (Parser);
    AST_STMT  *Stmt = AstStmtCreate (AST_STMT_WHILE, &Tok->Location);

    ParserConsume (Parser, TOK_LPAREN);
    Stmt->While.Condition = ParserParseExpression (Parser);
    ParserConsume (Parser, TOK_RPAREN);

    Stmt->While.Body = ParserParseStatement (Parser);
    return Stmt;
  }

  //
  // Expression statement or empty statement
  //
  if (ParserExpect (Parser, TOK_SEMICOLON)) {
    // Empty statement
    ParserAdvance (Parser);
    return AstStmtCreate (AST_STMT_NULL, &Tok->Location);
  }

  // Try to parse expression statement
  AST_EXPR  *Expr = ParserParseExpression (Parser);
  if (Expr == NULL) {
    return NULL;
  }

  ParserConsume (Parser, TOK_SEMICOLON);

  AST_STMT  *Stmt = AstStmtCreate (AST_STMT_EXPR, &Tok->Location);
  Stmt->Expr.Expression = Expr;
  return Stmt;
}

/**
  Check if token is a type qualifier.

  @param[in]      Type          Token type.

  @return  TRUE if type qualifier, FALSE otherwise.

**/
STATIC
BOOLEAN
IsTypeQualifier (
  IN  TOKEN_TYPE  Type
  )
{
  return (Type == TOK_CONST || Type == TOK_VOLATILE || Type == TOK_RESTRICT ||
          Type == TOK_ATOMIC);
}

/**
  Check if token is a storage class specifier.

  @param[in]      Type          Token type.

  @return  TRUE if storage class, FALSE otherwise.

**/
STATIC
BOOLEAN
IsStorageClassSpecifier (
  IN  TOKEN_TYPE  Type
  )
{
  return (Type == TOK_AUTO || Type == TOK_REGISTER || Type == TOK_STATIC ||
          Type == TOK_EXTERN || Type == TOK_TYPEDEF || Type == TOK_THREAD_LOCAL);
}

/**
  Check if token is a type specifier.

  @param[in]      Type          Token type.

  @return  TRUE if type specifier, FALSE otherwise.

**/
STATIC
BOOLEAN
IsTypeSpecifier (
  IN  TOKEN_TYPE  Type
  )
{
  return (Type == TOK_VOID || Type == TOK_CHAR || Type == TOK_SHORT ||
          Type == TOK_INT || Type == TOK_LONG || Type == TOK_FLOAT ||
          Type == TOK_DOUBLE || Type == TOK_SIGNED || Type == TOK_UNSIGNED ||
          Type == TOK_BOOL || Type == TOK_COMPLEX || Type == TOK_IMAGINARY ||
          Type == TOK_STRUCT || Type == TOK_UNION || Type == TOK_ENUM ||
          Type == TOK_TYPEOF || Type == TOK_TYPEOF_UNQUAL || Type == TOK_TYPEOF_GNU ||
          Type == TOK_BITINT || Type == TOK_INT128 || Type == TOK_FLOAT128 ||
          Type == TOK_INT8 || Type == TOK_INT16 || Type == TOK_INT32 || Type == TOK_INT64);
}

/**
  Parse type qualifiers.

  @param[in,out]  Parser        Parser state.
  @param[out]     IsConst       Pointer to receive const flag.
  @param[out]     IsVolatile    Pointer to receive volatile flag.
  @param[out]     IsRestrict    Pointer to receive restrict flag.
  @param[out]     IsAtomic      Pointer to receive atomic flag.

**/
STATIC
VOID
ParseTypeQualifiers (
  IN OUT PARSER_STATE  *Parser,
  OUT    BOOLEAN       *IsConst,
  OUT    BOOLEAN       *IsVolatile,
  OUT    BOOLEAN       *IsRestrict,
  OUT    BOOLEAN       *IsAtomic
  )
{
  *IsConst = FALSE;
  *IsVolatile = FALSE;
  *IsRestrict = FALSE;
  *IsAtomic = FALSE;

  while (IsTypeQualifier (Parser->CurrentToken->Type)) {
    switch (Parser->CurrentToken->Type) {
      case TOK_CONST:
        *IsConst = TRUE;
        break;
      case TOK_VOLATILE:
        *IsVolatile = TRUE;
        break;
      case TOK_RESTRICT:
        *IsRestrict = TRUE;
        break;
      case TOK_ATOMIC:
        *IsAtomic = TRUE;
        break;
      default:
        break;
    }
    ParserAdvance (Parser);
  }
}

//
// Forward declarations for mutual recursion
//
STATIC AST_TYPE * ParseTypeSpecifiers (IN OUT PARSER_STATE *Parser);
STATIC AST_TYPE * ParseDeclarator (IN OUT PARSER_STATE *Parser, IN AST_TYPE *BaseType, OUT CHAR8 **Name);

/**
  Parse struct or union declaration.

  @param[in,out]  Parser        Parser state.
  @param[in]      IsUnion       TRUE for union, FALSE for struct.

  @return  Pointer to type node, or NULL on error.

**/
STATIC
AST_TYPE *
ParseStructOrUnion (
  IN OUT PARSER_STATE  *Parser,
  IN     BOOLEAN       IsUnion
  )
{
  TOKEN_LOCATION  Loc = Parser->CurrentToken->Location;
  ParserAdvance (Parser);  // Skip 'struct' or 'union'

  CHAR8  *Tag = NULL;

  //
  // Optional tag name
  //
  if (ParserExpect (Parser, TOK_IDENTIFIER)) {
    Tag = strdup (Parser->CurrentToken->Text);
    ParserAdvance (Parser);
  }

  //
  // Check for definition (has body)
  //
  if (!ParserExpect (Parser, TOK_LBRACE)) {
    //
    // Forward declaration or reference
    //
    AST_TYPE  *Type = AstTypeCreate (IsUnion ? AST_TYPE_UNION : AST_TYPE_STRUCT);
    Type->Struct.Name = Tag;
    Type->Struct.Fields = NULL;
    Type->Struct.FieldCount = 0;
    Type->Struct.IsComplete = FALSE;
    return Type;
  }

  //
  // Parse struct/union body
  //
  ParserAdvance (Parser);  // Skip '{'

  AST_TYPE  *Type = AstTypeCreate (IsUnion ? AST_TYPE_UNION : AST_TYPE_STRUCT);
  Type->Struct.Name = Tag;
  Type->Struct.FieldCount = 0;
  Type->Struct.IsComplete = TRUE;

  //
  // Parse field declarations
  //
  UINT32      FieldCapacity = 8;
  AST_DECL  **Fields = (AST_DECL **)malloc (FieldCapacity * sizeof (AST_DECL *));
  UINT32      FieldCount = 0;

  while (!ParserExpect (Parser, TOK_RBRACE) && !ParserExpect (Parser, TOK_EOF)) {
    //
    // Parse type specifiers for field
    //
    BOOLEAN  IsConst, IsVolatile, IsRestrict, IsAtomic;
    ParseTypeQualifiers (Parser, &IsConst, &IsVolatile, &IsRestrict, &IsAtomic);

    AST_TYPE  *FieldBaseType = ParseTypeSpecifiers (Parser);
    if (FieldBaseType == NULL) {
      ParserError (Parser, "Expected type specifier in struct/union field");
      break;
    }

    //
    // Apply qualifiers
    //
    if (IsConst || IsVolatile || IsRestrict || IsAtomic) {
      AST_TYPE  *QualType = AstTypeCreate (AST_TYPE_QUALIFIED);
      QualType->Qualified.BaseType = FieldBaseType;
      QualType->Qualified.IsConst = IsConst;
      QualType->Qualified.IsVolatile = IsVolatile;
      QualType->Qualified.IsRestrict = IsRestrict;
      QualType->Qualified.IsAtomic = IsAtomic;
      FieldBaseType = QualType;
    }

    //
    // Parse declarator(s) - can have multiple fields of same type
    //
    while (TRUE) {
      CHAR8     *FieldName = NULL;
      AST_TYPE  *FieldType = ParseDeclarator (Parser, FieldBaseType, &FieldName);

      //
      // Create field declaration
      //
      TOKEN_LOCATION  FieldLoc = Parser->CurrentToken->Location;
      AST_DECL  *Field = AstDeclCreate (AST_DECL_FIELD, &FieldLoc, FieldName);
      Field->Type = FieldType;
      Field->StorageClass = STORAGE_NONE;

      //
      // Check for bit-field
      //
      if (ParserExpect (Parser, TOK_COLON)) {
        ParserAdvance (Parser);
        Field->Field.BitWidth = ParseAssignmentExpression (Parser);
      } else {
        Field->Field.BitWidth = NULL;
      }

      //
      // Add to fields array
      //
      if (FieldCount >= FieldCapacity) {
        FieldCapacity *= 2;
        Fields = (AST_DECL **)realloc (Fields, FieldCapacity * sizeof (AST_DECL *));
      }
      Fields[FieldCount++] = Field;

      //
      // Check for comma (multiple declarators)
      //
      if (ParserExpect (Parser, TOK_COMMA)) {
        ParserAdvance (Parser);
        continue;
      }

      break;
    }

    ParserConsume (Parser, TOK_SEMICOLON);
  }

  Type->Struct.Fields = Fields;
  Type->Struct.FieldCount = FieldCount;

  ParserConsume (Parser, TOK_RBRACE);
  return Type;
}

/**
  Parse enum declaration.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to type node, or NULL on error.

**/
STATIC
AST_TYPE *
ParseEnum (
  IN OUT PARSER_STATE  *Parser
  )
{
  TOKEN_LOCATION  Loc = Parser->CurrentToken->Location;
  ParserAdvance (Parser);  // Skip 'enum'

  CHAR8  *Tag = NULL;

  //
  // Optional tag name
  //
  if (ParserExpect (Parser, TOK_IDENTIFIER)) {
    Tag = strdup (Parser->CurrentToken->Text);
    ParserAdvance (Parser);
  }

  //
  // Check for definition (has body)
  //
  if (!ParserExpect (Parser, TOK_LBRACE)) {
    //
    // Forward declaration or reference
    //
    AST_TYPE  *Type = AstTypeCreate (AST_TYPE_ENUM);
    Type->Enum.Name = Tag;
    Type->Enum.Enumerators = NULL;
    Type->Enum.EnumeratorCount = 0;
    Type->Enum.UnderlyingType = NULL;
    return Type;
  }

  //
  // Parse enum body
  //
  ParserAdvance (Parser);  // Skip '{'

  AST_TYPE  *Type = AstTypeCreate (AST_TYPE_ENUM);
  Type->Enum.Name = Tag;
  Type->Enum.UnderlyingType = NULL;

  //
  // Parse enumerators
  //
  UINT32      EnumCapacity = 8;
  AST_DECL  **Enumerators = (AST_DECL **)malloc (EnumCapacity * sizeof (AST_DECL *));
  UINT32      EnumCount = 0;

  while (!ParserExpect (Parser, TOK_RBRACE) && !ParserExpect (Parser, TOK_EOF)) {
    //
    // Enumerator must have an identifier
    //
    if (!ParserExpect (Parser, TOK_IDENTIFIER)) {
      ParserError (Parser, "Expected enumerator name");
      break;
    }

    TOKEN_LOCATION  EnumLoc = Parser->CurrentToken->Location;
    CHAR8          *EnumName = strdup (Parser->CurrentToken->Text);
    ParserAdvance (Parser);

    //
    // Optional value assignment
    //
    AST_EXPR  *Value = NULL;
    if (ParserExpect (Parser, TOK_EQUAL)) {
      ParserAdvance (Parser);
      Value = ParseAssignmentExpression (Parser);
    }

    //
    // Create enumerator declaration
    //
    AST_DECL  *Enumerator = AstDeclCreate (AST_DECL_ENUMERATOR, &EnumLoc, EnumName);
    Enumerator->Type = Type;  // Reference to parent enum type
    Enumerator->StorageClass = STORAGE_NONE;
    Enumerator->Enumerator.Value = Value;

    //
    // Add to enumerators array
    //
    if (EnumCount >= EnumCapacity) {
      EnumCapacity *= 2;
      Enumerators = (AST_DECL **)realloc (Enumerators, EnumCapacity * sizeof (AST_DECL *));
    }
    Enumerators[EnumCount++] = Enumerator;

    //
    // Check for comma
    //
    if (ParserExpect (Parser, TOK_COMMA)) {
      ParserAdvance (Parser);
      //
      // Allow trailing comma before '}'
      //
      if (ParserExpect (Parser, TOK_RBRACE)) {
        break;
      }
      continue;
    }

    break;
  }

  Type->Enum.Enumerators = Enumerators;
  Type->Enum.EnumeratorCount = EnumCount;

  ParserConsume (Parser, TOK_RBRACE);
  return Type;
}

/**
  Parse base type specifiers.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to type node, or NULL on error.

**/
STATIC
AST_TYPE *
ParseTypeSpecifiers (
  IN OUT PARSER_STATE  *Parser
  )
{
  BOOLEAN  HasVoid = FALSE;
  BOOLEAN  HasChar = FALSE;
  BOOLEAN  HasShort = FALSE;
  BOOLEAN  HasInt = FALSE;
  BOOLEAN  HasLong = FALSE;
  BOOLEAN  HasLongLong = FALSE;
  BOOLEAN  HasFloat = FALSE;
  BOOLEAN  HasDouble = FALSE;
  BOOLEAN  HasSigned = FALSE;
  BOOLEAN  HasUnsigned = FALSE;
  BOOLEAN  HasBool = FALSE;
  BOOLEAN  HasComplex = FALSE;
  BOOLEAN  HasImaginary = FALSE;


  //
  // Parse type specifiers (can have multiple tokens like 'unsigned long long')
  //
  while (IsTypeSpecifier (Parser->CurrentToken->Type)) {
    switch (Parser->CurrentToken->Type) {
      case TOK_VOID:
        HasVoid = TRUE;
        break;
      case TOK_CHAR:
        HasChar = TRUE;
        break;
      case TOK_SHORT:
        HasShort = TRUE;
        break;
      case TOK_INT:
        HasInt = TRUE;
        break;
      case TOK_LONG:
        if (HasLong) {
          HasLongLong = TRUE;
        }
        HasLong = TRUE;
        break;
      case TOK_FLOAT:
        HasFloat = TRUE;
        break;
      case TOK_DOUBLE:
        HasDouble = TRUE;
        break;
      case TOK_SIGNED:
        HasSigned = TRUE;
        break;
      case TOK_UNSIGNED:
        HasUnsigned = TRUE;
        break;
      case TOK_BOOL:
        HasBool = TRUE;
        break;
      case TOK_COMPLEX:
        HasComplex = TRUE;
        break;
      case TOK_IMAGINARY:
        HasImaginary = TRUE;
        break;

      case TOK_STRUCT:
        return ParseStructOrUnion (Parser, FALSE);

      case TOK_UNION:
        return ParseStructOrUnion (Parser, TRUE);

      case TOK_ENUM:
        return ParseEnum (Parser);

      case TOK_INT8:
        ParserAdvance (Parser);
        return AstTypeCreate (AST_TYPE_INT8);
        break;

      case TOK_INT16:
        ParserAdvance (Parser);
        return AstTypeCreate (AST_TYPE_INT16);
        break;

      case TOK_INT32:
        ParserAdvance (Parser);
        return AstTypeCreate (AST_TYPE_INT32);
        break;

      case TOK_INT64:
        ParserAdvance (Parser);
        return AstTypeCreate (AST_TYPE_INT64);
        break;

      case TOK_INT128:
        ParserAdvance (Parser);
        return AstTypeCreate (AST_TYPE_INT128);
        break;

      case TOK_FLOAT128:
        ParserAdvance (Parser);
        return AstTypeCreate (AST_TYPE_FLOAT128);
        break;

      case TOK_TYPEOF:
      case TOK_TYPEOF_UNQUAL:
      case TOK_TYPEOF_GNU:
        // TODO: Implement typeof
        ParserError (Parser, "typeof not yet implemented");
        return NULL;

      case TOK_BITINT:
        // TODO: Implement _BitInt(N)
        ParserError (Parser, "_BitInt not yet implemented");
        return NULL;

      default:
        break;
    }

    ParserAdvance (Parser);
  }

  //
  // Determine final type from combination
  //
  if (HasVoid) {
    return AstTypeCreate (AST_TYPE_VOID);
  }

  if (HasBool) {
    return AstTypeCreate (AST_TYPE_BOOL);
  }

  if (HasChar) {
    if (HasUnsigned) {
      return AstTypeCreate (AST_TYPE_UCHAR);
    }
    return AstTypeCreate (AST_TYPE_CHAR);
  }

  if (HasFloat) {
    if (HasComplex) {
      return AstTypeCreate (AST_TYPE_FLOAT_COMPLEX);
    }
    return AstTypeCreate (AST_TYPE_FLOAT);
  }

  if (HasDouble) {
    if (HasLong) {
      return AstTypeCreate (AST_TYPE_LONG_DOUBLE);
    }
    if (HasComplex) {
      return AstTypeCreate (AST_TYPE_DOUBLE_COMPLEX);
    }
    return AstTypeCreate (AST_TYPE_DOUBLE);
  }

  if (HasShort) {
    if (HasUnsigned) {
      return AstTypeCreate (AST_TYPE_USHORT);
    }
    return AstTypeCreate (AST_TYPE_SHORT);
  }

  if (HasLongLong) {
    if (HasUnsigned) {
      return AstTypeCreate (AST_TYPE_ULONG_LONG);
    }
    return AstTypeCreate (AST_TYPE_LONG_LONG);
  }

  if (HasLong) {
    if (HasUnsigned) {
      return AstTypeCreate (AST_TYPE_ULONG);
    }
    return AstTypeCreate (AST_TYPE_LONG);
  }

  //
  // Default to int (can be 'int', 'signed', 'unsigned', or implied)
  //
  if (HasUnsigned) {
    return AstTypeCreate (AST_TYPE_UINT);
  }

  return AstTypeCreate (AST_TYPE_INT);
}

/**
  Parse declarator and apply it to base type.

  @param[in,out]  Parser        Parser state.
  @param[in]      BaseType      Base type to modify.
  @param[out]     Name          Pointer to receive declarator name.

  @return  Pointer to complete type, or NULL on error.

**/
STATIC
AST_TYPE *
ParseDeclarator (
  IN OUT PARSER_STATE  *Parser,
  IN     AST_TYPE      *BaseType,
  OUT    CHAR8         **Name
  )
{
  AST_TYPE  *Type = BaseType;
  *Name = NULL;

  //
  // Parse pointer prefix (* ...)
  //
  while (ParserExpect (Parser, TOK_STAR)) {
    ParserAdvance (Parser);

    //
    // Parse qualifiers after *
    //
    BOOLEAN  IsConst, IsVolatile, IsRestrict, IsAtomic;
    ParseTypeQualifiers (Parser, &IsConst, &IsVolatile, &IsRestrict, &IsAtomic);

    AST_TYPE  *PtrType = AstTypeCreate (AST_TYPE_POINTER);
    PtrType->Pointer.PointeeType = Type;
    PtrType->Pointer.IsRestrict = IsRestrict;

    //
    // Apply const/volatile/atomic qualifiers to the pointer type
    //
    if (IsConst) {
      PtrType->Qualifiers |= QUAL_CONST;
    }
    if (IsVolatile) {
      PtrType->Qualifiers |= QUAL_VOLATILE;
    }
    if (IsAtomic) {
      PtrType->Qualifiers |= QUAL_ATOMIC;
    }

    Type = PtrType;
  }

  //
  // Parse direct declarator (name, arrays, functions, parentheses)
  //
  if (ParserExpect (Parser, TOK_IDENTIFIER)) {
    *Name = strdup (Parser->CurrentToken->Text);
    ParserAdvance (Parser);
  } else if (ParserExpect (Parser, TOK_LPAREN)) {
    //
    // Parenthesized declarator
    //
    ParserAdvance (Parser);
    Type = ParseDeclarator (Parser, Type, Name);
    ParserConsume (Parser, TOK_RPAREN);
  }

  //
  // Parse postfix (arrays and functions)
  //
  while (TRUE) {
    if (ParserExpect (Parser, TOK_LBRACKET)) {
      //
      // Array declarator
      //
      ParserAdvance (Parser);

      AST_EXPR  *Size = NULL;
      if (!ParserExpect (Parser, TOK_RBRACKET)) {
        Size = ParseAssignmentExpression (Parser);
      }

      ParserConsume (Parser, TOK_RBRACKET);

      AST_TYPE  *ArrayType = AstTypeCreate (AST_TYPE_ARRAY);
      ArrayType->Array.ElementType = Type;
      ArrayType->Array.Size = Size;
      Type = ArrayType;
    } else if (ParserExpect (Parser, TOK_LPAREN)) {
      //
      // Function declarator
      //
      ParserAdvance (Parser);

      AST_TYPE  *FuncType = AstTypeCreate (AST_TYPE_FUNCTION);
      FuncType->Function.ReturnType = Type;
      FuncType->Function.Parameters = NULL;
      FuncType->Function.ParameterCount = 0;
      FuncType->Function.IsVariadic = FALSE;

      //
      // Parse parameters
      //
      if (!ParserExpect (Parser, TOK_RPAREN)) {
        UINT32      ParamCapacity = 8;
        AST_DECL  **Parameters = (AST_DECL **)malloc (ParamCapacity * sizeof (AST_DECL *));
        UINT32      ParamCount = 0;

        while (TRUE) {
          //
          // Check for variadic (...)
          //
          if (ParserExpect (Parser, TOK_ELLIPSIS)) {
            FuncType->Function.IsVariadic = TRUE;
            ParserAdvance (Parser);
            break;
          }

          //
          // Parse parameter type
          //
          BOOLEAN  IsConst, IsVolatile, IsRestrict, IsAtomic;
          ParseTypeQualifiers (Parser, &IsConst, &IsVolatile, &IsRestrict, &IsAtomic);

          AST_TYPE  *ParamBaseType = ParseTypeSpecifiers (Parser);
          if (ParamBaseType == NULL) {
            ParserError (Parser, "Expected parameter type");
            break;
          }

          //
          // Apply qualifiers
          //
          if (IsConst || IsVolatile || IsRestrict || IsAtomic) {
            AST_TYPE  *QualType = AstTypeCreate (AST_TYPE_QUALIFIED);
            QualType->Qualified.BaseType = ParamBaseType;
            QualType->Qualified.IsConst = IsConst;
            QualType->Qualified.IsVolatile = IsVolatile;
            QualType->Qualified.IsRestrict = IsRestrict;
            QualType->Qualified.IsAtomic = IsAtomic;
            ParamBaseType = QualType;
          }

          //
          // Parse declarator (parameter name is optional)
          //
          CHAR8     *ParamName = NULL;
          AST_TYPE  *ParamType = ParseDeclarator (Parser, ParamBaseType, &ParamName);

          //
          // Create parameter declaration
          //
          TOKEN_LOCATION  ParamLoc = Parser->CurrentToken->Location;
          AST_DECL  *Param = AstDeclCreate (AST_DECL_VAR, &ParamLoc, ParamName);
          Param->Type = ParamType;
          Param->StorageClass = STORAGE_NONE;
          Param->Var.Initializer = NULL;

          //
          // Add to parameters array
          //
          if (ParamCount >= ParamCapacity) {
            ParamCapacity *= 2;
            Parameters = (AST_DECL **)realloc (Parameters, ParamCapacity * sizeof (AST_DECL *));
          }
          Parameters[ParamCount++] = Param;

          //
          // Check for comma
          //
          if (ParserExpect (Parser, TOK_COMMA)) {
            ParserAdvance (Parser);
            continue;
          }

          break;
        }

        FuncType->Function.Parameters = Parameters;
        FuncType->Function.ParameterCount = ParamCount;
      }

      ParserConsume (Parser, TOK_RPAREN);
      Type = FuncType;
    } else {
      break;
    }
  }

  return Type;
}

/**
  Parse a declaration.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to declaration node, or NULL on error.

**/
AST_DECL *
ParserParseDeclaration (
  IN OUT PARSER_STATE  *Parser
  )
{
  TOKEN_LOCATION  Loc = Parser->CurrentToken->Location;


  //
  // Parse storage class specifiers
  //
  BOOLEAN  IsStatic = FALSE;
  BOOLEAN  IsExtern = FALSE;
  BOOLEAN  IsTypedef = FALSE;
  BOOLEAN  IsAuto = FALSE;
  BOOLEAN  IsRegister = FALSE;

  while (IsStorageClassSpecifier (Parser->CurrentToken->Type)) {
    switch (Parser->CurrentToken->Type) {
      case TOK_STATIC:
        IsStatic = TRUE;
        break;
      case TOK_EXTERN:
        IsExtern = TRUE;
        break;
      case TOK_TYPEDEF:
        IsTypedef = TRUE;
        break;
      case TOK_AUTO:
        IsAuto = TRUE;
        break;
      case TOK_REGISTER:
        IsRegister = TRUE;
        break;
      default:
        break;
    }
    ParserAdvance (Parser);
  }

  //
  // Parse type qualifiers
  //
  BOOLEAN  IsConst, IsVolatile, IsRestrict, IsAtomic;
  ParseTypeQualifiers (Parser, &IsConst, &IsVolatile, &IsRestrict, &IsAtomic);

  //
  // Parse type specifiers
  //
  AST_TYPE  *BaseType = ParseTypeSpecifiers (Parser);
  if (BaseType == NULL) {
    ParserError (Parser, "Expected type specifier");
    return NULL;
  }

  //
  // Apply qualifiers to type
  //
  if (IsConst || IsVolatile || IsRestrict || IsAtomic) {
    AST_TYPE  *QualType = AstTypeCreate (AST_TYPE_QUALIFIED);
    QualType->Qualified.BaseType = BaseType;
    QualType->Qualified.IsConst = IsConst;
    QualType->Qualified.IsVolatile = IsVolatile;
    QualType->Qualified.IsRestrict = IsRestrict;
    QualType->Qualified.IsAtomic = IsAtomic;
    BaseType = QualType;
  }

  //
  // Parse declarator
  //
  CHAR8     *Name = NULL;
  AST_TYPE  *Type = ParseDeclarator (Parser, BaseType, &Name);

  //
  // Determine storage class
  //
  STORAGE_CLASS  StorageClass = STORAGE_NONE;
  if (IsTypedef) {
    StorageClass = STORAGE_TYPEDEF;
  } else if (IsStatic) {
    StorageClass = STORAGE_STATIC;
  } else if (IsExtern) {
    StorageClass = STORAGE_EXTERN;
  } else if (IsRegister) {
    StorageClass = STORAGE_REGISTER;
  } else if (IsAuto) {
    StorageClass = STORAGE_AUTO;
  }

  //
  // Create declaration node
  //
  AST_DECL  *Decl = NULL;

  if (Type->Kind == AST_TYPE_FUNCTION) {
    //
    // Function declaration
    //
    Decl = AstDeclCreate (AST_DECL_FUNCTION, &Loc, Name);
    Decl->Type = Type;
    Decl->StorageClass = StorageClass;
    Decl->Function.Parameters = Type->Function.Parameters;
    Decl->Function.ParameterCount = Type->Function.ParameterCount;
    Decl->Function.IsVariadic = Type->Function.IsVariadic;
    Decl->Function.Body = NULL;
    Decl->Function.IsInline = FALSE;

    //
    // Check for function body
    //
    if (ParserExpect (Parser, TOK_LBRACE)) {
      Decl->Function.Body = ParserParseStatement (Parser);
      Decl->Function.IsDefinition = TRUE;
    } else {
      ParserConsume (Parser, TOK_SEMICOLON);
      Decl->Function.IsDefinition = FALSE;
    }
  } else {
    //
    // Variable declaration
    //
    Decl = AstDeclCreate (AST_DECL_VAR, &Loc, Name);
    Decl->Type = Type;
    Decl->StorageClass = StorageClass;
    Decl->Var.Initializer = NULL;

    //
    // Check for initializer
    //
    if (ParserExpect (Parser, TOK_EQUAL)) {
      ParserAdvance (Parser);
      Decl->Var.Initializer = ParseAssignmentExpression (Parser);
    }

    ParserConsume (Parser, TOK_SEMICOLON);
  }

  return Decl;
}

/**
  Parse a type specifier.

  @param[in,out]  Parser        Parser state.

  @return  Pointer to type node, or NULL on error.

**/
AST_TYPE *
ParserParseType (
  IN OUT PARSER_STATE  *Parser
  )
{
  return ParseTypeSpecifiers (Parser);
}

/**
  Parse a translation unit (entire file).

  @param[in,out]  Parser        Parser state.

  @return  Pointer to translation unit AST, or NULL on error.

**/
AST_TRANSLATION_UNIT *
ParserParseTranslationUnit (
  IN OUT PARSER_STATE  *Parser
  )
{
  AST_TRANSLATION_UNIT  *Unit;
  UINT32                Capacity;

  Unit = (AST_TRANSLATION_UNIT *)calloc (1, sizeof (AST_TRANSLATION_UNIT));
  if (Unit == NULL) {
    return NULL;
  }

  // Allocate initial declaration array
  Capacity = 16;
  Unit->Declarations = (AST_DECL **)malloc (Capacity * sizeof (AST_DECL *));
  if (Unit->Declarations == NULL) {
    free (Unit);
    return NULL;
  }

  // Parse top-level declarations
  while (!ParserExpect (Parser, TOK_EOF)) {
    AST_DECL  *Decl = ParserParseDeclaration (Parser);
    if (Decl != NULL) {
      // Grow array if needed
      if (Unit->DeclarationCount >= Capacity) {
        Capacity *= 2;
        AST_DECL  **NewDecls = (AST_DECL **)realloc (Unit->Declarations, Capacity * sizeof (AST_DECL *));
        if (NewDecls == NULL) {
          // Cleanup and fail
          free (Unit->Declarations);
          free (Unit);
          return NULL;
        }
        Unit->Declarations = NewDecls;
      }

      // Add to translation unit
      Unit->Declarations[Unit->DeclarationCount] = Decl;
      Unit->DeclarationCount++;
    }

    if (Parser->ErrorCount > 0) {
      break;
    }
  }

  Parser->TranslationUnit = Unit;
  return Unit;
}
