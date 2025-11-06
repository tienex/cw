/**
  @file IrGen.c

  IR generation for MMIX C23 compiler.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/MmixIr.h"

//
// Forward declarations
//
STATIC VOID IrAddAddressTakenVar (IN OUT IR_FUNCTION *Func, IN CONST CHAR8 *Name);
STATIC BOOLEAN IrIsAddressTaken (IN IR_FUNCTION *Func, IN CONST CHAR8 *Name);

//
// Simple hash function for symbol table
//
STATIC
UINT32
HashString (
  IN  CONST CHAR8  *Str,
  IN  UINT32       TableSize
  )
{
  UINT32  Hash = 5381;
  INT32   C;

  while ((C = *Str++) != 0) {
    Hash = ((Hash << 5) + Hash) + C;  // hash * 33 + c
  }

  return Hash % TableSize;
}

/**
  Create symbol table.

  @param[in]      Size          Hash table size.

  @return  Pointer to symbol table, or NULL on error.

**/
IR_SYMBOL_TABLE *
IrCreateSymbolTable (
  IN  UINT32  Size
  )
{
  IR_SYMBOL_TABLE  *Table;

  Table = (IR_SYMBOL_TABLE *)calloc (1, sizeof (IR_SYMBOL_TABLE));
  if (Table == NULL) {
    return NULL;
  }

  Table->Size = Size;
  Table->Count = 0;
  Table->Entries = (IR_SYMBOL **)calloc (Size, sizeof (IR_SYMBOL *));
  if (Table->Entries == NULL) {
    free (Table);
    return NULL;
  }

  return Table;
}

/**
  Destroy symbol table.

  @param[in]      Table         Symbol table.

**/
VOID
IrDestroySymbolTable (
  IN  IR_SYMBOL_TABLE  *Table
  )
{
  if (Table == NULL) {
    return;
  }

  if (Table->Entries != NULL) {
    for (UINT32 i = 0; i < Table->Size; i++) {
      IR_SYMBOL  *Sym = Table->Entries[i];
      while (Sym != NULL) {
        IR_SYMBOL  *Next = Sym->Next;
        if (Sym->Name != NULL) {
          free (Sym->Name);
        }
        free (Sym);
        Sym = Next;
      }
    }
    free (Table->Entries);
  }

  free (Table);
}

/**
  Add symbol to symbol table.

  @param[in,out]  Table         Symbol table.
  @param[in]      Name          Symbol name.
  @param[in]      Operand       Operand for this symbol.
  @param[in]      Type          Symbol type.
  @param[in]      Decl          Original AST declaration.
  @param[in]      IsParameter   TRUE if function parameter.
  @param[in]      ParamIndex    Parameter index if parameter.

  @return  TRUE on success, FALSE on error.

**/
BOOLEAN
IrAddSymbol (
  IN OUT IR_SYMBOL_TABLE  *Table,
  IN     CONST CHAR8      *Name,
  IN     IR_OPERAND       Operand,
  IN     AST_TYPE         *Type,
  IN     AST_DECL         *Decl,
  IN     BOOLEAN          IsParameter,
  IN     UINT32           ParamIndex
  )
{
  IR_SYMBOL  *Sym;
  UINT32     Hash;

  if (Table == NULL || Name == NULL) {
    return FALSE;
  }

  //
  // Create new symbol
  //
  Sym = (IR_SYMBOL *)calloc (1, sizeof (IR_SYMBOL));
  if (Sym == NULL) {
    return FALSE;
  }

  Sym->Name = strdup (Name);
  Sym->Operand = Operand;
  Sym->Type = Type;
  Sym->Decl = Decl;
  Sym->IsParameter = IsParameter;
  Sym->ParamIndex = ParamIndex;

  //
  // Add to hash table
  //
  Hash = HashString (Name, Table->Size);
  Sym->Next = Table->Entries[Hash];
  Table->Entries[Hash] = Sym;
  Table->Count++;

  return TRUE;
}

/**
  Lookup symbol in symbol table.

  @param[in]      Table         Symbol table.
  @param[in]      Name          Symbol name.

  @return  Pointer to symbol, or NULL if not found.

**/
IR_SYMBOL *
IrLookupSymbol (
  IN  IR_SYMBOL_TABLE  *Table,
  IN  CONST CHAR8      *Name
  )
{
  UINT32      Hash;
  IR_SYMBOL   *Sym;

  if (Table == NULL || Name == NULL) {
    return NULL;
  }

  Hash = HashString (Name, Table->Size);
  Sym = Table->Entries[Hash];

  while (Sym != NULL) {
    if (strcmp (Sym->Name, Name) == 0) {
      return Sym;
    }
    Sym = Sym->Next;
  }

  return NULL;
}

/**
  Create IR module from AST.

  @param[in]      Ast           Translation unit AST.
  @param[in]      Mode32Bit     TRUE for 32-bit mode.

  @return  Pointer to IR module, or NULL on error.

**/
IR_MODULE *
IrCreateModule (
  IN  AST_TRANSLATION_UNIT  *Ast,
  IN  BOOLEAN               Mode32Bit
  )
{
  IR_MODULE  *Module;

  Module = (IR_MODULE *)calloc (1, sizeof (IR_MODULE));
  if (Module == NULL) {
    return NULL;
  }

  Module->Ast = Ast;
  Module->FunctionCount = 0;
  Module->Functions = NULL;
  Module->SourceFile = Ast->SourceFile;
  Module->Mode32Bit = Mode32Bit;

  return Module;
}

/**
  Destroy IR module.

  @param[in]      Module        IR module.

**/
VOID
IrDestroyModule (
  IN  IR_MODULE  *Module
  )
{
  if (Module == NULL) {
    return;
  }

  if (Module->Functions != NULL) {
    for (UINT32 i = 0; i < Module->FunctionCount; i++) {
      IrDestroyFunction (Module->Functions[i]);
    }
    free (Module->Functions);
  }

  free (Module);
}

/**
  Create IR function.

  @param[in]      Decl          Function declaration.

  @return  Pointer to IR function, or NULL on error.

**/
IR_FUNCTION *
IrCreateFunction (
  IN  AST_DECL  *Decl
  )
{
  IR_FUNCTION  *Func;

  Func = (IR_FUNCTION *)calloc (1, sizeof (IR_FUNCTION));
  if (Func == NULL) {
    return NULL;
  }

  Func->Name = Decl->Name;
  Func->Decl = Decl;
  Func->EntryBlock = NULL;
  Func->Blocks = NULL;
  Func->BlockCount = 0;
  Func->NextRegNum = 0;
  Func->NextLabelId = 0;
  Func->NextInstrId = 0;
  Func->AddressTakenVars = NULL;
  Func->AddressTakenCount = 0;
  Func->AddressTakenCapacity = 0;

  //
  // Get function type information
  //
  if (Decl->Type != NULL && Decl->Type->Kind == AST_TYPE_FUNCTION) {
    Func->ParamCount = Decl->Type->Function.ParameterCount;
    Func->ReturnType = Decl->Type->Function.ReturnType;
  } else {
    Func->ParamCount = 0;
    Func->ReturnType = NULL;
  }

  //
  // Create symbol table (size 64 should be enough for most functions)
  //
  Func->Symbols = IrCreateSymbolTable (64);
  if (Func->Symbols == NULL) {
    free (Func);
    return NULL;
  }

  return Func;
}

/**
  Destroy IR function.

  @param[in]      Func          IR function.

**/
VOID
IrDestroyFunction (
  IN  IR_FUNCTION  *Func
  )
{
  if (Func == NULL) {
    return;
  }

  //
  // Free all basic blocks
  //
  if (Func->Blocks != NULL) {
    for (UINT32 i = 0; i < Func->BlockCount; i++) {
      IR_BASIC_BLOCK  *Block = Func->Blocks[i];
      if (Block != NULL) {
        //
        // Free instructions
        //
        IR_INSTRUCTION  *Instr = Block->First;
        while (Instr != NULL) {
          IR_INSTRUCTION  *Next = Instr->Next;
          if (Instr->Args != NULL) {
            free (Instr->Args);
          }
          free (Instr);
          Instr = Next;
        }

        //
        // Free predecessor/successor arrays
        //
        if (Block->Preds != NULL) {
          free (Block->Preds);
        }
        if (Block->Succs != NULL) {
          free (Block->Succs);
        }

        if (Block->Label != NULL) {
          free (Block->Label);
        }

        free (Block);
      }
    }
    free (Func->Blocks);
  }

  //
  // Free symbol table
  //
  if (Func->Symbols != NULL) {
    IrDestroySymbolTable (Func->Symbols);
  }

  free (Func);
}

