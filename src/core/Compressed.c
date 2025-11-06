/** @file
  MMIX Compressed instruction implementation.

  This file implements 16-bit compressed instruction execution for
  improved code density. Compressed instructions have the same semantics
  as their 32-bit counterparts but with encoding restrictions.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include "../../include/MmixCore.h"
#include "../../include/MmixMemory.h"

//
// Compressed instruction opcodes
//

#define C_ADD_OPCODE    0x8
#define C_SUB_OPCODE    0x8
#define C_MUL_OPCODE    0x8
#define C_AND_OPCODE    0x8
#define C_OR_OPCODE     0x8
#define C_XOR_OPCODE    0x8
#define C_ADDI_OPCODE   0x9
#define C_SUBI_OPCODE   0x9
#define C_ANDI_OPCODE   0x9
#define C_ORI_OPCODE    0x9
#define C_XORI_OPCODE   0x9
#define C_LI_OPCODE     0x9
#define C_LDO_OPCODE    0xA
#define C_LDT_OPCODE    0xA
#define C_LDW_OPCODE    0xA
#define C_LDB_OPCODE    0xA
#define C_STO_OPCODE    0xB
#define C_STT_OPCODE    0xB
#define C_STW_OPCODE    0xB
#define C_STB_OPCODE    0xB
#define C_J_OPCODE      0xC
#define C_JR_OPCODE     0xC
#define C_BR_OPCODE     0xC
#define C_MV_OPCODE     0xC

/**
  Execute compressed register-register instruction.

  Format: opcode[4] | func[4] | rs1[4] | rs2[4]

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      CompInst      Compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteCompressedRegister (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  UINT8   Opcode, Func, Rs1, Rs2;
  UINT64  Rs1Value, Rs2Value, Result;

  //
  // Decode fields
  //
  Opcode = (CompInst >> 12) & 0xF;
  Func = CompInst & 0xF;
  Rs1 = (CompInst >> 4) & 0xF;
  Rs2 = (CompInst >> 8) & 0xF;

  //
  // Read source registers
  //
  MmixCpuReadRegister (CpuState, Rs1, &Rs1Value);
  MmixCpuReadRegister (CpuState, Rs2, &Rs2Value);

  //
  // Execute based on function code
  //
  switch (Func) {
    case 0x0: // C.ADD
      Result = Rs1Value + Rs2Value;
      break;

    case 0x1: // C.SUB
      Result = Rs1Value - Rs2Value;
      break;

    case 0x2: // C.MUL
      Result = Rs1Value * Rs2Value;
      break;

    case 0x3: // C.AND
      Result = Rs1Value & Rs2Value;
      break;

    case 0x4: // C.OR
      Result = Rs1Value | Rs2Value;
      break;

    case 0x5: // C.XOR
      Result = Rs1Value ^ Rs2Value;
      break;

    case 0xF: // C.MV (move)
      Result = Rs2Value;
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Write result to Rs1 (destination)
  //
  MmixCpuWriteRegister (CpuState, Rs1, Result);

  return MMIX_SUCCESS;
}

/**
  Execute compressed immediate instruction.

  Format: opcode[4] | imm_high[4] | rs1/rd[4] | imm_low[4]

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      CompInst      Compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteCompressedImmediate (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  UINT8   Opcode, Rd, ImmHigh, ImmLow;
  INT32   Immediate;
  UINT64  RdValue, Result;

  //
  // Decode fields
  //
  Opcode = (CompInst >> 12) & 0xF;
  ImmLow = CompInst & 0xF;
  Rd = (CompInst >> 4) & 0xF;
  ImmHigh = (CompInst >> 8) & 0xF;

  //
  // Combine immediate (sign-extend 8-bit immediate)
  //
  Immediate = (INT32)((ImmHigh << 4) | ImmLow);
  if (Immediate & 0x80) {
    Immediate |= 0xFFFFFF00;  // Sign extend
  }

  //
  // Read destination register (also source for most operations)
  //
  MmixCpuReadRegister (CpuState, Rd, &RdValue);

  //
  // Determine operation from immediate high bits
  //
  switch (ImmHigh) {
    case 0x0: // C.ADDI
      Result = RdValue + (INT64)Immediate;
      break;

    case 0x1: // C.SUBI
      Result = RdValue - (INT64)Immediate;
      break;

    case 0x2: // C.ANDI
      Result = RdValue & (UINT64)(INT64)Immediate;
      break;

    case 0x3: // C.ORI
      Result = RdValue | (UINT64)(INT64)Immediate;
      break;

    case 0x4: // C.XORI
      Result = RdValue ^ (UINT64)(INT64)Immediate;
      break;

    case 0x5: // C.LI (load immediate)
      Result = (UINT64)(INT64)Immediate;
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Write result
  //
  MmixCpuWriteRegister (CpuState, Rd, Result);

  return MMIX_SUCCESS;
}

/**
  Execute compressed load instruction.

  Format: opcode[4] | offset[4] | rs1[4] | func[4]

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      CompInst      Compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteCompressedLoad (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  MMIX_STATUS  Status;
  UINT8        Func, Rs1, Offset;
  UINT64       Rs1Value, Address, LoadedValue;
  UINT8        Size;

  //
  // Decode fields
  //
  Func = CompInst & 0xF;
  Rs1 = (CompInst >> 4) & 0xF;
  Offset = (CompInst >> 8) & 0xF;

  //
  // Read base register
  //
  MmixCpuReadRegister (CpuState, Rs1, &Rs1Value);

  //
  // Determine size based on function
  //
  switch (Func) {
    case 0x0: // C.LDO - Load octa
      Size = 8;
      Offset *= 8;  // Scale by 8
      break;

    case 0x1: // C.LDT - Load tetra
      Size = 4;
      Offset *= 4;  // Scale by 4
      break;

    case 0x2: // C.LDW - Load wyde
      Size = 2;
      Offset *= 2;  // Scale by 2
      break;

    case 0x3: // C.LDB - Load byte
      Size = 1;
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Calculate effective address
  //
  Address = Rs1Value + Offset;

  //
  // Load from memory
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
  // Write to destination (Rs1 is reused as destination)
  //
  MmixCpuWriteRegister (CpuState, Rs1, LoadedValue);

  return MMIX_SUCCESS;
}

/**
  Execute compressed store instruction.

  Format: opcode[4] | offset[4] | rs1[4] | func[4]

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      CompInst      Compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteCompressedStore (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  MMIX_STATUS  Status;
  UINT8        Func, Rs1, Offset;
  UINT64       Rs1Value, Address, StoreValue;
  UINT8        Size;

  //
  // Decode fields
  //
  Func = CompInst & 0xF;
  Rs1 = (CompInst >> 4) & 0xF;
  Offset = (CompInst >> 8) & 0xF;

  //
  // Read source register
  //
  MmixCpuReadRegister (CpuState, Rs1, &Rs1Value);

  //
  // Determine size based on function
  //
  switch (Func) {
    case 0x0: // C.STO - Store octa
      Size = 8;
      Offset *= 8;
      break;

    case 0x1: // C.STT - Store tetra
      Size = 4;
      Offset *= 4;
      break;

    case 0x2: // C.STW - Store wyde
      Size = 2;
      Offset *= 2;
      break;

    case 0x3: // C.STB - Store byte
      Size = 1;
      break;

    default:
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }

  //
  // Calculate effective address (using Rs1 as base and destination)
  //
  Address = Rs1Value + Offset;

  //
  // Prepare store value (from register after Rs1)
  //
  MmixCpuReadRegister (CpuState, Rs1 + 1, &StoreValue);

  //
  // Handle endianness
  //
  if (CpuState->EndiannessMode == MmixEndianLittle) {
    StoreValue = MmixHostToLittleEndian (StoreValue, Size);
  } else {
    StoreValue = MmixHostToBigEndian (StoreValue, Size);
  }

  //
  // Store to memory
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
  Execute compressed jump/branch instruction.

  Format: opcode[4] | offset[12]

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      CompInst      Compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
STATIC
MMIX_STATUS
ExecuteCompressedJump (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  INT32   Offset;

  //
  // Extract 12-bit offset and sign-extend
  //
  Offset = CompInst & 0xFFF;
  if (Offset & 0x800) {
    Offset |= 0xFFFFF000;  // Sign extend
  }

  //
  // Scale by 2 for byte offset (compressed instructions are 2-byte aligned)
  //
  Offset *= 2;

  //
  // Update PC (subtract 2 because PC was already advanced by 2)
  //
  CpuState->Pc = (UINT64)((INT64)CpuState->Pc + Offset - 2);

  return MMIX_SUCCESS;
}

/**
  Execute compressed instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      CompInst      Compressed instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
MMIX_STATUS
MmixCompressedExecute (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT16          CompInst
  )
{
  UINT8  Opcode;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Extract top 4 bits as opcode
  //
  Opcode = (CompInst >> 12) & 0xF;

  //
  // Dispatch based on opcode
  //
  switch (Opcode) {
    case 0x8: // Register-register operations
      return ExecuteCompressedRegister (CpuState, CompInst);

    case 0x9: // Immediate operations
      return ExecuteCompressedImmediate (CpuState, CompInst);

    case 0xA: // Load operations
      return ExecuteCompressedLoad (CpuState, CompInst);

    case 0xB: // Store operations
      return ExecuteCompressedStore (CpuState, CompInst);

    case 0xC: // Jump/branch operations
      return ExecuteCompressedJump (CpuState, CompInst);

    default:
      //
      // Invalid compressed instruction
      //
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }
}
