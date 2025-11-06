/** @file
  MMIX instruction execution implementation.

  This file implements instruction fetch, decode, and execution for all
  MMIX instructions including standard instructions, compressed instructions,
  and extended instructions.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include "../../include/MmixCore.h"
#include "../../include/MmixMemory.h"
#include "../../include/private/MmixFpu.h"
#include "../../include/private/MmixCompressed.h"

//
// Forward declarations for instruction handlers
//

STATIC MMIX_STATUS ExecuteLoad (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteStore (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteArithmetic (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteLogical (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteBranch (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteFloatingPoint (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteVector (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteMatrix (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteMachineLearning (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteHypervisor (IN OUT MMIX_CPU_STATE *CpuState, IN MMIX_INSTRUCTION Inst);
STATIC MMIX_STATUS ExecuteCompressed (IN OUT MMIX_CPU_STATE *CpuState, IN UINT16 CompInst);

/**
  Execute a single instruction.

  Fetches, decodes, and executes one instruction at the current PC.
  Updates CPU state and PC appropriately. Handles both standard 32-bit
  and compressed 16-bit instructions.

  @param[in,out]  CpuState      Pointer to CPU state.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred during execution.

**/
MMIX_STATUS
MmixCpuExecuteInstruction (
  IN OUT MMIX_CPU_STATE  *CpuState
  )
{
  MMIX_INSTRUCTION  Instruction;
  BOOLEAN           Compressed;
  MMIX_STATUS       Status;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Check if CPU is halted
  //
  if (CpuState->ExecutionMode == MmixExecutionModeHalted) {
    return MMIX_SUCCESS;
  }

  //
  // Increment cycle counter
  //
  CpuState->CycleCount++;
  CpuState->SpecialRegisters[MMIX_rC] = CpuState->CycleCount;

  //
  // Check for pending interrupts
  //
  Status = MmixCpuDeliverInterrupt (CpuState);
  if (MMIX_IS_ERROR (Status)) {
    return Status;
  }

  //
  // Fetch instruction
  //
  Status = MmixCpuFetchInstruction (CpuState, &Instruction, &Compressed);
  if (MMIX_IS_ERROR (Status)) {
    return Status;
  }

  //
  // Decode and execute
  //
  Status = MmixCpuDecodeExecute (CpuState, Instruction, Compressed);
  if (MMIX_IS_ERROR (Status)) {
    return Status;
  }

  //
  // Increment instruction counter
  //
  CpuState->InstructionCount++;

  return MMIX_SUCCESS;
}

/**
  Execute multiple instructions.

  Continues executing instructions until an exception occurs,
  a breakpoint is hit, or the requested count is reached.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Count         Number of instructions to execute (0 = infinite).

  @retval MMIX_SUCCESS          Execution completed normally.
  @retval Others                Exception occurred during execution.

**/
MMIX_STATUS
MmixCpuExecute (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT64          Count
  )
{
  MMIX_STATUS  Status;
  UINT64       Executed;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Executed = 0;

  while (Count == 0 || Executed < Count) {
    Status = MmixCpuExecuteInstruction (CpuState);
    if (MMIX_IS_ERROR (Status)) {
      return Status;
    }

    if (CpuState->ExecutionMode == MmixExecutionModeHalted) {
      break;
    }

    Executed++;
  }

  return MMIX_SUCCESS;
}