/**
  Create basic block.

  @param[in,out]  Func          IR function.
  @param[in]      Label         Block label (optional).

  @return  Pointer to basic block, or NULL on error.

**/
IR_BASIC_BLOCK *
IrCreateBasicBlock (
  IN OUT IR_FUNCTION  *Func,
  IN     CONST CHAR8  *Label
  )
{
  IR_BASIC_BLOCK  *Block;
  UINT32          NewCount;
  IR_BASIC_BLOCK  **NewBlocks;

  Block = (IR_BASIC_BLOCK *)calloc (1, sizeof (IR_BASIC_BLOCK));
  if (Block == NULL) {
    return NULL;
  }

  Block->Id = Func->NextLabelId++;
  if (Label != NULL) {
    Block->Label = strdup (Label);
  } else {
    CHAR8  LabelBuf[64];
    snprintf (LabelBuf, sizeof (LabelBuf), "BB%u", Block->Id);
    Block->Label = strdup (LabelBuf);
  }

  Block->First = NULL;
  Block->Last = NULL;
  Block->InstrCount = 0;
  Block->Preds = NULL;
  Block->PredCount = 0;
  Block->Succs = NULL;
  Block->SuccCount = 0;
  Block->IsEntry = FALSE;
  Block->IsExit = FALSE;
  Block->Next = NULL;

  //
  // Add to function's block list
  //
  NewCount = Func->BlockCount + 1;
  NewBlocks = (IR_BASIC_BLOCK **)realloc (Func->Blocks, NewCount * sizeof (IR_BASIC_BLOCK *));
  if (NewBlocks == NULL) {
    free (Block->Label);
    free (Block);
    return NULL;
  }

  Func->Blocks = NewBlocks;
  Func->Blocks[Func->BlockCount] = Block;
  Func->BlockCount = NewCount;

  return Block;
}

/**
  Create IR instruction.

  @param[in]      Opcode        Instruction opcode.

  @return  Pointer to instruction, or NULL on error.

**/
IR_INSTRUCTION *
IrCreateInstruction (
  IN  IR_OPCODE  Opcode
  )
{
  IR_INSTRUCTION  *Instr;

  Instr = (IR_INSTRUCTION *)calloc (1, sizeof (IR_INSTRUCTION));
  if (Instr == NULL) {
    return NULL;
  }

  Instr->Opcode = Opcode;
  Instr->Dst.Type = IR_OPERAND_NONE;
  Instr->Src1.Type = IR_OPERAND_NONE;
  Instr->Src2.Type = IR_OPERAND_NONE;
  Instr->Args = NULL;
  Instr->ArgCount = 0;
  Instr->LabelId = 0;
  Instr->LabelId2 = 0;
  Instr->Next = NULL;
  Instr->Prev = NULL;
  Instr->Id = 0;

  return Instr;
}

/**
  Append instruction to basic block.

  @param[in,out]  Block         Basic block.
  @param[in]      Instr         Instruction to append.

**/
VOID
IrAppendInstruction (
  IN OUT IR_BASIC_BLOCK  *Block,
  IN     IR_INSTRUCTION  *Instr
  )
{
  if (Block == NULL || Instr == NULL) {
    return;
  }

  Instr->Prev = Block->Last;
  Instr->Next = NULL;

  if (Block->Last != NULL) {
    Block->Last->Next = Instr;
  } else {
    Block->First = Instr;
  }

  Block->Last = Instr;
  Block->InstrCount++;
}

/**
  Add edge between basic blocks.

  @param[in,out]  From          Source block.
  @param[in,out]  To            Destination block.

**/
VOID
IrAddEdge (
  IN OUT IR_BASIC_BLOCK  *From,
  IN OUT IR_BASIC_BLOCK  *To
  )
{
  if (From == NULL || To == NULL) {
    return;
  }

  //
  // Add to From's successors
  //
  From->Succs = (IR_BASIC_BLOCK **)realloc (
    From->Succs,
    (From->SuccCount + 1) * sizeof (IR_BASIC_BLOCK *)
  );
  From->Succs[From->SuccCount++] = To;

  //
  // Add to To's predecessors
  //
  To->Preds = (IR_BASIC_BLOCK **)realloc (
    To->Preds,
    (To->PredCount + 1) * sizeof (IR_BASIC_BLOCK *)
  );
  To->Preds[To->PredCount++] = From;
}

/**
  Allocate virtual register.

  @param[in,out]  Func          IR function.
  @param[in]      Type          Register type.

  @return  Register operand.

**/
IR_OPERAND
IrAllocReg (
  IN OUT IR_FUNCTION  *Func,
  IN     AST_TYPE     *Type
  )
{
  IR_OPERAND  Reg;

  Reg.Type = IR_OPERAND_REG;
  Reg.RegNum = Func->NextRegNum++;
  Reg.DataType = Type;
  Reg.BitWidth = 0;  // Not applicable for registers

  return Reg;
}

/**
  Create constant operand.

  @param[in]      Value         Constant value.
  @param[in]      Type          Value type.
  @param[in]      BitWidth      Explicit bit width (0 = unspecified).

  @return  Constant operand.

**/
IR_OPERAND
IrConstant (
  IN  INT64     Value,
  IN  AST_TYPE  *Type,
  IN  UINT32    BitWidth
  )
{
  IR_OPERAND  Const;

  Const.Type = IR_OPERAND_CONST;
  Const.ConstValue = Value;
  Const.DataType = Type;
  Const.BitWidth = BitWidth;

  return Const;
}

/**
  Create symbol operand.

  @param[in]      Name          Symbol name.
  @param[in]      Type          Symbol type.

  @return  Symbol operand.

**/
IR_OPERAND
IrSymbol (
  IN  CONST CHAR8  *Name,
  IN  AST_TYPE     *Type
  )
{
  IR_OPERAND  Sym;

  Sym.Type = IR_OPERAND_SYMBOL;
  Sym.SymbolName = (CHAR8 *)Name;
  Sym.DataType = Type;
  Sym.BitWidth = 0;  // Not applicable for symbols

  return Sym;
}

