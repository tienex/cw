/**
  @file CodeGen.c

  MMIX code generator implementation.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/compiler/MmixCodeGen.h"

/**
  Create register allocator.

  @param[in]      VirtRegCount  Number of virtual registers.

  @return  Pointer to allocator, or NULL on error.

**/
REG_ALLOC *
RegAllocCreate (
  IN  UINT32  VirtRegCount
  )
{
  REG_ALLOC  *Alloc;

  Alloc = (REG_ALLOC *)calloc (1, sizeof (REG_ALLOC));
  if (Alloc == NULL) {
    return NULL;
  }

  Alloc->VirtRegCount = VirtRegCount;
  Alloc->VirtToPhys = (UINT32 *)calloc (VirtRegCount, sizeof (UINT32));
  Alloc->RegInUse = (BOOLEAN *)calloc (256, sizeof (BOOLEAN));
  Alloc->NextPhysReg = MMIX_REG_LOCAL_START;
  Alloc->MaxPhysReg = MMIX_REG_LOCAL_START;

  if (Alloc->VirtToPhys == NULL || Alloc->RegInUse == NULL) {
    if (Alloc->VirtToPhys != NULL) {
      free (Alloc->VirtToPhys);
    }
    if (Alloc->RegInUse != NULL) {
      free (Alloc->RegInUse);
    }
    free (Alloc);
    return NULL;
  }

  //
  // Initialize all virtual registers to unmapped
  //
  for (UINT32 i = 0; i < VirtRegCount; i++) {
    Alloc->VirtToPhys[i] = 0xFFFFFFFF;
  }

  return Alloc;
}

/**
  Destroy register allocator.

  @param[in]      Alloc         Register allocator.

**/
VOID
RegAllocDestroy (
  IN  REG_ALLOC  *Alloc
  )
{
  if (Alloc == NULL) {
    return;
  }

  if (Alloc->VirtToPhys != NULL) {
    free (Alloc->VirtToPhys);
  }

  if (Alloc->RegInUse != NULL) {
    free (Alloc->RegInUse);
  }

  free (Alloc);
}

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
  )
{
  UINT32  PhysReg;

  if (Alloc == NULL) {
    return 0;
  }

  //
  // Check if already allocated
  //
  if (VirtReg < Alloc->VirtRegCount && Alloc->VirtToPhys[VirtReg] != 0xFFFFFFFF) {
    return Alloc->VirtToPhys[VirtReg];
  }

  //
  // Allocate new physical register
  //
  PhysReg = Alloc->NextPhysReg;
  if (PhysReg >= MMIX_REG_STACK) {
    //
    // Out of registers, would need spilling
    // For now, just wrap around
    //
    PhysReg = MMIX_REG_LOCAL_START;
  }

  Alloc->NextPhysReg = PhysReg + 1;
  if (PhysReg > Alloc->MaxPhysReg) {
    Alloc->MaxPhysReg = PhysReg;
  }

  if (VirtReg < Alloc->VirtRegCount) {
    Alloc->VirtToPhys[VirtReg] = PhysReg;
  }

  Alloc->RegInUse[PhysReg] = TRUE;

  return PhysReg;
}

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
  )
{
  CODEGEN_CONTEXT  *Context;

  Context = (CODEGEN_CONTEXT *)calloc (1, sizeof (CODEGEN_CONTEXT));
  if (Context == NULL) {
    return NULL;
  }

  Context->Module = Module;
  Context->Output = Output;
  Context->CurrentFunc = NULL;
  Context->RegAlloc = NULL;
  Context->LabelCounter = 0;
  Context->ErrorCount = 0;

  return Context;
}

/**
  Destroy code generator context.

  @param[in]      Context       Code generation context.

**/
VOID
CodeGenDestroy (
  IN  CODEGEN_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return;
  }

  if (Context->RegAlloc != NULL) {
    RegAllocDestroy (Context->RegAlloc);
  }

  free (Context);
}