/**
  Fetch an instruction from memory.

  Fetches a 32-bit instruction or 16-bit compressed instruction
  from the address pointed to by PC. Handles endianness conversion.

  @param[in]      CpuState      Pointer to CPU state.
  @param[out]     Instruction   Pointer to receive instruction.
  @param[out]     Compressed    Pointer to receive compressed flag.

  @retval MMIX_SUCCESS          Instruction fetched successfully.
  @retval Others                Error occurred during fetch.

**/
MMIX_STATUS
MmixCpuFetchInstruction (
  IN  MMIX_CPU_STATE       *CpuState,
  OUT MMIX_INSTRUCTION     *Instruction,
  OUT BOOLEAN              *Compressed
  )
{
  MMIX_STATUS  Status;
  UINT16       FirstHalf;
  UINT32       FullInstruction;

  if (CpuState == NULL || Instruction == NULL || Compressed == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Check PC alignment (must be at least 2-byte aligned for compressed)
  //
  if ((CpuState->Pc & 0x1) != 0) {
    return MmixCpuRaiseException (CpuState, MmixExceptionAlignment, CpuState->Pc);
  }

  //
  // Read first 16 bits to check if compressed
  //
  Status = MmixMemoryRead (
             CpuState->MemoryState,
             CpuState->Pc,
             &FirstHalf,
             sizeof (UINT16),
             TRUE  // This is an instruction fetch
             );
  if (MMIX_IS_ERROR (Status)) {
    return MmixCpuRaiseException (CpuState, MmixExceptionPageFault, CpuState->Pc);
  }

  //
  // Convert endianness if needed
  //
  if (CpuState->EndiannessMode == MmixEndianLittle) {
    FirstHalf = (UINT16)MmixLittleEndianToHost (FirstHalf, sizeof (UINT16));
  } else {
    FirstHalf = (UINT16)MmixBigEndianToHost (FirstHalf, sizeof (UINT16));
  }

  //
  // Check if this is a compressed instruction
  // Compressed instructions have top 4 bits >= 0x8
  //
  if ((FirstHalf & 0xF000) >= 0x8000) {
    //
    // This is a compressed instruction
    //
    *Compressed = TRUE;
    Instruction->Uint32 = FirstHalf;  // Store in lower 16 bits
    CpuState->Pc += 2;
    return MMIX_SUCCESS;
  }

  //
  // This is a full 32-bit instruction
  // We need to read the complete 32 bits
  //
  Status = MmixMemoryRead (
             CpuState->MemoryState,
             CpuState->Pc,
             &FullInstruction,
             sizeof (UINT32),
             TRUE
             );
  if (MMIX_IS_ERROR (Status)) {
    return MmixCpuRaiseException (CpuState, MmixExceptionPageFault, CpuState->Pc);
  }

  //
  // Convert endianness
  //
  if (CpuState->EndiannessMode == MmixEndianLittle) {
    FullInstruction = (UINT32)MmixLittleEndianToHost (FullInstruction, sizeof (UINT32));
  } else {
    FullInstruction = (UINT32)MmixBigEndianToHost (FullInstruction, sizeof (UINT32));
  }

  *Compressed = FALSE;
  Instruction->Uint32 = FullInstruction;
  CpuState->Pc += 4;

  return MMIX_SUCCESS;
}

/**
  Decode and execute an instruction.

  Decodes the instruction opcode and dispatches to the appropriate
  execution handler. Updates CPU state and sets PC to next instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Instruction   The instruction to execute.
  @param[in]      Compressed    TRUE if compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred during execution.

**/
MMIX_STATUS
MmixCpuDecodeExecute (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Instruction,
  IN     BOOLEAN          Compressed
  )
{
  UINT8   Opcode;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Handle compressed instructions separately
  //
  if (Compressed) {
    return ExecuteCompressed (CpuState, (UINT16)Instruction.Uint32);
  }

  //
  // Extract opcode
  //
  Opcode = Instruction.Fields.Opcode;

  //
  // Dispatch based on opcode range
  //
  if (Opcode >= 0x00 && Opcode <= 0x0F) {
    //
    // Load instructions
    //
    return ExecuteLoad (CpuState, Instruction);
  } else if (Opcode >= 0x10 && Opcode <= 0x1F) {
    //
    // Store instructions
    //
    return ExecuteStore (CpuState, Instruction);
  } else if (Opcode >= 0x20 && Opcode <= 0x3F) {
    //
    // Arithmetic instructions
    //
    return ExecuteArithmetic (CpuState, Instruction);
  } else if (Opcode >= 0x40 && Opcode <= 0x4F) {
    //
    // Logical instructions
    //
    return ExecuteLogical (CpuState, Instruction);
  } else if (Opcode >= 0x50 && Opcode <= 0x5F) {
    //
    // Branch instructions
    //
    return ExecuteBranch (CpuState, Instruction);
  } else if (Opcode >= 0x60 && Opcode <= 0x7F) {
    //
    // Floating-point instructions
    //
    return ExecuteFloatingPoint (CpuState, Instruction);
  } else if (Opcode >= 0xD0 && Opcode <= 0xDF) {
    //
    // Machine learning instructions
    //
    return ExecuteMachineLearning (CpuState, Instruction);
  } else if (Opcode >= 0xE0 && Opcode <= 0xEF) {
    //
    // Vector instructions
    //
    return ExecuteVector (CpuState, Instruction);
  } else if (Opcode >= 0xF0 && Opcode <= 0xF7) {
    //
    // Matrix instructions
    //
    return ExecuteMatrix (CpuState, Instruction);
  } else if (Opcode >= 0xF8 && Opcode <= 0xFE) {
    //
    // Hypervisor instructions
    //
    return ExecuteHypervisor (CpuState, Instruction);
  }

  //
  // Invalid or unimplemented instruction
  //
  return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
}

/**
  Execute load instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Inst          The instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteLoad (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  MMIX_STATUS  Status;
  UINT64       YValue, ZValue;
  UINT64       Address;
  UINT64       LoadedValue;
  UINT8        Size;
  BOOLEAN      Signed;

  //
  // Read Y and Z registers
  //
  MmixCpuReadRegister (CpuState, Inst.Fields.Y, &YValue);
  MmixCpuReadRegister (CpuState, Inst.Fields.Z, &ZValue);

  //
  // Calculate effective address
  //
  Address = YValue + ZValue;

  //
  // Determine size and signedness based on opcode
  //
  switch (Inst.Fields.Opcode) {
    case 0x00: // LDB - Load byte unsigned
    case 0x01: // LDBU
      Size = 1;
      Signed = FALSE;
      break;

    case 0x02: // LDW - Load wyde unsigned
    case 0x03: // LDWU
      Size = 2;
      Signed = FALSE;
      break;

    case 0x04: // LDT - Load tetra unsigned
    case 0x05: // LDTU
      Size = 4;
      Signed = FALSE;
      break;

    case 0x06: // LDO - Load octa
    case 0x07: // LDOU
      Size = 8;
      Signed = FALSE;
      break;

    case 0x08: // LDBS - Load byte signed
      Size = 1;
      Signed = TRUE;
      break;

    case 0x09: // LDWS - Load wyde signed
      Size = 2;
      Signed = TRUE;
      break;

    case 0x0A: // LDTS - Load tetra signed
      Size = 4;
      Signed = TRUE;
      break;

    default:
      Size = 8;
      Signed = FALSE;
      break;
  }

  //
  // Read from memory
  //
  LoadedValue = 0;
  Status = MmixMemoryRead (
             CpuState->MemoryState,
             Address,
             &LoadedValue,
             Size,
             FALSE
             );
  if (MMIX_IS_ERROR (Status)) {
    return MmixCpuRaiseException (CpuState, MmixExceptionPageFault, Address);
  }

  //
  // Handle endianness
  //
  if (CpuState->EndiannessMode == MmixEndianLittle) {
    LoadedValue = MmixLittleEndianToHost (LoadedValue, Size);
  } else {
    LoadedValue = MmixBigEndianToHost (LoadedValue, Size);
  }

  //
  // Sign extend if needed
  //
  if (Signed) {
    switch (Size) {
      case 1:
        if (LoadedValue & 0x80) {
          LoadedValue |= 0xFFFFFFFFFFFFFF00ULL;
        }
        break;
      case 2:
        if (LoadedValue & 0x8000) {
          LoadedValue |= 0xFFFFFFFFFFFF0000ULL;
        }
        break;
      case 4:
        if (LoadedValue & 0x80000000) {
          LoadedValue |= 0xFFFFFFFF00000000ULL;
        }
        break;
    }
  }

  //
  // Write to destination register
  //
  MmixCpuWriteRegister (CpuState, Inst.Fields.X, LoadedValue);

  return MMIX_SUCCESS;
}

/**
  Execute store instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Inst          The instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteStore (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  MMIX_STATUS  Status;
  UINT64       XValue, YValue, ZValue;
  UINT64       Address;
  UINT64       StoreValue;
  UINT8        Size;

  //
  // Read registers
  //
  MmixCpuReadRegister (CpuState, Inst.Fields.X, &XValue);
  MmixCpuReadRegister (CpuState, Inst.Fields.Y, &YValue);
  MmixCpuReadRegister (CpuState, Inst.Fields.Z, &ZValue);

  //
  // Calculate effective address
  //
  Address = YValue + ZValue;

  //
  // Determine size based on opcode
  //
  switch (Inst.Fields.Opcode) {
    case 0x10: // STB - Store byte
    case 0x11: // STBU
      Size = 1;
      break;

    case 0x12: // STW - Store wyde
    case 0x13: // STWU
      Size = 2;
      break;

    case 0x14: // STT - Store tetra
    case 0x15: // STTU
      Size = 4;
      break;

    case 0x16: // STO - Store octa
    case 0x17: // STOU
      Size = 8;
      break;

    default:
      Size = 8;
      break;
  }

  //
  // Handle endianness
  //
  StoreValue = XValue;
  if (CpuState->EndiannessMode == MmixEndianLittle) {
    StoreValue = MmixHostToLittleEndian (StoreValue, Size);
  } else {
    StoreValue = MmixHostToBigEndian (StoreValue, Size);
  }

  //
  // Write to memory
  //
  Status = MmixMemoryWrite (
             CpuState->MemoryState,
             Address,
             &StoreValue,
             Size
             );
  if (MMIX_IS_ERROR (Status)) {
    return MmixCpuRaiseException (CpuState, MmixExceptionPageFault, Address);
  }

  return MMIX_SUCCESS;
}

/**
  Execute arithmetic instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Inst          The instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteArithmetic (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  UINT64   YValue, ZValue;
  UINT64   Result;
  BOOLEAN  UseImmediate;

  //
  // Determine if this uses immediate (odd opcodes)
  //
  UseImmediate = (Inst.Fields.Opcode & 0x1) != 0;

  //
  // Read Y register
  //
  MmixCpuReadRegister (CpuState, Inst.Fields.Y, &YValue);

  //
  // Get Z value (register or immediate)
  //
  if (UseImmediate) {
    ZValue = Inst.Fields.Z;
  } else {
    MmixCpuReadRegister (CpuState, Inst.Fields.Z, &ZValue);
  }

  //
  // Execute based on opcode
  //
  switch (Inst.Fields.Opcode & 0xFE) {  // Mask off immediate bit
    case 0x20: // ADD/ADDI
      Result = YValue + ZValue;
      //
      // Check for signed overflow
      //
      if (((YValue ^ Result) & (ZValue ^ Result) & 0x8000000000000000ULL) != 0) {
        CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_OVERFLOW;
      }
      break;

    case 0x22: // ADDU/ADDUI
      Result = YValue + ZValue;
      break;

    case 0x24: // SUB/SUBI
      Result = YValue - ZValue;
      //
      // Check for signed overflow
      //
      if (((YValue ^ ZValue) & (YValue ^ Result) & 0x8000000000000000ULL) != 0) {
        CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_OVERFLOW;
      }
      break;

    case 0x26: // SUBU/SUBUI
      Result = YValue - ZValue;
      break;

    case 0x28: // MUL/MULI
    case 0x2A: // MULU/MULUI
      Result = YValue * ZValue;
      //
      // Store high 64 bits in rH
      //
      CpuState->SpecialRegisters[MMIX_rH] = 0;  // Simplified - full 128-bit math needed
      break;

    case 0x2C: // DIV/DIVI
      if (ZValue == 0) {
        CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_DIVIDE_BY_ZERO;
        return MmixCpuRaiseException (CpuState, MmixExceptionDivideByZero, 0);
      }
      Result = (INT64)YValue / (INT64)ZValue;
      CpuState->SpecialRegisters[MMIX_rR] = (INT64)YValue % (INT64)ZValue;
      break;

    case 0x2E: // DIVU/DIVUI
      if (ZValue == 0) {
        CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_DIVIDE_BY_ZERO;
        return MmixCpuRaiseException (CpuState, MmixExceptionDivideByZero, 0);
      }
      Result = YValue / ZValue;
      CpuState->SpecialRegisters[MMIX_rR] = YValue % ZValue;
      break;

    case 0x34: // SL/SLI - Shift left
      Result = YValue << (ZValue & 0x3F);
      break;

    case 0x38: // SR/SRI - Shift right (arithmetic)
      Result = (UINT64)((INT64)YValue >> (ZValue & 0x3F));
      break;

    case 0x3A: // SRU/SRUI - Shift right (logical)
      Result = YValue >> (ZValue & 0x3F);
      break;

    case 0x3C: // CMP/CMPI - Compare
      if ((INT64)YValue < (INT64)ZValue) {
        Result = (UINT64)-1;
      } else if (YValue == ZValue) {
        Result = 0;
      } else {
        Result = 1;
      }
      break;

    case 0x3E: // CMPU/CMPUI - Compare unsigned
      if (YValue < ZValue) {
        Result = (UINT64)-1;
      } else if (YValue == ZValue) {
        Result = 0;
      } else {
        Result = 1;
      }
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Write result to destination register
  //
  MmixCpuWriteRegister (CpuState, Inst.Fields.X, Result);

  return MMIX_SUCCESS;
}

/**
  Execute logical instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Inst          The instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.

**/
STATIC
MMIX_STATUS
ExecuteLogical (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  UINT64   YValue, ZValue;
  UINT64   Result;
  BOOLEAN  UseImmediate;

  //
  // Determine if this uses immediate
  //
  UseImmediate = (Inst.Fields.Opcode & 0x1) != 0;

  //
  // Read Y register
  //
  MmixCpuReadRegister (CpuState, Inst.Fields.Y, &YValue);

  //
  // Get Z value
  //
  if (UseImmediate) {
    ZValue = Inst.Fields.Z;
  } else {
    MmixCpuReadRegister (CpuState, Inst.Fields.Z, &ZValue);
  }

  //
  // Execute based on opcode
  //
  switch (Inst.Fields.Opcode & 0xFE) {
    case 0x40: // AND/ANDI
      Result = YValue & ZValue;
      break;

    case 0x42: // OR/ORI
      Result = YValue | ZValue;
      break;

    case 0x44: // XOR/XORI
      Result = YValue ^ ZValue;
      break;

    case 0x46: // ANDN/ANDNI
      Result = YValue & ~ZValue;
      break;

    case 0x48: // ORN/ORNI
      Result = YValue | ~ZValue;
      break;

    case 0x4A: // NAND/NANDI
      Result = ~(YValue & ZValue);
      break;

    case 0x4C: // NOR/NORI
      Result = ~(YValue | ZValue);
      break;

    case 0x4E: // NXOR/NXORI
      Result = ~(YValue ^ ZValue);
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Write result
  //
  MmixCpuWriteRegister (CpuState, Inst.Fields.X, Result);

  return MMIX_SUCCESS;
}

/**
  Execute branch instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Inst          The instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.

**/
STATIC
MMIX_STATUS
ExecuteBranch (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  UINT64   XValue;
  INT64    Offset;
  BOOLEAN  TakeBranch;
  BOOLEAN  Backward;

  //
  // Read X register
  //
  MmixCpuReadRegister (CpuState, Inst.Fields.X, &XValue);

  //
  // Determine if backward branch (odd opcodes)
  //
  Backward = (Inst.Fields.Opcode & 0x1) != 0;

  //
  // Calculate offset (YZ field is 16-bit signed)
  //
  Offset = (INT64)(INT16)Inst.Immediate16.YZ;
  if (Backward) {
    Offset = -Offset;
  }
  Offset *= 4;  // Multiply by 4 for byte offset

  //
  // Determine if branch should be taken
  //
  TakeBranch = FALSE;
  switch (Inst.Fields.Opcode & 0xFE) {
    case 0x50: // BN/BNB - Branch if negative
      TakeBranch = ((INT64)XValue < 0);
      break;

    case 0x52: // BZ/BZB - Branch if zero
      TakeBranch = (XValue == 0);
      break;

    case 0x54: // BP/BPB - Branch if positive
      TakeBranch = ((INT64)XValue > 0);
      break;

    case 0x56: // BOD/BODB - Branch if odd
      TakeBranch = ((XValue & 0x1) != 0);
      break;

    case 0x58: // BNN/BNNB - Branch if non-negative
      TakeBranch = ((INT64)XValue >= 0);
      break;

    case 0x5A: // BNZ/BNZB - Branch if non-zero
      TakeBranch = (XValue != 0);
      break;

    case 0x5C: // BNP/BNPB - Branch if non-positive
      TakeBranch = ((INT64)XValue <= 0);
      break;

    case 0x5E: // BEV/BEVB - Branch if even
      TakeBranch = ((XValue & 0x1) == 0);
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Take branch if condition met
  //
  if (TakeBranch) {
    CpuState->Pc = (UINT64)((INT64)CpuState->Pc + Offset - 4);  // -4 because PC already advanced
  }

  return MMIX_SUCCESS;
}

//
// Placeholder implementations for other instruction types
// These would be fully implemented in a complete emulator
//

STATIC
MMIX_STATUS
ExecuteFloatingPoint (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  //
  // Call FPU implementation
  //
  return MmixFpuExecute (CpuState, Inst);
}

STATIC
MMIX_STATUS
ExecuteVector (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  //
  // Vector operations (SVE-style) would be implemented here
  //
  return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
}

STATIC
MMIX_STATUS
ExecuteMatrix (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  //
  // Matrix operations (SME-style) would be implemented here
  //
  return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
}

STATIC
MMIX_STATUS
ExecuteMachineLearning (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  //
  // ML/AI operations would be implemented here
  //
  return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
}

STATIC
MMIX_STATUS
ExecuteHypervisor (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  //
  // Hypervisor operations would be implemented here
  //
  if (CpuState->PrivilegeLevel != MmixPrivilegeHypervisor) {
    return MmixCpuRaiseException (CpuState, MmixExceptionPrivileged, 0);
  }
  return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
}

STATIC
MMIX_STATUS
ExecuteCompressed (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  //
  // Call compressed instruction implementation
  //
  return MmixCompressedExecute (CpuState, CompInst);
}

/**
  Raise an exception.

  Saves current state, switches to supervisor mode, and transfers
  control to the exception handler. Updates rW, rX, rY, rZ as needed.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Exception     Exception type.
  @param[in]      FaultAddress  Faulting address (for memory exceptions).

  @retval MMIX_SUCCESS          Exception raised successfully.

**/
MMIX_STATUS
MmixCpuRaiseException (
  IN OUT MMIX_CPU_STATE      *CpuState,
  IN     MMIX_EXCEPTION_TYPE Exception,
  IN     UINT64              FaultAddress
  )
{
  UINT64  TrapHandler;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Save interrupted PC in rW
  //
  CpuState->SpecialRegisters[MMIX_rW] = CpuState->Pc;

  //
  // Save fault address in rF (for memory exceptions)
  //
  if (Exception == MmixExceptionPageFault ||
      Exception == MmixExceptionProtection ||
      Exception == MmixExceptionAlignment) {
    CpuState->SpecialRegisters[MMIX_rF] = FaultAddress;
  }

  //
  // Switch to supervisor mode
  //
  CpuState->PrivilegeLevel = MmixPrivilegeSupervisor;

  //
  // Get trap handler address from rT
  //
  TrapHandler = CpuState->SpecialRegisters[MMIX_rT];

  //
  // Add exception type offset
  //
  TrapHandler += (Exception * 16);  // 16 bytes per exception vector

  //
  // Jump to trap handler
  //
  CpuState->Pc = TrapHandler;

  return MMIX_SUCCESS;
}

/**
  Deliver an interrupt.

  Checks if interrupts are enabled and pending, then delivers
  the highest priority interrupt to the CPU.

  @param[in,out]  CpuState      Pointer to CPU state.

  @retval MMIX_SUCCESS          Interrupt delivered or no interrupt pending.

**/
MMIX_STATUS
MmixCpuDeliverInterrupt (
  IN OUT MMIX_CPU_STATE  *CpuState
  )
{
  UINT64  PendingEnabled;
  UINT8   InterruptNum;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Check if any interrupts are both pending and enabled
  //
  PendingEnabled = CpuState->InterruptsPending & CpuState->InterruptsEnabled;
  if (PendingEnabled == 0) {
    return MMIX_SUCCESS;
  }

  //
  // Find highest priority interrupt (lowest bit number)
  //
  for (InterruptNum = 0; InterruptNum < 64; InterruptNum++) {
    if ((PendingEnabled & (1ULL << InterruptNum)) != 0) {
      //
      // Clear pending bit
      //
      CpuState->InterruptsPending &= ~(1ULL << InterruptNum);

      //
      // Raise interrupt as exception
      //
      return MmixCpuRaiseException (
               CpuState,
               (MMIX_EXCEPTION_TYPE)(MmixExceptionTimer + InterruptNum),
               0
               );
    }
  }

  return MMIX_SUCCESS;
}