/**
  Generate IR for binary expression.

  @param[in,out]  Context       IR context.
  @param[in]      Expr          Binary expression.

  @return  Operand containing result.

**/
STATIC
IR_OPERAND
IrGenBinaryExpr (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_EXPR    *Expr
  )
{
  IR_OPERAND      Left, Right, Result;
  IR_INSTRUCTION  *Instr;
  IR_OPCODE       Opcode;

  //
  // For assignments, handle left side specially
  // Don't evaluate if it's an array index, address-taken identifier, or dereference
  //
  BOOLEAN IsAssignment = (Expr->Binary.Op >= BIN_OP_ASSIGN && Expr->Binary.Op <= BIN_OP_SHR_ASSIGN);
  BOOLEAN IsAddressTakenId = FALSE;
  BOOLEAN IsDeref = FALSE;

  if (IsAssignment && Expr->Binary.Left->Kind == AST_EXPR_IDENTIFIER) {
    IsAddressTakenId = IrIsAddressTaken (Context->CurrentFunc, Expr->Binary.Left->Identifier.Name);
  } else if (IsAssignment && Expr->Binary.Left->Kind == AST_EXPR_UNARY &&
             Expr->Binary.Left->Unary.Op == UN_OP_DEREF) {
    IsDeref = TRUE;
  }

  if (IsAssignment && (Expr->Binary.Left->Kind == AST_EXPR_INDEX || IsAddressTakenId || IsDeref)) {
    // Special handling for array/address-taken/dereference assignment - don't evaluate left side yet
    Right = IrGenExpression (Context, Expr->Binary.Right);
  } else {
    // Normal case - evaluate both sides
    Left = IrGenExpression (Context, Expr->Binary.Left);
    Right = IrGenExpression (Context, Expr->Binary.Right);
  }

  //
  // Determine opcode based on operator
  //
  switch (Expr->Binary.Op) {
    case BIN_OP_ADD:        Opcode = IR_ADD; break;
    case BIN_OP_SUB:        Opcode = IR_SUB; break;
    case BIN_OP_MUL:        Opcode = IR_MUL; break;
    case BIN_OP_DIV:        Opcode = IR_DIV; break;
    case BIN_OP_MOD:        Opcode = IR_MOD; break;
    case BIN_OP_BIT_AND:    Opcode = IR_AND; break;
    case BIN_OP_BIT_OR:     Opcode = IR_OR; break;
    case BIN_OP_BIT_XOR:    Opcode = IR_XOR; break;
    case BIN_OP_BIT_CONCAT: Opcode = IR_BFCONCAT; break;
    case BIN_OP_SHIFT_LEFT:  Opcode = IR_SHL; break;
    case BIN_OP_SHIFT_RIGHT: Opcode = IR_SHR; break;
    case BIN_OP_ROTATE_LEFT:  Opcode = IR_ROL; break;
    case BIN_OP_ROTATE_RIGHT: Opcode = IR_ROR; break;
    case BIN_OP_EQ:         Opcode = IR_EQ; break;
    case BIN_OP_NE:         Opcode = IR_NE; break;
    case BIN_OP_LT:         Opcode = IR_LT; break;
    case BIN_OP_LE:         Opcode = IR_LE; break;
    case BIN_OP_GT:         Opcode = IR_GT; break;
    case BIN_OP_GE:         Opcode = IR_GE; break;
    case BIN_OP_LOGICAL_AND: Opcode = IR_LAND; break;
    case BIN_OP_LOGICAL_OR:  Opcode = IR_LOR; break;

    case BIN_OP_ASSIGN:
    case BIN_OP_ADD_ASSIGN:
    case BIN_OP_SUB_ASSIGN:
    case BIN_OP_MUL_ASSIGN:
    case BIN_OP_DIV_ASSIGN:
    case BIN_OP_MOD_ASSIGN:
    case BIN_OP_AND_ASSIGN:
    case BIN_OP_OR_ASSIGN:
    case BIN_OP_XOR_ASSIGN:
    case BIN_OP_SHL_ASSIGN:
    case BIN_OP_SHR_ASSIGN:
      //
      // Handle assignment operators
      // Special case for array indexing: arr[i] = value
      //
      if (Expr->Binary.Left->Kind == AST_EXPR_INDEX) {
        //
        // Array assignment: arr[i] = value
        // Compute address and store
        //
        IR_OPERAND  Array, Index, Addr, FinalValue;

        Array = IrGenExpression (Context, Expr->Binary.Left->Index.Array);
        Index = IrGenExpression (Context, Expr->Binary.Left->Index.Index);

        // Compute address: addr = array + index
        Addr = IrAllocReg (Context->CurrentFunc, Expr->Type);
        Instr = IrCreateInstruction (IR_ADD);
        Instr->Dst = Addr;
        Instr->Src1 = Array;
        Instr->Src2 = Index;
        IrAppendInstruction (Context->CurrentBlock, Instr);

        if (Expr->Binary.Op == BIN_OP_ASSIGN) {
          // Simple assignment
          FinalValue = Right;
        } else {
          // Compound assignment - need to load first
          IR_OPERAND  OldValue;
          IR_OPCODE   CompoundOp;

          switch (Expr->Binary.Op) {
            case BIN_OP_ADD_ASSIGN: CompoundOp = IR_ADD; break;
            case BIN_OP_SUB_ASSIGN: CompoundOp = IR_SUB; break;
            case BIN_OP_MUL_ASSIGN: CompoundOp = IR_MUL; break;
            case BIN_OP_DIV_ASSIGN: CompoundOp = IR_DIV; break;
            case BIN_OP_MOD_ASSIGN: CompoundOp = IR_MOD; break;
            case BIN_OP_AND_ASSIGN: CompoundOp = IR_AND; break;
            case BIN_OP_OR_ASSIGN:  CompoundOp = IR_OR; break;
            case BIN_OP_XOR_ASSIGN: CompoundOp = IR_XOR; break;
            case BIN_OP_SHL_ASSIGN: CompoundOp = IR_SHL; break;
            case BIN_OP_SHR_ASSIGN: CompoundOp = IR_SHR; break;
            default:                CompoundOp = IR_ADD; break;
          }

          // Load current value
          OldValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (IR_LOAD);
          Instr->Dst = OldValue;
          Instr->Src1 = Addr;
          IrAppendInstruction (Context->CurrentBlock, Instr);

          // Compute new value
          FinalValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (CompoundOp);
          Instr->Dst = FinalValue;
          Instr->Src1 = OldValue;
          Instr->Src2 = Right;
          IrAppendInstruction (Context->CurrentBlock, Instr);
        }

        // Store to array
        Instr = IrCreateInstruction (IR_STORE);
        Instr->Src1 = Addr;
        Instr->Src2 = FinalValue;
        IrAppendInstruction (Context->CurrentBlock, Instr);

        return FinalValue;
      } else if (Expr->Binary.Left->Kind == AST_EXPR_IDENTIFIER && IsAddressTakenId) {
        //
        // Address-taken variable assignment: var = value
        // Get address and store
        //
        IR_SYMBOL   *Sym;
        IR_OPERAND  VarAddr, FinalValue;

        Sym = IrLookupSymbol (Context->CurrentFunc->Symbols, Expr->Binary.Left->Identifier.Name);
        VarAddr = Sym->Operand;  // This is the address (from ALLOCA)

        if (Expr->Binary.Op == BIN_OP_ASSIGN) {
          // Simple assignment
          FinalValue = Right;
        } else {
          // Compound assignment - need to load first
          IR_OPERAND  OldValue;
          IR_OPCODE   CompoundOp;

          switch (Expr->Binary.Op) {
            case BIN_OP_ADD_ASSIGN: CompoundOp = IR_ADD; break;
            case BIN_OP_SUB_ASSIGN: CompoundOp = IR_SUB; break;
            case BIN_OP_MUL_ASSIGN: CompoundOp = IR_MUL; break;
            case BIN_OP_DIV_ASSIGN: CompoundOp = IR_DIV; break;
            case BIN_OP_MOD_ASSIGN: CompoundOp = IR_MOD; break;
            case BIN_OP_AND_ASSIGN: CompoundOp = IR_AND; break;
            case BIN_OP_OR_ASSIGN:  CompoundOp = IR_OR; break;
            case BIN_OP_XOR_ASSIGN: CompoundOp = IR_XOR; break;
            case BIN_OP_SHL_ASSIGN: CompoundOp = IR_SHL; break;
            case BIN_OP_SHR_ASSIGN: CompoundOp = IR_SHR; break;
            default:                CompoundOp = IR_ADD; break;
          }

          // Load current value
          OldValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (IR_LOAD);
          Instr->Dst = OldValue;
          Instr->Src1 = VarAddr;
          IrAppendInstruction (Context->CurrentBlock, Instr);

          // Compute new value
          FinalValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (CompoundOp);
          Instr->Dst = FinalValue;
          Instr->Src1 = OldValue;
          Instr->Src2 = Right;
          IrAppendInstruction (Context->CurrentBlock, Instr);
        }

        // Store to variable
        Instr = IrCreateInstruction (IR_STORE);
        Instr->Src1 = VarAddr;
        Instr->Src2 = FinalValue;
        IrAppendInstruction (Context->CurrentBlock, Instr);

        return FinalValue;
      } else if (Expr->Binary.Left->Kind == AST_EXPR_UNARY && IsDeref) {
        //
        // Dereference assignment: *ptr = value
        // Evaluate pointer to get address, then store
        //
        IR_OPERAND  PtrAddr, FinalValue;

        PtrAddr = IrGenExpression (Context, Expr->Binary.Left->Unary.Operand);

        if (Expr->Binary.Op == BIN_OP_ASSIGN) {
          // Simple assignment
          FinalValue = Right;
        } else {
          // Compound assignment - need to load first
          IR_OPERAND  OldValue;
          IR_OPCODE   CompoundOp;

          switch (Expr->Binary.Op) {
            case BIN_OP_ADD_ASSIGN: CompoundOp = IR_ADD; break;
            case BIN_OP_SUB_ASSIGN: CompoundOp = IR_SUB; break;
            case BIN_OP_MUL_ASSIGN: CompoundOp = IR_MUL; break;
            case BIN_OP_DIV_ASSIGN: CompoundOp = IR_DIV; break;
            case BIN_OP_MOD_ASSIGN: CompoundOp = IR_MOD; break;
            case BIN_OP_AND_ASSIGN: CompoundOp = IR_AND; break;
            case BIN_OP_OR_ASSIGN:  CompoundOp = IR_OR; break;
            case BIN_OP_XOR_ASSIGN: CompoundOp = IR_XOR; break;
            case BIN_OP_SHL_ASSIGN: CompoundOp = IR_SHL; break;
            case BIN_OP_SHR_ASSIGN: CompoundOp = IR_SHR; break;
            default:                CompoundOp = IR_ADD; break;
          }

          // Load current value
          OldValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (IR_LOAD);
          Instr->Dst = OldValue;
          Instr->Src1 = PtrAddr;
          IrAppendInstruction (Context->CurrentBlock, Instr);

          // Compute new value
          FinalValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (CompoundOp);
          Instr->Dst = FinalValue;
          Instr->Src1 = OldValue;
          Instr->Src2 = Right;
          IrAppendInstruction (Context->CurrentBlock, Instr);
        }

        // Store through pointer
        Instr = IrCreateInstruction (IR_STORE);
        Instr->Src1 = PtrAddr;
        Instr->Src2 = FinalValue;
        IrAppendInstruction (Context->CurrentBlock, Instr);

        return FinalValue;
      } else {
        //
        // Regular assignment to variable
        //
        IR_OPERAND  FinalValue;

        if (Expr->Binary.Op == BIN_OP_ASSIGN) {
          //
          // Simple assignment: y = x
          //
          FinalValue = Right;
        } else {
          //
          // Compound assignment: y += x
          // Need to load current value of left, perform operation, then store
          //
          IR_OPCODE  CompoundOp;

          switch (Expr->Binary.Op) {
            case BIN_OP_ADD_ASSIGN: CompoundOp = IR_ADD; break;
            case BIN_OP_SUB_ASSIGN: CompoundOp = IR_SUB; break;
            case BIN_OP_MUL_ASSIGN: CompoundOp = IR_MUL; break;
            case BIN_OP_DIV_ASSIGN: CompoundOp = IR_DIV; break;
            case BIN_OP_MOD_ASSIGN: CompoundOp = IR_MOD; break;
            case BIN_OP_AND_ASSIGN: CompoundOp = IR_AND; break;
            case BIN_OP_OR_ASSIGN:  CompoundOp = IR_OR; break;
            case BIN_OP_XOR_ASSIGN: CompoundOp = IR_XOR; break;
            case BIN_OP_SHL_ASSIGN: CompoundOp = IR_SHL; break;
            case BIN_OP_SHR_ASSIGN: CompoundOp = IR_SHR; break;
            default:                CompoundOp = IR_ADD; break;
          }

          //
          // Compute: temp = left op right
          //
          FinalValue = IrAllocReg (Context->CurrentFunc, Expr->Type);
          Instr = IrCreateInstruction (CompoundOp);
          Instr->Dst = FinalValue;
          Instr->Src1 = Left;
          Instr->Src2 = Right;
          IrAppendInstruction (Context->CurrentBlock, Instr);
        }

        //
        // Store result back to left side
        // Generate MOVE instruction: left = FinalValue
        //
        Instr = IrCreateInstruction (IR_MOVE);
        Instr->Dst = Left;
        Instr->Src1 = FinalValue;
        IrAppendInstruction (Context->CurrentBlock, Instr);

        //
        // Assignment expressions return the value that was assigned
        //
        return Left;
      }

    default:
      //
      // TODO: Handle other operators
      //
      Result.Type = IR_OPERAND_NONE;
      return Result;
  }

  //
  // Allocate result register
  //
  Result = IrAllocReg (Context->CurrentFunc, Expr->Type);

  //
  // Create instruction
  //
  Instr = IrCreateInstruction (Opcode);
  Instr->Dst = Result;
  Instr->Src1 = Left;
  Instr->Src2 = Right;

  IrAppendInstruction (Context->CurrentBlock, Instr);

  return Result;
}