/**
  Emit assembly directive.

  @param[in,out]  Context       Code generation context.
  @param[in]      Directive     Directive string.

**/
VOID
CodeGenEmitDirective (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     CONST CHAR8      *Directive
  )
{
  fprintf (Context->Output, "%s\n", Directive);
}

/**
  Emit assembly label.

  @param[in,out]  Context       Code generation context.
  @param[in]      Label         Label name.

**/
VOID
CodeGenEmitLabel (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     CONST CHAR8      *Label
  )
{
  fprintf (Context->Output, "%s:\n", Label);
}

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
  )
{
  fprintf (Context->Output, "    %-8s %s\n", Opcode, Operands);
}

/**
  Get operand string for IR operand.

  @param[in]      Context       Code generation context.
  @param[in]      Operand       IR operand.
  @param[out]     Buffer        Output buffer.
  @param[in]      BufferSize    Buffer size.

**/
STATIC
VOID
CodeGenGetOperand (
  IN  CODEGEN_CONTEXT  *Context,
  IN  IR_OPERAND       *Operand,
  OUT CHAR8            *Buffer,
  IN  UINT32           BufferSize
  )
{
  switch (Operand->Type) {
    case IR_OPERAND_REG:
      {
        UINT32  PhysReg = RegAllocGet (Context->RegAlloc, Operand->RegNum);
        snprintf (Buffer, BufferSize, "$%u", PhysReg);
      }
      break;

    case IR_OPERAND_CONST:
      snprintf (Buffer, BufferSize, "%lld", (long long)Operand->ConstValue);
      break;

    case IR_OPERAND_SYMBOL:
      snprintf (Buffer, BufferSize, "%s", Operand->SymbolName);
      break;

    case IR_OPERAND_LABEL:
      snprintf (Buffer, BufferSize, "L%u", Operand->LabelId);
      break;

    default:
      snprintf (Buffer, BufferSize, "$0");
      break;
  }
}

/**
  Emit function prologue.

  @param[in,out]  Context       Code generation context.
  @param[in]      Func          IR function.

**/
VOID
CodeGenEmitPrologue (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_FUNCTION      *Func
  )
{
  CHAR8  Buf[256];

  //
  // Save return address and frame
  //
  CodeGenEmitInstr (Context, "PUSHJ", "$255,0");  // Will be fixed up

  //
  // Allocate stack space if needed
  //
  if (Context->RegAlloc->MaxPhysReg > MMIX_REG_LOCAL_START + 32) {
    UINT32  LocalCount = Context->RegAlloc->MaxPhysReg - MMIX_REG_LOCAL_START;
    snprintf (Buf, sizeof (Buf), "$254,$254,%u", LocalCount * 8);
    CodeGenEmitInstr (Context, "SUBU", Buf);
  }

  (void)Func;
}

