/**
  @file MmixCodeGen.h

  MMIX code generator for C23 compiler.
  Converts IR to MMIX assembly.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#ifndef MMIX_CODEGEN_H_
#define MMIX_CODEGEN_H_

#include "MmixTypes.h"
#include "MmixIr.h"

/**
  MMIX register allocation

  Register conventions:
  $0-$31    - Caller-saved temporaries
  $32-$63   - Callee-saved registers
  $64-$127  - Argument/temporary registers
  $128-$254 - Local variables
  $255      - Reserved (rR - return value register)

  Special registers:
  rG (Global threshold) - Divides local/global registers
  rL (Local registers) - Number of local registers used
  rJ (Return address) - Function return address
  rO (Register stack offset)
**/

#define MMIX_REG_ZERO       0    // $0 - Always zero
#define MMIX_REG_RET_START  32   // $32 - First return value register
#define MMIX_REG_ARG_START  64   // $64 - First argument register
#define MMIX_REG_LOCAL_START 128 // $128 - First local register
#define MMIX_REG_STACK      254  // $254 - Stack pointer
#define MMIX_REG_RETVAL     255  // $255 - Return value (rR)

/**
  Register allocation map
**/
typedef struct {
  UINT32  *VirtToPhys;      // Virtual to physical register map
  UINT32  VirtRegCount;     // Number of virtual registers
  UINT32  NextPhysReg;      // Next available physical register
  UINT32  MaxPhysReg;       // Maximum physical register used
  BOOLEAN *RegInUse;        // Register usage bitmap
} REG_ALLOC;

/**
  Code generation context
**/
typedef struct {
  IR_MODULE      *Module;           // IR module
  IR_FUNCTION    *CurrentFunc;      // Current function
  REG_ALLOC      *RegAlloc;         // Register allocator
  FILE           *Output;           // Output file
  UINT32         LabelCounter;      // Label counter
  UINT32         ErrorCount;        // Error count
} CODEGEN_CONTEXT;

/**
  Create register allocator.

  @param[in]      VirtRegCount  Number of virtual registers.

  @return  Pointer to allocator, or NULL on error.

**/
REG_ALLOC *
RegAllocCreate (
  IN  UINT32  VirtRegCount
  );

/**
  Destroy register allocator.

  @param[in]      Alloc         Register allocator.

**/
VOID
RegAllocDestroy (
  IN  REG_ALLOC  *Alloc
  );

/**
  Allocate physical register for virtual register.

  @param[in,out]  Alloc         Register allocator.
  @param[in]      VirtReg       Virtual register number.

  @return  Physical register number.

**/
UINT32
RegAllocGet (
  IN OUT REG_ALLOC  *Alloc,
  IN     UINT32     VirtReg
  );

/**
  Create code generator context.

  @param[in]      Module        IR module.
  @param[in]      Output        Output file.

  @return  Pointer to context, or NULL on error.

**/
CODEGEN_CONTEXT *
CodeGenCreate (
  IN  IR_MODULE  *Module,
  IN  FILE       *Output
  );

/**
  Destroy code generator context.

  @param[in]      Context       Code generation context.

**/
VOID
CodeGenDestroy (
  IN  CODEGEN_CONTEXT  *Context
  );

/**
  Generate MMIX assembly for IR instruction.

  @param[in,out]  Context       Code generation context.
  @param[in]      Instr         IR instruction.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
CodeGenInstruction (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_INSTRUCTION   *Instr
  );

/**
  Generate MMIX assembly for basic block.

  @param[in,out]  Context       Code generation context.
  @param[in]      Block         Basic block.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
CodeGenBasicBlock (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_BASIC_BLOCK   *Block
  );

/**
  Generate MMIX assembly for function.

  @param[in,out]  Context       Code generation context.
  @param[in]      Func          IR function.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
CodeGenFunction (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_FUNCTION      *Func
  );

/**
  Generate MMIX assembly for module.

  @param[in,out]  Context       Code generation context.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
CodeGenModule (
  IN OUT CODEGEN_CONTEXT  *Context
  );

/**
  Emit assembly directive.

  @param[in,out]  Context       Code generation context.
  @param[in]      Directive     Directive string.

**/
VOID
CodeGenEmitDirective (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     CONST CHAR8      *Directive
  );

/**
  Emit assembly label.

  @param[in,out]  Context       Code generation context.
  @param[in]      Label         Label name.

**/
VOID
CodeGenEmitLabel (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     CONST CHAR8      *Label
  );

/**
  Emit MMIX instruction.

  @param[in,out]  Context       Code generation context.
  @param[in]      Opcode        Instruction opcode.
  @param[in]      Operands      Operand string.

**/
VOID
CodeGenEmitInstr (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     CONST CHAR8      *Opcode,
  IN     CONST CHAR8      *Operands
  );

/**
  Emit function prologue.

  @param[in,out]  Context       Code generation context.
  @param[in]      Func          IR function.

**/
VOID
CodeGenEmitPrologue (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_FUNCTION      *Func
  );

/**
  Emit function epilogue.

  @param[in,out]  Context       Code generation context.
  @param[in]      Func          IR function.

**/
VOID
CodeGenEmitEpilogue (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_FUNCTION      *Func
  );

#endif // MMIX_CODEGEN_H_
