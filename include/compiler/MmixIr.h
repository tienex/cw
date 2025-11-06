/**
  @file MmixIr.h

  Intermediate representation for MMIX C23 compiler.
  Three-address code with SSA support.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#ifndef MMIX_IR_H_
#define MMIX_IR_H_

#include "MmixTypes.h"
#include "MmixAst.h"

/**
  IR opcode types
**/
typedef enum {
  //
  // Arithmetic operations
  //
  IR_ADD,               // dst = a + b
  IR_SUB,               // dst = a - b
  IR_MUL,               // dst = a * b
  IR_DIV,               // dst = a / b
  IR_MOD,               // dst = a % b
  IR_NEG,               // dst = -a

  //
  // Bitwise operations
  //
  IR_AND,               // dst = a & b
  IR_OR,                // dst = a | b
  IR_XOR,               // dst = a ^ b
  IR_NOT,               // dst = ~a
  IR_SHL,               // dst = a << b
  IR_SHR,               // dst = a >> b
  IR_BFEXT,             // dst = bitfield_extract(src, index, count)
  IR_BFINS,             // dst = bitfield_insert(dst, src, index, count)
  IR_BFCONCAT,          // dst = a .. b (bit concatenation)

  //
  // Comparison operations
  //
  IR_EQ,                // dst = a == b
  IR_NE,                // dst = a != b
  IR_LT,                // dst = a < b
  IR_LE,                // dst = a <= b
  IR_GT,                // dst = a > b
  IR_GE,                // dst = a >= b

  //
  // Logical operations
  //
  IR_LAND,              // dst = a && b
  IR_LOR,               // dst = a || b
  IR_LNOT,              // dst = !a

  //
  // Memory operations
  //
  IR_LOAD,              // dst = *addr
  IR_STORE,             // *addr = src
  IR_ALLOCA,            // dst = alloca(size)
  IR_ADDR,              // dst = &var

  //
  // Control flow
  //
  IR_BR,                // goto label
  IR_BR_COND,           // if cond goto label1 else goto label2
  IR_CALL,              // dst = call func(args...)
  IR_RET,               // return value
  IR_PHI,               // dst = phi(a, b, c, ...) for SSA

  //
  // Type operations
  //
  IR_CAST,              // dst = (type)src
  IR_SEXT,              // dst = sign_extend(src)
  IR_ZEXT,              // dst = zero_extend(src)
  IR_TRUNC,             // dst = truncate(src)

  //
  // Other
  //
  IR_MOVE,              // dst = src
  IR_NOP,               // no operation
  IR_LABEL,             // label:
} IR_OPCODE;

/**
  IR operand types
**/
typedef enum {
  IR_OPERAND_NONE,      // No operand
  IR_OPERAND_REG,       // Virtual register
  IR_OPERAND_CONST,     // Constant value
  IR_OPERAND_SYMBOL,    // Symbol reference
  IR_OPERAND_LABEL,     // Label reference
  IR_OPERAND_TEMP,      // Temporary value
} IR_OPERAND_TYPE;

/**
  IR operand
**/
typedef struct {
  IR_OPERAND_TYPE    Type;
  union {
    UINT32           RegNum;       // Virtual register number
    INT64            ConstValue;   // Constant integer value
    double           ConstFloat;   // Constant float value
    CHAR8            *SymbolName;  // Symbol name
    UINT32           LabelId;      // Label ID
    UINT32           TempId;       // Temporary ID
  };
  AST_TYPE           *DataType;    // Type of this operand
} IR_OPERAND;

/**
  IR instruction
**/
typedef struct _IR_INSTRUCTION {
  IR_OPCODE              Opcode;
  IR_OPERAND             Dst;          // Destination operand
  IR_OPERAND             Src1;         // First source operand
  IR_OPERAND             Src2;         // Second source operand
  IR_OPERAND             *Args;        // Additional arguments (for calls, phi)
  UINT32                 ArgCount;     // Number of additional arguments
  UINT32                 LabelId;      // Label ID (for labels, branches)
  UINT32                 LabelId2;     // Second label (for conditional branches)
  TOKEN_LOCATION         Location;     // Source location
  struct _IR_INSTRUCTION *Next;        // Next instruction
  struct _IR_INSTRUCTION *Prev;        // Previous instruction
  UINT32                 Id;           // Unique instruction ID
} IR_INSTRUCTION;

/**
  Basic block
**/
typedef struct _IR_BASIC_BLOCK {
  UINT32                  Id;          // Block ID
  CHAR8                   *Label;      // Block label
  IR_INSTRUCTION          *First;      // First instruction
  IR_INSTRUCTION          *Last;       // Last instruction
  UINT32                  InstrCount;  // Number of instructions
  struct _IR_BASIC_BLOCK  **Preds;     // Predecessor blocks
  UINT32                  PredCount;   // Number of predecessors
  struct _IR_BASIC_BLOCK  **Succs;     // Successor blocks
  UINT32                  SuccCount;   // Number of successors
  BOOLEAN                 IsEntry;     // Is entry block
  BOOLEAN                 IsExit;      // Is exit block
  struct _IR_BASIC_BLOCK  *Next;       // Next block in list
} IR_BASIC_BLOCK;

/**
  Symbol table entry for variables
**/
typedef struct _IR_SYMBOL {
  CHAR8                 *Name;         // Variable name
  IR_OPERAND            Operand;       // Operand (register/const/etc)
  AST_TYPE              *Type;         // Variable type
  BOOLEAN               IsParameter;   // Is function parameter
  UINT32                ParamIndex;    // Parameter index if parameter
  struct _IR_SYMBOL     *Next;         // Next in hash chain
} IR_SYMBOL;