/**
  Emit function epilogue.

  @param[in,out]  Context       Code generation context.
  @param[in]      Func          IR function.

**/
VOID
CodeGenEmitEpilogue (
  IN OUT CODEGEN_CONTEXT  *Context,
  IN     IR_FUNCTION      *Func
  )
{
  CHAR8  Buf[256];

  //
  // Deallocate stack space if needed
  //
  if (Context->RegAlloc->MaxPhysReg > MMIX_REG_LOCAL_START + 32) {
    UINT32  LocalCount = Context->RegAlloc->MaxPhysReg - MMIX_REG_LOCAL_START;
    snprintf (Buf, sizeof (Buf), "$254,$254,%u", LocalCount * 8);
    CodeGenEmitInstr (Context, "ADDU", Buf);
  }

  //
  // Return
  //
  CodeGenEmitInstr (Context, "POP", "0,0");

  (void)Func;
}

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
  )
{
  CHAR8  Dst[64], Src1[64], Src2[64], Operands[256];

  //
  // Get operand strings
  //
  CodeGenGetOperand (Context, &Instr->Dst, Dst, sizeof (Dst));
  CodeGenGetOperand (Context, &Instr->Src1, Src1, sizeof (Src1));
  CodeGenGetOperand (Context, &Instr->Src2, Src2, sizeof (Src2));

  switch (Instr->Opcode) {
    //
    // Arithmetic
    //
    case IR_ADD:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "ADDU", Operands);
      break;

    case IR_SUB:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "SUBU", Operands);
      break;

    case IR_MUL:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "MULU", Operands);
      break;

    case IR_DIV:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "DIVU", Operands);
      break;

    case IR_MOD:
      //
      // MMIX doesn't have MOD, use DIV and multiply back
      //
      snprintf (Operands, sizeof (Operands), "$255,%s,%s", Src1, Src2);
      CodeGenEmitInstr (Context, "DIVU", Operands);
      snprintf (Operands, sizeof (Operands), "$255,$255,%s", Src2);
      CodeGenEmitInstr (Context, "MULU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,$255", Dst, Src1);
      CodeGenEmitInstr (Context, "SUBU", Operands);
      break;

    case IR_NEG:
      snprintf (Operands, sizeof (Operands), "%s,$0,%s", Dst, Src1);
      CodeGenEmitInstr (Context, "SUBU", Operands);
      break;

    //
    // Bitwise
    //
    case IR_AND:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "AND", Operands);
      break;

    case IR_OR:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "OR", Operands);
      break;

    case IR_XOR:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "XOR", Operands);
      break;

    case IR_NOT:
      snprintf (Operands, sizeof (Operands), "%s,%s,0xFFFFFFFFFFFFFFFF", Dst, Src1);
      CodeGenEmitInstr (Context, "XOR", Operands);
      break;

    case IR_SHL:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "SLU", Operands);
      break;

    case IR_SHR:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "SRU", Operands);
      break;

    //
    // Rotate operations (MMIX extension)
    // Synthesized from shift operations: ROL(x,n) = (x << n) | (x >> (64-n))
    //
    case IR_ROL:
      {
        // ROL: dst = (src1 << src2) | (src1 >> (64 - src2))
        // Use $32 and $33 as temporary registers

        // $32 = src1 << src2 (left shift part)
        snprintf (Operands, sizeof (Operands), "$32,%s,%s", Src1, Src2);
        CodeGenEmitInstr (Context, "SLU", Operands);

        // $33 = 64 - src2 (compute right shift amount)
        snprintf (Operands, sizeof (Operands), "$33,64,%s", Src2);
        CodeGenEmitInstr (Context, "SUBU", Operands);

        // $33 = src1 >> $33 (right shift part)
        snprintf (Operands, sizeof (Operands), "$33,%s,$33", Src1);
        CodeGenEmitInstr (Context, "SRU", Operands);

        // dst = $32 | $33 (combine)
        snprintf (Operands, sizeof (Operands), "%s,$32,$33", Dst);
        CodeGenEmitInstr (Context, "OR", Operands);
      }
      break;

    case IR_ROR:
      {
        // ROR: dst = (src1 >> src2) | (src1 << (64 - src2))
        // Use $32 and $33 as temporary registers

        // $32 = src1 >> src2 (right shift part)
        snprintf (Operands, sizeof (Operands), "$32,%s,%s", Src1, Src2);
        CodeGenEmitInstr (Context, "SRU", Operands);

        // $33 = 64 - src2 (compute left shift amount)
        snprintf (Operands, sizeof (Operands), "$33,64,%s", Src2);
        CodeGenEmitInstr (Context, "SUBU", Operands);

        // $33 = src1 << $33 (left shift part)
        snprintf (Operands, sizeof (Operands), "$33,%s,$33", Src1);
        CodeGenEmitInstr (Context, "SLU", Operands);

        // dst = $32 | $33 (combine)
        snprintf (Operands, sizeof (Operands), "%s,$32,$33", Dst);
        CodeGenEmitInstr (Context, "OR", Operands);
      }
      break;

    //
    // Bit field operations (MMIX extension)
    //
    case IR_BFEXT:
      {
        //
        // Bit field extract: dst = src[index:count]
        // 1. Shift right by index: dst = src >> index
        // 2. Mask to count bits: dst = dst & ((1 << count) - 1)
        //
        CHAR8  Count[64];

        CodeGenGetOperand (Context, &Instr->Args[0], Count, sizeof (Count));

        // Shift right by index
        snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
        CodeGenEmitInstr (Context, "SRU", Operands);

        // Create mask: (1 << count) - 1
        // Use temp register $32 for mask calculation
        snprintf (Operands, sizeof (Operands), "$32,1,%s", Count);
        CodeGenEmitInstr (Context, "SLU", Operands);
        snprintf (Operands, sizeof (Operands), "$32,$32,1");
        CodeGenEmitInstr (Context, "SUBU", Operands);

        // Mask the result
        snprintf (Operands, sizeof (Operands), "%s,%s,$32", Dst, Dst);
        CodeGenEmitInstr (Context, "AND", Operands);
      }
      break;

    case IR_BFINS:
      {
        //
        // Bit field insert: dst[index:count] = src
        // 1. Create mask: mask = ((1 << count) - 1) << index
        // 2. Clear bits: dst = dst & ~mask
        // 3. Shift source: temp = src << index
        // 4. Mask source: temp = temp & mask
        // 5. Insert: dst = dst | temp
        //
        CHAR8  Count[64];

        CodeGenGetOperand (Context, &Instr->Args[0], Count, sizeof (Count));

        // Create mask: (1 << count) - 1
        // Use temp registers $32 and $33
        snprintf (Operands, sizeof (Operands), "$32,1,%s", Count);
        CodeGenEmitInstr (Context, "SLU", Operands);
        snprintf (Operands, sizeof (Operands), "$32,$32,1");
        CodeGenEmitInstr (Context, "SUBU", Operands);

        // Shift mask to position: mask = mask << index
        snprintf (Operands, sizeof (Operands), "$32,$32,%s", Src2);
        CodeGenEmitInstr (Context, "SLU", Operands);

        // Clear bits in destination: dst = dst & ~mask
        snprintf (Operands, sizeof (Operands), "$33,$32");
        CodeGenEmitInstr (Context, "NOR", Operands);
        snprintf (Operands, sizeof (Operands), "%s,%s,$33", Dst, Dst);
        CodeGenEmitInstr (Context, "AND", Operands);

        // Shift source to position: temp = src << index
        snprintf (Operands, sizeof (Operands), "$33,%s,%s", Src1, Src2);
        CodeGenEmitInstr (Context, "SLU", Operands);

        // Mask source: temp = temp & mask
        snprintf (Operands, sizeof (Operands), "$33,$33,$32");
        CodeGenEmitInstr (Context, "AND", Operands);

        // Insert: dst = dst | temp
        snprintf (Operands, sizeof (Operands), "%s,%s,$33", Dst, Dst);
        CodeGenEmitInstr (Context, "OR", Operands);
      }
      break;

    case IR_BFCONCAT:
      {
        //
        // Bit concatenation: dst = left .. right
        // Shift left operand by width of right operand, then OR
        // For simplicity, assume right operand is 8 bits (will need proper typing later)
        //

        // Shift left operand left by 8 (TODO: use actual bit width)
        // Use temp register $32
        snprintf (Operands, sizeof (Operands), "$32,%s,8", Src1);
        CodeGenEmitInstr (Context, "SLU", Operands);

        // OR with right operand
        snprintf (Operands, sizeof (Operands), "%s,$32,%s", Dst, Src2);
        CodeGenEmitInstr (Context, "OR", Operands);
      }
      break;

    //
    // Comparison
    //
    case IR_EQ:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "CMPU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,0", Dst, Dst);
      CodeGenEmitInstr (Context, "ZSZ", Operands);
      break;

    case IR_NE:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "CMPU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,1", Dst, Dst);
      CodeGenEmitInstr (Context, "ZSNZ", Operands);
      break;

    case IR_LT:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "CMPU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,1", Dst, Dst);
      CodeGenEmitInstr (Context, "ZSN", Operands);
      break;

    case IR_LE:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "CMPU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,1", Dst, Dst);
      CodeGenEmitInstr (Context, "ZSNP", Operands);
      break;

    case IR_GT:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "CMPU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,1", Dst, Dst);
      CodeGenEmitInstr (Context, "ZSP", Operands);
      break;

    case IR_GE:
      snprintf (Operands, sizeof (Operands), "%s,%s,%s", Dst, Src1, Src2);
      CodeGenEmitInstr (Context, "CMPU", Operands);
      snprintf (Operands, sizeof (Operands), "%s,%s,1", Dst, Dst);
      CodeGenEmitInstr (Context, "ZSNN", Operands);
      break;

    //
    // Memory
    //
    case IR_LOAD:
      snprintf (Operands, sizeof (Operands), "%s,%s,0", Dst, Src1);
      CodeGenEmitInstr (Context, "LDO", Operands);
      break;

    case IR_STORE:
      // STORE: Mem[Src1] = Src2
      // MMIX: STO $value, $addr, 0
      snprintf (Operands, sizeof (Operands), "%s,%s,0", Src2, Src1);
      CodeGenEmitInstr (Context, "STO", Operands);
      break;

    case IR_ALLOCA:
      // Allocate space on stack
      // For MMIX, we use register $254 as stack pointer
      // ALLOCA: dst = $254; $254 = $254 - size
      snprintf (Operands, sizeof (Operands), "%s,$254", Dst);
      CodeGenEmitInstr (Context, "SET", Operands);
      snprintf (Operands, sizeof (Operands), "$254,$254,%s", Src1);
      CodeGenEmitInstr (Context, "SUBU", Operands);
      break;

    case IR_ADDR:
      snprintf (Operands, sizeof (Operands), "%s,%s", Dst, Src1);
      CodeGenEmitInstr (Context, "GETA", Operands);
      break;

    //
    // Control flow
    //
    case IR_BR:
      {
        CHAR8  Label[64];
        snprintf (Label, sizeof (Label), "L%u", Instr->LabelId);
        CodeGenEmitInstr (Context, "JMP", Label);
      }
      break;

    case IR_BR_COND:
      {
        CHAR8  Label1[64], Label2[64];
        snprintf (Label1, sizeof (Label1), "L%u", Instr->LabelId);
        snprintf (Label2, sizeof (Label2), "L%u", Instr->LabelId2);

        //
        // Branch if non-zero
        //
        snprintf (Operands, sizeof (Operands), "%s,%s", Src1, Label1);
        CodeGenEmitInstr (Context, "BNZ", Operands);
        CodeGenEmitInstr (Context, "JMP", Label2);
      }
      break;

    case IR_CALL:
      {
        //
        // Set up arguments in registers $231-$246
        // GCC MMIX ABI: First 16 args in $231+, rest on stack
        //
        if (Instr->Args != NULL && Instr->ArgCount > 0) {
          for (UINT32 i = 0; i < Instr->ArgCount && i < 16; i++) {
            CHAR8  ArgReg[32];
            CHAR8  ArgOperand[256];

            //
            // Format argument operand
            //
            CodeGenGetOperand (Context, &Instr->Args[i], ArgOperand, sizeof (ArgOperand));

            //
            // Move argument to register $231+i (GCC MMIX calling convention)
            //
            snprintf (ArgReg, sizeof (ArgReg), "$%u", 231 + i);
            snprintf (Operands, sizeof (Operands), "%s,%s", ArgReg, ArgOperand);
            CodeGenEmitInstr (Context, "SET", Operands);
          }

          //
          // TODO: Handle more than 16 arguments (push to stack)
          //
          if (Instr->ArgCount > 16) {
            fprintf (stderr, "Warning: Functions with >16 arguments not yet supported\n");
          }
        }

        //
        // Call function
        //
        snprintf (Operands, sizeof (Operands), "$255,%s", Src1);
        CodeGenEmitInstr (Context, "PUSHJ", Operands);

        //
        // Move return value from $231 to destination
        // GCC MMIX ABI: return values in $231
        //
        if (Instr->Dst.Type != IR_OPERAND_NONE) {
          snprintf (Operands, sizeof (Operands), "%s,$231", Dst);
          CodeGenEmitInstr (Context, "SET", Operands);
        }
      }
      break;

    case IR_RET:
      if (Instr->Src1.Type != IR_OPERAND_NONE) {
        //
        // Move return value to $231 (GCC MMIX ABI)
        //
        snprintf (Operands, sizeof (Operands), "$231,%s", Src1);
        CodeGenEmitInstr (Context, "SET", Operands);
      }
      CodeGenEmitEpilogue (Context, Context->CurrentFunc);
      break;

    case IR_MOVE:
      snprintf (Operands, sizeof (Operands), "%s,%s", Dst, Src1);
      CodeGenEmitInstr (Context, "SET", Operands);
      break;

    case IR_NOP:
      CodeGenEmitInstr (Context, "SWYM", "0,0,0");
      break;

    default:
      //
      // Unsupported instruction
      //
      fprintf (stderr, "Unsupported IR opcode: %d\n", Instr->Opcode);
      break;
  }

  return MMIX_SUCCESS;
}

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
  )
{
  CHAR8  Label[64];

  //
  // Emit block label
  //
  snprintf (Label, sizeof (Label), "L%u", Block->Id);
  CodeGenEmitLabel (Context, Label);

  //
  // Generate code for each instruction
  //
  for (IR_INSTRUCTION *Instr = Block->First; Instr != NULL; Instr = Instr->Next) {
    CodeGenInstruction (Context, Instr);
  }

  return MMIX_SUCCESS;
}

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
  )
{
  //
  // Create register allocator for this function
  //
  Context->RegAlloc = RegAllocCreate (Func->NextRegNum);
  if (Context->RegAlloc == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Context->CurrentFunc = Func;

  //
  // Emit function label and directives
  //
  fprintf (Context->Output, "\n");
  CodeGenEmitDirective (Context, ".text");
  fprintf (Context->Output, ".globl %s\n", Func->Name);
  CodeGenEmitLabel (Context, Func->Name);

  //
  // Emit prologue
  //
  CodeGenEmitPrologue (Context, Func);

  //
  // Generate code for each basic block
  //
  for (UINT32 i = 0; i < Func->BlockCount; i++) {
    CodeGenBasicBlock (Context, Func->Blocks[i]);
  }

  //
  // Clean up register allocator
  //
  RegAllocDestroy (Context->RegAlloc);
  Context->RegAlloc = NULL;
  Context->CurrentFunc = NULL;

  return MMIX_SUCCESS;
}

/**
  Generate MMIX assembly for module.

  @param[in,out]  Context       Code generation context.

  @return  MMIX_SUCCESS if successful, error code otherwise.

**/
MMIX_STATUS
CodeGenModule (
  IN OUT CODEGEN_CONTEXT  *Context
  )
{
  if (Context == NULL || Context->Module == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Emit header comment
  //
  fprintf (Context->Output, "# Generated MMIX assembly\n");
  fprintf (Context->Output, "# Source: %s\n",
           Context->Module->SourceFile ? Context->Module->SourceFile : "<unknown>");

  //
  // Generate code for each function
  //
  for (UINT32 i = 0; i < Context->Module->FunctionCount; i++) {
    CodeGenFunction (Context, Context->Module->Functions[i]);
  }

  if (Context->ErrorCount > 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  return MMIX_SUCCESS;
}