/**
  Add variable name to address-taken list.

  @param[in,out]  Func          IR function.
  @param[in]      Name          Variable name.

**/
STATIC
VOID
IrAddAddressTakenVar (
  IN OUT IR_FUNCTION  *Func,
  IN     CONST CHAR8  *Name
  )
{
  CHAR8  **NewArray;
  UINT32 i;

  if (Func == NULL || Name == NULL) {
    return;
  }

  //
  // Check if already in list
  //
  for (i = 0; i < Func->AddressTakenCount; i++) {
    if (strcmp (Func->AddressTakenVars[i], Name) == 0) {
      return;  // Already added
    }
  }

  //
  // Grow array if needed
  //
  if (Func->AddressTakenCount >= Func->AddressTakenCapacity) {
    Func->AddressTakenCapacity = (Func->AddressTakenCapacity == 0) ? 8 : Func->AddressTakenCapacity * 2;
    NewArray = (CHAR8 **)realloc (Func->AddressTakenVars, Func->AddressTakenCapacity * sizeof (CHAR8 *));
    if (NewArray == NULL) {
      return;  // Out of memory
    }
    Func->AddressTakenVars = NewArray;
  }

  //
  // Add name to list
  //
  Func->AddressTakenVars[Func->AddressTakenCount] = strdup (Name);
  Func->AddressTakenCount++;
}

