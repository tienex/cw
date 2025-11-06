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
      return 0;
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
    Stmt->Compound.StatementCount = 0;
    Stmt->Compound.Statements = NULL;

    while (!ParserExpect (Parser, TOK_RBRACE) && !ParserExpect (Parser, TOK_EOF)) {
      AST_STMT  *SubStmt = ParserParseStatement (Parser);
      if (SubStmt != NULL) {
        // Add to list (simplified)
        Stmt->Compound.StatementCount++;
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
  // Expression statement
  //
  AST_EXPR  *Expr = ParserParseExpression (Parser);
  ParserConsume (Parser, TOK_SEMICOLON);

  AST_STMT  *Stmt = AstStmtCreate (AST_STMT_EXPR, &Tok->Location);
  Stmt->Expr.Expression = Expr;
  return Stmt;
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
  // TODO: Implement declaration parsing
  ParserError (Parser, "Declaration parsing not yet implemented");
  return NULL;
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
  TOKEN  *Tok = Parser->CurrentToken;

  //
  // Basic types
  //
  if (Tok->Type == TOK_INT) {
    ParserAdvance (Parser);
    return AstTypeCreate (AST_TYPE_INT);
  }

  if (Tok->Type == TOK_VOID) {
    ParserAdvance (Parser);
    return AstTypeCreate (AST_TYPE_VOID);
  }

  // TODO: Implement full type parsing

  return NULL;
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

  Unit = (AST_TRANSLATION_UNIT *)calloc (1, sizeof (AST_TRANSLATION_UNIT));
  if (Unit == NULL) {
    return NULL;
  }

  // Parse top-level declarations
  while (!ParserExpect (Parser, TOK_EOF)) {
    AST_DECL  *Decl = ParserParseDeclaration (Parser);
    if (Decl != NULL) {
      // Add to translation unit
      Unit->DeclarationCount++;
    }

    if (Parser->ErrorCount > 0) {
      break;
    }
  }

  Parser->TranslationUnit = Unit;
  return Unit;
}