/**
  Symbol table for tracking variables
**/
typedef struct {
  IR_SYMBOL             **Entries;     // Hash table entries
  UINT32                Size;          // Hash table size
  UINT32                Count;         // Number of symbols
} IR_SYMBOL_TABLE;

/**
  IR function
**/
typedef struct {
  CHAR8              *Name;            // Function name
  AST_DECL           *Decl;            // Function declaration
  IR_BASIC_BLOCK     *EntryBlock;      // Entry basic block
  IR_BASIC_BLOCK     **Blocks;         // All basic blocks
  UINT32             BlockCount;       // Number of blocks
  UINT32             NextRegNum;       // Next virtual register number
  UINT32             NextLabelId;      // Next label ID
  UINT32             NextInstrId;      // Next instruction ID
  UINT32             ParamCount;       // Number of parameters
  AST_TYPE           *ReturnType;      // Return type
  IR_SYMBOL_TABLE    *Symbols;         // Symbol table for this function
} IR_FUNCTION;

/**
  IR module (translation unit)
**/
typedef struct {
  IR_FUNCTION        **Functions;      // All functions
  UINT32             FunctionCount;    // Number of functions
  AST_TRANSLATION_UNIT *Ast;           // Source AST
  CHAR8              *SourceFile;      // Source file name
} IR_MODULE;

/**
  IR generator context
**/
typedef struct {
  IR_MODULE          *Module;          // Current module
  IR_FUNCTION        *CurrentFunc;     // Current function
  IR_BASIC_BLOCK     *CurrentBlock;    // Current basic block
  IR_BASIC_BLOCK     *BreakTarget;     // Target for break statements
  IR_BASIC_BLOCK     *ContinueTarget;  // Target for continue statements
  UINT32             ErrorCount;       // Number of errors
} IR_CONTEXT;

/**
  Create IR module from AST.

  @param[in]      Ast           Translation unit AST.

  @return  Pointer to IR module, or NULL on error.

**/
IR_MODULE *
IrCreateModule (
  IN  AST_TRANSLATION_UNIT  *Ast
  );

/**
  Destroy IR module.

  @param[in]      Module        IR module.

**/
VOID
IrDestroyModule (
  IN  IR_MODULE  *Module
  );

/**
  Create IR function.

  @param[in]      Decl          Function declaration.

  @return  Pointer to IR function, or NULL on error.

**/
IR_FUNCTION *
IrCreateFunction (
  IN  AST_DECL  *Decl
  );

/**
  Destroy IR function.

  @param[in]      Func          IR function.

**/
VOID
IrDestroyFunction (
  IN  IR_FUNCTION  *Func
  );

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
  );

/**
  Create IR instruction.

  @param[in]      Opcode        Instruction opcode.

  @return  Pointer to instruction, or NULL on error.

**/
IR_INSTRUCTION *
IrCreateInstruction (
  IN  IR_OPCODE  Opcode
  );

/**
  Append instruction to basic block.

  @param[in,out]  Block         Basic block.
  @param[in]      Instr         Instruction to append.

**/
VOID
IrAppendInstruction (
  IN OUT IR_BASIC_BLOCK  *Block,
  IN     IR_INSTRUCTION  *Instr
  );

/**
  Add edge between basic blocks.

  @param[in,out]  From          Source block.
  @param[in,out]  To            Destination block.

**/
VOID
IrAddEdge (
  IN OUT IR_BASIC_BLOCK  *From,
  IN OUT IR_BASIC_BLOCK  *To
  );

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
  );

/**
  Create constant operand.

  @param[in]      Value         Constant value.
  @param[in]      Type          Value type.

  @return  Constant operand.

**/
IR_OPERAND
IrConstant (
  IN  INT64     Value,
  IN  AST_TYPE  *Type
  );

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
  );

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
  );

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
  );

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
  );

/**
  Generate IR for entire module.

  @param[in,out]  Module        IR module.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
IrGenerateModule (
  IN OUT IR_MODULE  *Module
  );

/**
  Print IR function in human-readable format.

  @param[in]      Func          IR function.
  @param[in]      Output        Output file.

**/
VOID
IrPrintFunction (
  IN  IR_FUNCTION  *Func,
  IN  FILE         *Output
  );

/**
  Print IR module in human-readable format.

  @param[in]      Module        IR module.
  @param[in]      Output        Output file.

**/
VOID
IrPrintModule (
  IN  IR_MODULE  *Module,
  IN  FILE       *Output
  );

/**
  Create symbol table.

  @param[in]      Size          Hash table size.

  @return  Pointer to symbol table, or NULL on error.

**/
IR_SYMBOL_TABLE *
IrCreateSymbolTable (
  IN  UINT32  Size
  );

/**
  Destroy symbol table.

  @param[in]      Table         Symbol table.

**/
VOID
IrDestroySymbolTable (
  IN  IR_SYMBOL_TABLE  *Table
  );

/**
  Add symbol to symbol table.

  @param[in,out]  Table         Symbol table.
  @param[in]      Name          Symbol name.
  @param[in]      Operand       Operand for this symbol.
  @param[in]      Type          Symbol type.
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
  IN     BOOLEAN          IsParameter,
  IN     UINT32           ParamIndex
  );

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
  );

#endif // MMIX_IR_H_