/**
  Check if variable is address-taken.

  @param[in]      Func          IR function.
  @param[in]      Name          Variable name.

  @return  TRUE if address is taken, FALSE otherwise.

**/
STATIC
BOOLEAN
IrIsAddressTaken (
  IN  IR_FUNCTION  *Func,
  IN  CONST CHAR8  *Name
  )
{
  UINT32  i;

  if (Func == NULL || Name == NULL) {
    return FALSE;
  }

  for (i = 0; i < Func->AddressTakenCount; i++) {
    if (strcmp (Func->AddressTakenVars[i], Name) == 0) {
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Scan expression tree to mark address-taken variables.

  @param[in,out]  Context       IR context.
  @param[in]      Expr          Expression to scan.

**/
STATIC
VOID
IrMarkAddressTaken (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_EXPR    *Expr
  )
{

  if (Expr == NULL || Context->CurrentFunc == NULL) {
    return;
  }

  switch (Expr->Kind) {
    case AST_EXPR_UNARY:
      if (Expr->Unary.Op == UN_OP_ADDRESS_OF &&
          Expr->Unary.Operand->Kind == AST_EXPR_IDENTIFIER) {
        //
        // Add this variable name to address-taken list
        //
        IrAddAddressTakenVar (Context->CurrentFunc, Expr->Unary.Operand->Identifier.Name);
      }
      IrMarkAddressTaken (Context, Expr->Unary.Operand);
      break;

    case AST_EXPR_BINARY:
      IrMarkAddressTaken (Context, Expr->Binary.Left);
      IrMarkAddressTaken (Context, Expr->Binary.Right);
      break;

    case AST_EXPR_CALL:
      for (UINT32 i = 0; i < Expr->Call.ArgumentCount; i++) {
        IrMarkAddressTaken (Context, Expr->Call.Arguments[i]);
      }
      break;

    case AST_EXPR_INDEX:
      IrMarkAddressTaken (Context, Expr->Index.Array);
      IrMarkAddressTaken (Context, Expr->Index.Index);
      break;

    case AST_EXPR_BIT_FIELD:
      IrMarkAddressTaken (Context, Expr->BitField.Object);
      IrMarkAddressTaken (Context, Expr->BitField.BitIndex);
      IrMarkAddressTaken (Context, Expr->BitField.BitCount);
      break;

    case AST_EXPR_CONDITIONAL:
      IrMarkAddressTaken (Context, Expr->Conditional.Condition);
      IrMarkAddressTaken (Context, Expr->Conditional.ThenExpr);
      IrMarkAddressTaken (Context, Expr->Conditional.ElseExpr);
      break;

    default:
      //
      // Leaf nodes or unsupported nodes
      //
      break;
  }
}

/**
  Scan statement tree to mark address-taken variables.

  @param[in,out]  Context       IR context.
  @param[in]      Stmt          Statement to scan.

**/
STATIC
VOID
IrMarkAddressTakenStmt (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_STMT    *Stmt
  )
{
  if (Stmt == NULL) {
    return;
  }

  switch (Stmt->Kind) {
    case AST_STMT_EXPR:
      IrMarkAddressTaken (Context, Stmt->Expr.Expression);
      break;

    case AST_STMT_RETURN:
      IrMarkAddressTaken (Context, Stmt->Return.Value);
      break;

    case AST_STMT_IF:
      IrMarkAddressTaken (Context, Stmt->If.Condition);
      IrMarkAddressTakenStmt (Context, Stmt->If.ThenBranch);
      IrMarkAddressTakenStmt (Context, Stmt->If.ElseBranch);
      break;

    case AST_STMT_WHILE:
    case AST_STMT_DO_WHILE:
      IrMarkAddressTaken (Context, Stmt->While.Condition);
      IrMarkAddressTakenStmt (Context, Stmt->While.Body);
      break;

    case AST_STMT_FOR:
      if (Stmt->For.Initializer != NULL) {
        IrMarkAddressTakenStmt (Context, Stmt->For.Initializer);
      }
      IrMarkAddressTaken (Context, Stmt->For.Condition);
      IrMarkAddressTaken (Context, Stmt->For.Increment);
      IrMarkAddressTakenStmt (Context, Stmt->For.Body);
      break;

    case AST_STMT_COMPOUND:
      for (UINT32 i = 0; i < Stmt->Compound.StatementCount; i++) {
        IrMarkAddressTakenStmt (Context, Stmt->Compound.Statements[i]);
      }
      break;

    case AST_STMT_DECL:
      //
      // Check initializers
      //
      for (UINT32 i = 0; i < Stmt->Decl.DeclarationCount; i++) {
        AST_DECL  *Decl = Stmt->Decl.Declarations[i];
        if (Decl != NULL && Decl->Kind == AST_DECL_VAR && Decl->Var.Initializer != NULL) {
          IrMarkAddressTaken (Context, Decl->Var.Initializer);
        }
      }
      break;

    default:
      break;
  }
}

/**
  Generate IR for unary expression.

  @param[in,out]  Context       IR context.
  @param[in]      Expr          Unary expression.

  @return  Operand containing result.

**/
STATIC
IR_OPERAND
IrGenUnaryExpr (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_EXPR    *Expr
  )
{
  IR_OPERAND      Operand, Result;
  IR_INSTRUCTION  *Instr;
  IR_OPCODE       Opcode;

  //
  // Special case: address-of operator on address-taken variable
  // The operand already contains the address, so just return it
  //
  if (Expr->Unary.Op == UN_OP_ADDRESS_OF &&
      Expr->Unary.Operand->Kind == AST_EXPR_IDENTIFIER &&
      IrIsAddressTaken (Context->CurrentFunc, Expr->Unary.Operand->Identifier.Name)) {
    //
    // Address-taken variable - the symbol's operand is already the address
    //
    IR_SYMBOL  *Sym = IrLookupSymbol (Context->CurrentFunc->Symbols, Expr->Unary.Operand->Identifier.Name);
    if (Sym != NULL) {
      return Sym->Operand;
    }
  }

  //
  // Generate operand
  //
  Operand = IrGenExpression (Context, Expr->Unary.Operand);

  //
  // Determine opcode
  //
  switch (Expr->Unary.Op) {
    case UN_OP_MINUS:       Opcode = IR_NEG; break;
    case UN_OP_BIT_NOT:     Opcode = IR_NOT; break;
    case UN_OP_LOGICAL_NOT: Opcode = IR_LNOT; break;
    case UN_OP_ADDRESS_OF:  Opcode = IR_ADDR; break;
    case UN_OP_DEREF:       Opcode = IR_LOAD; break;
    default:
      //
      // TODO: Handle other operators
      //
      Result.Type = IR_OPERAND_NONE;
      return Result;
  }

  //
  // Allocate result register
  //
  Result = IrAllocReg (Context->CurrentFunc, Expr->Type);

  //
  // Create instruction
  //
  Instr = IrCreateInstruction (Opcode);
  Instr->Dst = Result;
  Instr->Src1 = Operand;

  IrAppendInstruction (Context->CurrentBlock, Instr);

  return Result;
}

/**
  Compute the size in bytes of a type.

  @param[in]      Type          Type to compute size of.
  @param[in]      Mode32Bit     TRUE for 32-bit mode.

  @return  Size in bytes, or 0 if unknown.

**/
STATIC
UINT32
IrGetTypeSize (
  IN  AST_TYPE  *Type,
  IN  BOOLEAN   Mode32Bit
  )
{
  if (Type == NULL) {
    return 0;
  }

  switch (Type->Kind) {
    case AST_TYPE_VOID:
      return 0;

    case AST_TYPE_BOOL:
    case AST_TYPE_CHAR:
    case AST_TYPE_UCHAR:
    case AST_TYPE_INT8:
      return 1;

    case AST_TYPE_SHORT:
    case AST_TYPE_USHORT:
    case AST_TYPE_INT16:
      return 2;

    case AST_TYPE_INT:
    case AST_TYPE_UINT:
    case AST_TYPE_INT32:
    case AST_TYPE_FLOAT:
      return 4;

    case AST_TYPE_LONG:
    case AST_TYPE_ULONG:
    case AST_TYPE_LONG_LONG:
    case AST_TYPE_ULONG_LONG:
    case AST_TYPE_INT64:
    case AST_TYPE_DOUBLE:
      return 8;

    case AST_TYPE_POINTER:
      //
      // Pointer size depends on mode and __ptr64/__ptr32 qualifiers
      // In 32-bit mode: pointers are 4 bytes unless __ptr64 (then 8)
      // In 64-bit mode: pointers are 8 bytes unless __ptr32 (then 4)
      //
      if (Type->Pointer.IsPtr64) {
        return 8;  // __ptr64 forces 64-bit pointers
      } else if (Type->Pointer.IsPtr32) {
        return 4;  // __ptr32 forces 32-bit pointers
      } else if (Mode32Bit) {
        return 4;  // Default in 32-bit mode
      } else {
        return 8;  // Default in 64-bit mode
      }

    case AST_TYPE_LONG_DOUBLE:
    case AST_TYPE_INT128:
      return 16;

    case AST_TYPE_ARRAY:
      {
        UINT32  ElementSize = IrGetTypeSize (Type->Array.ElementType, Mode32Bit);
        if (Type->Array.Size != NULL && Type->Array.Size->Kind == AST_EXPR_INTEGER) {
          return ElementSize * Type->Array.Size->Integer.Value;
        }
        return 0;  // Unknown size array
      }

    case AST_TYPE_STRUCT:
    case AST_TYPE_UNION:
      // TODO: Compute struct/union size from members
      return 8;  // Placeholder

    case AST_TYPE_BITINT:
      // _BitInt(N) - round up to nearest byte
      return (Type->BitInt.Width + 7) / 8;

    default:
      return 0;
  }
}

/**
  Generate IR for expression.

  @param[in,out]  Context       IR context.
  @param[in]      Expr          Expression AST.

  @return  Operand containing result.

**/
IR_OPERAND
IrGenExpression (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_EXPR    *Expr
  )
{
  IR_OPERAND  Result;

  Result.Type = IR_OPERAND_NONE;
  Result.BitWidth = 0;
  Result.DataType = NULL;

  if (Expr == NULL) {
    return Result;
  }

  switch (Expr->Kind) {
    case AST_EXPR_INTEGER:
      return IrConstant (Expr->Integer.Value, Expr->Type, Expr->Integer.BitWidth);

    case AST_EXPR_IDENTIFIER:
      {
        IR_SYMBOL  *Sym;

        //
        // Look up identifier in symbol table
        //
        if (Context->CurrentFunc != NULL && Context->CurrentFunc->Symbols != NULL) {
          Sym = IrLookupSymbol (Context->CurrentFunc->Symbols, Expr->Identifier.Name);
          if (Sym != NULL) {
            //
            // Check if this is an address-taken variable
            //
            if (IrIsAddressTaken (Context->CurrentFunc, Expr->Identifier.Name)) {
              //
              // Address-taken variable - generate LOAD to read value from memory
              //
              IR_OPERAND      LoadResult;
              IR_INSTRUCTION  *LoadInstr;

              LoadResult = IrAllocReg (Context->CurrentFunc, Expr->Type);
              LoadInstr = IrCreateInstruction (IR_LOAD);
              LoadInstr->Dst = LoadResult;
              LoadInstr->Src1 = Sym->Operand;  // Address
              IrAppendInstruction (Context->CurrentBlock, LoadInstr);

              return LoadResult;
            }

            //
            // Regular variable - return the associated operand
            //
            return Sym->Operand;
          }
        }

        //
        // Not found in symbol table - treat as global symbol
        //
        return IrSymbol (Expr->Identifier.Name, Expr->Type);
      }

    case AST_EXPR_BINARY:
      return IrGenBinaryExpr (Context, Expr);

    case AST_EXPR_UNARY:
      return IrGenUnaryExpr (Context, Expr);

    case AST_EXPR_CALL:
      {
        IR_INSTRUCTION  *Instr;
        IR_OPERAND      Callee;

        //
        // Generate callee
        //
        Callee = IrGenExpression (Context, Expr->Call.Callee);

        //
        // Allocate result register
        //
        Result = IrAllocReg (Context->CurrentFunc, Expr->Type);

        //
        // Create call instruction
        //
        Instr = IrCreateInstruction (IR_CALL);
        Instr->Dst = Result;
        Instr->Src1 = Callee;

        //
        // Generate arguments
        //
        if (Expr->Call.ArgumentCount > 0) {
          Instr->Args = (IR_OPERAND *)malloc (Expr->Call.ArgumentCount * sizeof (IR_OPERAND));
          Instr->ArgCount = Expr->Call.ArgumentCount;

          for (UINT32 i = 0; i < Expr->Call.ArgumentCount; i++) {
            Instr->Args[i] = IrGenExpression (Context, Expr->Call.Arguments[i]);
          }
        }

        IrAppendInstruction (Context->CurrentBlock, Instr);
      }
      break;

    case AST_EXPR_INDEX:
      {
        //
        // Array indexing: array[index]
        // Generate: addr = array + index * element_size
        //           result = load addr
        //
        IR_OPERAND      Array, Index, Addr;
        IR_INSTRUCTION  *Instr;

        Array = IrGenExpression (Context, Expr->Index.Array);
        Index = IrGenExpression (Context, Expr->Index.Index);

        //
        // TODO: Multiply index by element size
        // For now, just add
        //
        Addr = IrAllocReg (Context->CurrentFunc, Expr->Type);
        Instr = IrCreateInstruction (IR_ADD);
        Instr->Dst = Addr;
        Instr->Src1 = Array;
        Instr->Src2 = Index;
        IrAppendInstruction (Context->CurrentBlock, Instr);

        //
        // Load from address
        //
        Result = IrAllocReg (Context->CurrentFunc, Expr->Type);
        Instr = IrCreateInstruction (IR_LOAD);
        Instr->Dst = Result;
        Instr->Src1 = Addr;
        IrAppendInstruction (Context->CurrentBlock, Instr);
      }
      break;

    case AST_EXPR_BIT_FIELD:
      {
        //
        // Bit field extraction: object[index:count]
        // Generate: result = bitfield_extract(object, index, count)
        //
        IR_OPERAND      Object, BitIndex, BitCount;
        IR_INSTRUCTION  *Instr;

        Object = IrGenExpression (Context, Expr->BitField.Object);
        BitIndex = IrGenExpression (Context, Expr->BitField.BitIndex);
        BitCount = IrGenExpression (Context, Expr->BitField.BitCount);

        //
        // Allocate result register
        //
        Result = IrAllocReg (Context->CurrentFunc, Expr->Type);

        //
        // Create bit field extract instruction
        // IR_BFEXT: dst = bitfield_extract(src, index, count)
        //
        Instr = IrCreateInstruction (IR_BFEXT);
        Instr->Dst = Result;
        Instr->Src1 = Object;
        Instr->Src2 = BitIndex;
        Instr->Args = (IR_OPERAND *)malloc (sizeof (IR_OPERAND));
        Instr->Args[0] = BitCount;
        Instr->ArgCount = 1;
        IrAppendInstruction (Context->CurrentBlock, Instr);
      }
      break;

    case AST_EXPR_SIZEOF:
      {
        //
        // sizeof operator - compute size at compile time
        //
        UINT32     Size;
        AST_TYPE   *TargetType = NULL;
        IR_SYMBOL  *Sym;

        if (Expr->Sizeof.TargetType != NULL) {
          //
          // sizeof(type)
          //
          TargetType = Expr->Sizeof.TargetType;
        } else if (Expr->Sizeof.Operand != NULL) {
          //
          // sizeof expr - need to determine the expression's type
          //
          AST_EXPR  *Operand = Expr->Sizeof.Operand;

          if (Operand->Type != NULL) {
            //
            // Type already set (from semantic analysis)
            //
            TargetType = Operand->Type;
          } else if (Operand->Kind == AST_EXPR_IDENTIFIER) {
            //
            // Identifier - look up its type in symbol table
            //
            if (Context->CurrentFunc != NULL && Context->CurrentFunc->Symbols != NULL) {
              Sym = IrLookupSymbol (Context->CurrentFunc->Symbols, Operand->Identifier.Name);
              if (Sym != NULL && Sym->Type != NULL) {
                TargetType = Sym->Type;
              }
            }
          } else if (Operand->Kind == AST_EXPR_INTEGER) {
            //
            // Integer literal - default to int type (4 bytes)
            //
            AST_TYPE  *IntType = AstTypeCreate (AST_TYPE_INT);
            TargetType = IntType;
          } else if (Operand->Kind == AST_EXPR_BINARY || Operand->Kind == AST_EXPR_UNARY) {
            //
            // Binary/unary expression - infer type from the expression
            // For now, assume int type (most arithmetic operations result in int)
            // TODO: Proper type inference
            //
            AST_TYPE  *IntType = AstTypeCreate (AST_TYPE_INT);
            TargetType = IntType;
          } else {
            //
            // For other expressions, try to use existing type
            //
            TargetType = Operand->Type;
          }
        }

        if (TargetType == NULL) {
          //
          // Could not determine type - return 0
          //
          return IrConstant (0, Expr->Type, 0);
        }

        Size = IrGetTypeSize (TargetType, Context->Module->Mode32Bit);

        //
        // Return size as a constant
        //
        return IrConstant (Size, Expr->Type, 0);
      }
      break;

    default:
      //
      // TODO: Handle other expression types
      //
      break;
  }

  return Result;
}

/**
  Generate IR for statement.

  @param[in,out]  Context       IR context.
  @param[in]      Stmt          Statement AST.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
IrGenStatement (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_STMT    *Stmt
  )
{
  if (Stmt == NULL) {
    return MMIX_SUCCESS;
  }

  switch (Stmt->Kind) {
    case AST_STMT_EXPR:
      if (Stmt->Expr.Expression != NULL) {
        IrGenExpression (Context, Stmt->Expr.Expression);
      }
      break;

    case AST_STMT_COMPOUND:
      for (UINT32 i = 0; i < Stmt->Compound.StatementCount; i++) {
        IrGenStatement (Context, Stmt->Compound.Statements[i]);
      }
      break;

    case AST_STMT_IF:
      {
        IR_OPERAND      Cond;
        IR_BASIC_BLOCK  *ThenBlock, *ElseBlock, *MergeBlock;
        IR_INSTRUCTION  *BrInstr;

        //
        // Generate condition
        //
        Cond = IrGenExpression (Context, Stmt->If.Condition);

        //
        // Create blocks
        //
        ThenBlock = IrCreateBasicBlock (Context->CurrentFunc, "if.then");
        MergeBlock = IrCreateBasicBlock (Context->CurrentFunc, "if.end");

        if (Stmt->If.ElseBranch != NULL) {
          ElseBlock = IrCreateBasicBlock (Context->CurrentFunc, "if.else");
        } else {
          ElseBlock = MergeBlock;
        }

        //
        // Branch instruction
        //
        BrInstr = IrCreateInstruction (IR_BR_COND);
        BrInstr->Src1 = Cond;
        BrInstr->LabelId = ThenBlock->Id;
        BrInstr->LabelId2 = ElseBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, ThenBlock);
        IrAddEdge (Context->CurrentBlock, ElseBlock);

        //
        // Generate then branch
        //
        Context->CurrentBlock = ThenBlock;
        IrGenStatement (Context, Stmt->If.ThenBranch);
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = MergeBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, MergeBlock);

        //
        // Generate else branch if present
        //
        if (Stmt->If.ElseBranch != NULL) {
          Context->CurrentBlock = ElseBlock;
          IrGenStatement (Context, Stmt->If.ElseBranch);
          BrInstr = IrCreateInstruction (IR_BR);
          BrInstr->LabelId = MergeBlock->Id;
          IrAppendInstruction (Context->CurrentBlock, BrInstr);
          IrAddEdge (Context->CurrentBlock, MergeBlock);
        }

        Context->CurrentBlock = MergeBlock;
      }
      break;

    case AST_STMT_WHILE:
      {
        IR_OPERAND      Cond;
        IR_BASIC_BLOCK  *CondBlock, *BodyBlock, *ExitBlock;
        IR_BASIC_BLOCK  *SaveBreak, *SaveContinue;
        IR_INSTRUCTION  *BrInstr;

        //
        // Create blocks
        //
        CondBlock = IrCreateBasicBlock (Context->CurrentFunc, "while.cond");
        BodyBlock = IrCreateBasicBlock (Context->CurrentFunc, "while.body");
        ExitBlock = IrCreateBasicBlock (Context->CurrentFunc, "while.end");

        //
        // Save break/continue targets
        //
        SaveBreak = Context->BreakTarget;
        SaveContinue = Context->ContinueTarget;
        Context->BreakTarget = ExitBlock;
        Context->ContinueTarget = CondBlock;

        //
        // Branch to condition
        //
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = CondBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, CondBlock);

        //
        // Generate condition
        //
        Context->CurrentBlock = CondBlock;
        Cond = IrGenExpression (Context, Stmt->While.Condition);
        BrInstr = IrCreateInstruction (IR_BR_COND);
        BrInstr->Src1 = Cond;
        BrInstr->LabelId = BodyBlock->Id;
        BrInstr->LabelId2 = ExitBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, BodyBlock);
        IrAddEdge (Context->CurrentBlock, ExitBlock);

        //
        // Generate body
        //
        Context->CurrentBlock = BodyBlock;
        IrGenStatement (Context, Stmt->While.Body);
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = CondBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, CondBlock);

        //
        // Restore break/continue targets
        //
        Context->BreakTarget = SaveBreak;
        Context->ContinueTarget = SaveContinue;

        Context->CurrentBlock = ExitBlock;
      }
      break;

    case AST_STMT_FOR:
      {
        IR_OPERAND      Cond;
        IR_BASIC_BLOCK  *CondBlock, *BodyBlock, *IncrBlock, *ExitBlock;
        IR_BASIC_BLOCK  *SaveBreak, *SaveContinue;
        IR_INSTRUCTION  *BrInstr;

        //
        // Generate initializer
        //
        if (Stmt->For.Initializer != NULL) {
          IrGenStatement (Context, Stmt->For.Initializer);
        }

        //
        // Create blocks
        //
        CondBlock = IrCreateBasicBlock (Context->CurrentFunc, "for.cond");
        BodyBlock = IrCreateBasicBlock (Context->CurrentFunc, "for.body");
        IncrBlock = IrCreateBasicBlock (Context->CurrentFunc, "for.incr");
        ExitBlock = IrCreateBasicBlock (Context->CurrentFunc, "for.end");

        //
        // Save break/continue targets
        // Continue goes to increment block
        //
        SaveBreak = Context->BreakTarget;
        SaveContinue = Context->ContinueTarget;
        Context->BreakTarget = ExitBlock;
        Context->ContinueTarget = IncrBlock;

        //
        // Branch to condition
        //
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = CondBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, CondBlock);

        //
        // Generate condition
        //
        Context->CurrentBlock = CondBlock;
        if (Stmt->For.Condition != NULL) {
          Cond = IrGenExpression (Context, Stmt->For.Condition);
          BrInstr = IrCreateInstruction (IR_BR_COND);
          BrInstr->Src1 = Cond;
          BrInstr->LabelId = BodyBlock->Id;
          BrInstr->LabelId2 = ExitBlock->Id;
          IrAppendInstruction (Context->CurrentBlock, BrInstr);
          IrAddEdge (Context->CurrentBlock, BodyBlock);
          IrAddEdge (Context->CurrentBlock, ExitBlock);
        } else {
          //
          // No condition means infinite loop (like for(;;))
          //
          BrInstr = IrCreateInstruction (IR_BR);
          BrInstr->LabelId = BodyBlock->Id;
          IrAppendInstruction (Context->CurrentBlock, BrInstr);
          IrAddEdge (Context->CurrentBlock, BodyBlock);
        }

        //
        // Generate body
        //
        Context->CurrentBlock = BodyBlock;
        IrGenStatement (Context, Stmt->For.Body);
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = IncrBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, IncrBlock);

        //
        // Generate increment
        //
        Context->CurrentBlock = IncrBlock;
        if (Stmt->For.Increment != NULL) {
          IrGenExpression (Context, Stmt->For.Increment);
        }
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = CondBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, CondBlock);

        //
        // Restore break/continue targets
        //
        Context->BreakTarget = SaveBreak;
        Context->ContinueTarget = SaveContinue;

        Context->CurrentBlock = ExitBlock;
      }
      break;

    case AST_STMT_DO_WHILE:
      {
        IR_OPERAND      Cond;
        IR_BASIC_BLOCK  *BodyBlock, *CondBlock, *ExitBlock;
        IR_BASIC_BLOCK  *SaveBreak, *SaveContinue;
        IR_INSTRUCTION  *BrInstr;

        //
        // Create blocks
        //
        BodyBlock = IrCreateBasicBlock (Context->CurrentFunc, "do.body");
        CondBlock = IrCreateBasicBlock (Context->CurrentFunc, "do.cond");
        ExitBlock = IrCreateBasicBlock (Context->CurrentFunc, "do.end");

        //
        // Save break/continue targets
        // Continue goes to condition block (check before next iteration)
        //
        SaveBreak = Context->BreakTarget;
        SaveContinue = Context->ContinueTarget;
        Context->BreakTarget = ExitBlock;
        Context->ContinueTarget = CondBlock;

        //
        // Branch to body (do-while executes body at least once)
        //
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = BodyBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, BodyBlock);

        //
        // Generate body
        //
        Context->CurrentBlock = BodyBlock;
        IrGenStatement (Context, Stmt->While.Body);
        BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = CondBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, CondBlock);

        //
        // Generate condition
        //
        Context->CurrentBlock = CondBlock;
        Cond = IrGenExpression (Context, Stmt->While.Condition);
        BrInstr = IrCreateInstruction (IR_BR_COND);
        BrInstr->Src1 = Cond;
        BrInstr->LabelId = BodyBlock->Id;
        BrInstr->LabelId2 = ExitBlock->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, BodyBlock);
        IrAddEdge (Context->CurrentBlock, ExitBlock);

        //
        // Restore break/continue targets
        //
        Context->BreakTarget = SaveBreak;
        Context->ContinueTarget = SaveContinue;

        Context->CurrentBlock = ExitBlock;
      }
      break;

    case AST_STMT_RETURN:
      {
        IR_INSTRUCTION  *RetInstr;

        RetInstr = IrCreateInstruction (IR_RET);
        if (Stmt->Return.Value != NULL) {
          RetInstr->Src1 = IrGenExpression (Context, Stmt->Return.Value);
        }
        IrAppendInstruction (Context->CurrentBlock, RetInstr);
      }
      break;

    case AST_STMT_BREAK:
      if (Context->BreakTarget != NULL) {
        IR_INSTRUCTION  *BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = Context->BreakTarget->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, Context->BreakTarget);
      }
      break;

    case AST_STMT_CONTINUE:
      if (Context->ContinueTarget != NULL) {
        IR_INSTRUCTION  *BrInstr = IrCreateInstruction (IR_BR);
        BrInstr->LabelId = Context->ContinueTarget->Id;
        IrAppendInstruction (Context->CurrentBlock, BrInstr);
        IrAddEdge (Context->CurrentBlock, Context->ContinueTarget);
      }
      break;

    case AST_STMT_DECL:
      //
      // Process local variable declarations
      //
      for (UINT32 i = 0; i < Stmt->Decl.DeclarationCount; i++) {
        AST_DECL  *Decl = Stmt->Decl.Declarations[i];
        if (Decl != NULL && Decl->Kind == AST_DECL_VAR) {
          IR_OPERAND  VarOp;

          //
          // Check if this is an array declaration
          //
          if (Decl->Type != NULL && Decl->Type->Kind == AST_TYPE_ARRAY) {
            //
            // Array declaration - allocate space on stack
            // Use ALLOCA to allocate array_size * element_size bytes
            //
            IR_INSTRUCTION  *AllocaInstr;
            IR_OPERAND      Size;
            UINT64          ArraySize = 0;

            // Get array size (TODO: handle dynamic arrays)
            if (Decl->Type->Array.Size != NULL &&
                Decl->Type->Array.Size->Kind == AST_EXPR_INTEGER) {
              ArraySize = Decl->Type->Array.Size->Integer.Value;
            }

            // For now, assume element size is 8 bytes (octa)
            Size = IrConstant (ArraySize * 8, Decl->Type, 0);

            // Allocate result register to hold array address
            VarOp = IrAllocReg (Context->CurrentFunc, Decl->Type);

            // Generate ALLOCA instruction
            AllocaInstr = IrCreateInstruction (IR_ALLOCA);
            AllocaInstr->Dst = VarOp;
            AllocaInstr->Src1 = Size;
            IrAppendInstruction (Context->CurrentBlock, AllocaInstr);
          } else if (IrIsAddressTaken (Context->CurrentFunc, Decl->Name)) {
            //
            // Address-taken scalar variable - allocate on stack
            //
            IR_INSTRUCTION  *AllocaInstr;
            IR_OPERAND      Size;

            // Allocate 8 bytes (one octa) for scalar
            Size = IrConstant (8, Decl->Type, 0);

            // Allocate result register to hold variable address
            VarOp = IrAllocReg (Context->CurrentFunc, Decl->Type);

            // Generate ALLOCA instruction
            AllocaInstr = IrCreateInstruction (IR_ALLOCA);
            AllocaInstr->Dst = VarOp;
            AllocaInstr->Src1 = Size;
            IrAppendInstruction (Context->CurrentBlock, AllocaInstr);
          } else {
            //
            // Regular scalar variable - allocate register
            //
            VarOp = IrAllocReg (Context->CurrentFunc, Decl->Type);
          }

          //
          // Add to function's symbol table
          //
          if (Decl->Name != NULL && Context->CurrentFunc->Symbols != NULL) {
            IrAddSymbol (
              Context->CurrentFunc->Symbols,
              Decl->Name,
              VarOp,
              Decl->Type,
              Decl,    // Store declaration for address-taken analysis
              FALSE,   // IsParameter = FALSE for local variables
              0        // ParamIndex (unused for locals)
            );
          }

          //
          // Generate initializer if present (only for scalars)
          //
          if (Decl->Var.Initializer != NULL &&
              (Decl->Type == NULL || Decl->Type->Kind != AST_TYPE_ARRAY)) {
            IR_OPERAND       InitValue;
            IR_INSTRUCTION  *StoreInstr;

            InitValue = IrGenExpression (Context, Decl->Var.Initializer);

            if (IrIsAddressTaken (Context->CurrentFunc, Decl->Name)) {
              //
              // Address-taken variable - use STORE to initialize
              //
              StoreInstr = IrCreateInstruction (IR_STORE);
              StoreInstr->Src1 = VarOp;      // Address
              StoreInstr->Src2 = InitValue;  // Value
              IrAppendInstruction (Context->CurrentBlock, StoreInstr);
            } else {
              //
              // Register variable - use MOVE to initialize
              //
              StoreInstr = IrCreateInstruction (IR_MOVE);
              StoreInstr->Dst = VarOp;
              StoreInstr->Src1 = InitValue;
              IrAppendInstruction (Context->CurrentBlock, StoreInstr);
            }
          }
        }
      }
      break;

    default:
      //
      // TODO: Handle other statement types
      //
      break;
  }

  return MMIX_SUCCESS;
}

/**
  Generate IR for declaration.

  @param[in,out]  Context       IR context.
  @param[in]      Decl          Declaration AST.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
IrGenDeclaration (
  IN OUT IR_CONTEXT  *Context,
  IN     AST_DECL    *Decl
  )
{
  if (Decl == NULL) {
    return MMIX_SUCCESS;
  }

  if (Decl->Kind == AST_DECL_FUNCTION && Decl->Function.IsDefinition) {
    //
    // Create function
    //
    IR_FUNCTION  *Func = IrCreateFunction (Decl);
    if (Func == NULL) {
      return MMIX_ERROR_OUT_OF_MEMORY;
    }

    //
    // Add to module
    //
    Context->Module->FunctionCount++;
    Context->Module->Functions = (IR_FUNCTION **)realloc (
      Context->Module->Functions,
      Context->Module->FunctionCount * sizeof (IR_FUNCTION *)
    );
    Context->Module->Functions[Context->Module->FunctionCount - 1] = Func;

    //
    // Create entry block
    //
    Func->EntryBlock = IrCreateBasicBlock (Func, "entry");
    Func->EntryBlock->IsEntry = TRUE;

    //
    // Set current function and block
    //
    Context->CurrentFunc = Func;
    Context->CurrentBlock = Func->EntryBlock;

    //
    // Add parameters to symbol table
    // GCC MMIX ABI: parameters are in registers $231-$246 (virtual regs mapped by code gen)
    //
    if (Decl->Function.Parameters != NULL) {
      for (UINT32 i = 0; i < Decl->Function.ParameterCount; i++) {
        AST_DECL    *Param = Decl->Function.Parameters[i];
        IR_OPERAND  ParamOp;

        //
        // Allocate virtual register for this parameter
        //
        ParamOp.Type = IR_OPERAND_REG;
        ParamOp.RegNum = Func->NextRegNum++;
        ParamOp.DataType = Param->Type;
        ParamOp.BitWidth = 0;  // Not applicable for parameters

        //
        // Add to symbol table
        //
        IrAddSymbol (Func->Symbols, Param->Name, ParamOp, Param->Type, Param, TRUE, i);
      }
    }

    //
    // Scan function body to mark address-taken variables
    //
    if (Decl->Function.Body != NULL) {
      IrMarkAddressTakenStmt (Context, Decl->Function.Body);
    }

    //
    // Generate function body
    //
    if (Decl->Function.Body != NULL) {
      IrGenStatement (Context, Decl->Function.Body);
    }

    Context->CurrentFunc = NULL;
    Context->CurrentBlock = NULL;
  }

  return MMIX_SUCCESS;
}

/**
  Generate IR for entire module.

  @param[in,out]  Module        IR module.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
IrGenerateModule (
  IN OUT IR_MODULE  *Module
  )
{
  IR_CONTEXT  Context;

  if (Module == NULL || Module->Ast == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Context.Module = Module;
  Context.CurrentFunc = NULL;
  Context.CurrentBlock = NULL;
  Context.BreakTarget = NULL;
  Context.ContinueTarget = NULL;
  Context.ErrorCount = 0;

  //
  // Generate IR for each top-level declaration
  //
  for (UINT32 i = 0; i < Module->Ast->DeclarationCount; i++) {
    IrGenDeclaration (&Context, Module->Ast->Declarations[i]);
  }

  if (Context.ErrorCount > 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  return MMIX_SUCCESS;
}

/**
  Get opcode name for printing.

  @param[in]      Opcode        Opcode.

  @return  Opcode name string.

**/
STATIC
CONST CHAR8 *
IrGetOpcodeName (
  IN  IR_OPCODE  Opcode
  )
{
  switch (Opcode) {
    case IR_ADD:      return "add";
    case IR_SUB:      return "sub";
    case IR_MUL:      return "mul";
    case IR_DIV:      return "div";
    case IR_MOD:      return "mod";
    case IR_NEG:      return "neg";
    case IR_AND:      return "and";
    case IR_OR:       return "or";
    case IR_XOR:      return "xor";
    case IR_NOT:      return "not";
    case IR_SHL:      return "shl";
    case IR_SHR:      return "shr";
    case IR_EQ:       return "eq";
    case IR_NE:       return "ne";
    case IR_LT:       return "lt";
    case IR_LE:       return "le";
    case IR_GT:       return "gt";
    case IR_GE:       return "ge";
    case IR_LAND:     return "land";
    case IR_LOR:      return "lor";
    case IR_LNOT:     return "lnot";
    case IR_LOAD:     return "load";
    case IR_STORE:    return "store";
    case IR_ALLOCA:   return "alloca";
    case IR_ADDR:     return "addr";
    case IR_BR:       return "br";
    case IR_BR_COND:  return "br_cond";
    case IR_CALL:     return "call";
    case IR_RET:      return "ret";
    case IR_PHI:      return "phi";
    case IR_CAST:     return "cast";
    case IR_SEXT:     return "sext";
    case IR_ZEXT:     return "zext";
    case IR_TRUNC:    return "trunc";
    case IR_MOVE:     return "move";
    case IR_NOP:      return "nop";
    case IR_LABEL:    return "label";
    default:          return "unknown";
  }
}

/**
  Print IR operand.

  @param[in]      Operand       Operand to print.
  @param[in]      Output        Output file.

**/
STATIC
VOID
IrPrintOperand (
  IN  IR_OPERAND  *Operand,
  IN  FILE        *Output
  )
{
  if (Operand == NULL) {
    return;
  }

  switch (Operand->Type) {
    case IR_OPERAND_REG:
      fprintf (Output, "%%r%u", Operand->RegNum);
      break;
    case IR_OPERAND_CONST:
      fprintf (Output, "%lld", (long long)Operand->ConstValue);
      break;
    case IR_OPERAND_SYMBOL:
      fprintf (Output, "@%s", Operand->SymbolName);
      break;
    case IR_OPERAND_LABEL:
      fprintf (Output, "L%u", Operand->LabelId);
      break;
    default:
      fprintf (Output, "<none>");
      break;
  }
}

/**
  Print IR function in human-readable format.

  @param[in]      Func          IR function.
  @param[in]      Output        Output file.

**/
VOID
IrPrintFunction (
  IN  IR_FUNCTION  *Func,
  IN  FILE         *Output
  )
{
  if (Func == NULL || Output == NULL) {
    return;
  }

  fprintf (Output, "\nfunction @%s:\n", Func->Name);

  //
  // Print each basic block
  //
  for (UINT32 i = 0; i < Func->BlockCount; i++) {
    IR_BASIC_BLOCK  *Block = Func->Blocks[i];

    fprintf (Output, "%s:\n", Block->Label);

    //
    // Print instructions
    //
    for (IR_INSTRUCTION *Instr = Block->First; Instr != NULL; Instr = Instr->Next) {
      fprintf (Output, "  ");

      if (Instr->Dst.Type != IR_OPERAND_NONE) {
        IrPrintOperand (&Instr->Dst, Output);
        fprintf (Output, " = ");
      }

      fprintf (Output, "%s ", IrGetOpcodeName (Instr->Opcode));

      if (Instr->Src1.Type != IR_OPERAND_NONE) {
        IrPrintOperand (&Instr->Src1, Output);
        if (Instr->Src2.Type != IR_OPERAND_NONE) {
          fprintf (Output, ", ");
          IrPrintOperand (&Instr->Src2, Output);
        }
      }

      if (Instr->Opcode == IR_BR) {
        fprintf (Output, "L%u", Instr->LabelId);
      } else if (Instr->Opcode == IR_BR_COND) {
        fprintf (Output, ", L%u, L%u", Instr->LabelId, Instr->LabelId2);
      }

      fprintf (Output, "\n");
    }
  }
}

/**
  Print IR module in human-readable format.

  @param[in]      Module        IR module.
  @param[in]      Output        Output file.

**/
VOID
IrPrintModule (
  IN  IR_MODULE  *Module,
  IN  FILE       *Output
  )
{
  if (Module == NULL || Output == NULL) {
    return;
  }

  fprintf (Output, "; IR Module: %s\n", Module->SourceFile ? Module->SourceFile : "<unknown>");

  for (UINT32 i = 0; i < Module->FunctionCount; i++) {
    IrPrintFunction (Module->Functions[i], Output);
  }
}
