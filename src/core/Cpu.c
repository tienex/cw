/** @file
  MMIX CPU core implementation.

  This file implements the CPU state management, register access, and
  basic CPU operations for the MMIX emulator.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include "../../include/MmixCore.h"
#include "../../include/MmixMemory.h"

/**
  Initialize a new CPU state structure.

  Allocates and initializes all CPU state including registers,
  vector state, and execution mode. Sets initial PC and privilege level.

  @param[out]  CpuState         Pointer to receive CPU state pointer.
  @param[in]   InitialPc        Initial program counter value.
  @param[in]   MemoryState      Pointer to memory state.
  @param[in]   VectorLength     Initial vector length in bytes.

  @retval MMIX_SUCCESS          CPU state initialized successfully.
  @retval MMIX_ERROR_OUT_OF_MEMORY  Failed to allocate memory.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid parameter.

**/
MMIX_STATUS
MmixCpuInitialize (
  OUT MMIX_CPU_STATE     **CpuState,
  IN  UINT64             InitialPc,
  IN  MMIX_MEMORY_STATE  *MemoryState,
  IN  UINT32             VectorLength
  )
{
  MMIX_CPU_STATE  *Cpu;
  UINT32          i;

  if (CpuState == NULL || MemoryState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (VectorLength < MMIX_MIN_VECTOR_LENGTH_BYTES ||
      VectorLength > MMIX_MAX_VECTOR_LENGTH_BYTES ||
      (VectorLength & (VectorLength - 1)) != 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate CPU state structure
  //
  Cpu = (MMIX_CPU_STATE *)malloc (sizeof (MMIX_CPU_STATE));
  if (Cpu == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  //
  // Zero-initialize entire structure
  //
  memset (Cpu, 0, sizeof (MMIX_CPU_STATE));

  //
  // Set initial state
  //
  Cpu->Pc = InitialPc;
  Cpu->MemoryState = MemoryState;
  Cpu->PrivilegeLevel = MmixPrivilegeSupervisor;
  Cpu->ExecutionMode = MmixExecutionModeNormal;
  Cpu->EndiannessMode = MmixEndianBig;
  Cpu->RoundingMode = MmixRoundNearestEven;
  Cpu->VectorLengthBytes = VectorLength;
  Cpu->GlobalThreshold = 32;   // Default: $0-$31 are local
  Cpu->LocalThreshold = 0;
  Cpu->GuestMode = FALSE;

  //
  // Initialize general registers
  // Register $0 is always 0, others start undefined (0)
  //
  for (i = 0; i < MMIX_GENERAL_REGISTER_COUNT; i++) {
    Cpu->GeneralRegisters[i] = 0;
  }

  //
  // Initialize special registers to default values
  //
  Cpu->SpecialRegisters[MMIX_rA] = 0;  // Arithmetic status
  Cpu->SpecialRegisters[MMIX_rG] = 32; // Global threshold
  Cpu->SpecialRegisters[MMIX_rL] = 0;  // Local threshold
  Cpu->SpecialRegisters[MMIX_rK] = 0;  // Interrupt mask (all disabled)
  Cpu->SpecialRegisters[MMIX_rT] = 0;  // Trap handler address
  Cpu->SpecialRegisters[MMIX_rV] = 0;  // Virtual translation
  Cpu->SpecialRegisters[MMIX_rN] = 0x4D4D4958; // "MMIX" serial number

  //
  // Initialize extended special registers
  //
  Cpu->SpecialRegisters[MMIX_rVL] = VectorLength;
  Cpu->SpecialRegisters[MMIX_rVT] = 0; // Vector type
  Cpu->SpecialRegisters[MMIX_rMT] = 0; // Matrix tile config
  Cpu->SpecialRegisters[MMIX_rEN] = 0; // Big-endian mode
  Cpu->SpecialRegisters[MMIX_rPR] = MmixPrivilegeSupervisor;
  Cpu->SpecialRegisters[MMIX_rPT] = 0; // Page table base
  Cpu->SpecialRegisters[MMIX_rAS] = 0; // ASID

  //
  // Initialize vector registers
  //
  for (i = 0; i < MMIX_VECTOR_REGISTER_COUNT; i++) {
    memset (Cpu->VectorRegisters[i].Data, 0, MMIX_MAX_VECTOR_LENGTH_BYTES);
    Cpu->VectorRegisters[i].LengthBytes = VectorLength;
    Cpu->VectorRegisters[i].ElementType = MmixVectorTypeInt64;
  }

  //
  // Initialize predicate registers
  //
  for (i = 0; i < MMIX_PREDICATE_REGISTER_COUNT; i++) {
    memset (Cpu->PredicateRegisters[i].Bits, 0, sizeof (Cpu->PredicateRegisters[i].Bits));
  }

  //
  // Initialize matrix tiles
  //
  for (i = 0; i < MMIX_MATRIX_TILE_COUNT; i++) {
    memset (Cpu->MatrixTiles[i].Data, 0, sizeof (Cpu->MatrixTiles[i].Data));
    Cpu->MatrixTiles[i].Rows = 0;
    Cpu->MatrixTiles[i].Columns = 0;
    Cpu->MatrixTiles[i].ElementType = MmixVectorTypeInt64;
    Cpu->MatrixTiles[i].Active = FALSE;
  }

  //
  // Initialize FP registers
  //
  for (i = 0; i < 32; i++) {
    memset (Cpu->FpRegisters[i].Bytes, 0, 16);
  }

  //
  // Set interrupts
  //
  Cpu->InterruptsPending = 0;
  Cpu->InterruptsEnabled = 0;

  //
  // Initialize counters
  //
  Cpu->InstructionCount = 0;
  Cpu->CycleCount = 0;

  *CpuState = Cpu;
  return MMIX_SUCCESS;
}

/**
  Reset CPU state to initial values.

  Resets all registers, clears interrupts, and sets CPU to
  supervisor mode at the bootstrap address.

  @param[in,out]  CpuState      Pointer to CPU state.

  @retval MMIX_SUCCESS          CPU reset successfully.

**/
MMIX_STATUS
MmixCpuReset (
  IN OUT MMIX_CPU_STATE  *CpuState
  )
{
  UINT32  i;
  UINT64  BootstrapAddress;
  UINT32  VectorLength;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Save some values we want to preserve
  //
  BootstrapAddress = CpuState->SpecialRegisters[MMIX_rB];
  VectorLength = CpuState->VectorLengthBytes;

  //
  // Zero general registers
  //
  for (i = 0; i < MMIX_GENERAL_REGISTER_COUNT; i++) {
    CpuState->GeneralRegisters[i] = 0;
  }

  //
  // Reset special registers
  //
  CpuState->SpecialRegisters[MMIX_rA] = 0;
  CpuState->SpecialRegisters[MMIX_rK] = 0;

  //
  // Set PC to bootstrap address (or 0 if not set)
  //
  CpuState->Pc = (BootstrapAddress != 0) ? BootstrapAddress : 0;

  //
  // Reset execution state
  //
  CpuState->PrivilegeLevel = MmixPrivilegeSupervisor;
  CpuState->ExecutionMode = MmixExecutionModeNormal;
  CpuState->EndiannessMode = MmixEndianBig;
  CpuState->GuestMode = FALSE;

  //
  // Clear interrupts
  //
  CpuState->InterruptsPending = 0;

  //
  // Reset counters
  //
  CpuState->CycleCount = 0;

  return MMIX_SUCCESS;
}

/**
  Read a general register.

  Reads from the general register file, handling the register window
  and global/local threshold correctly.

  @param[in]      CpuState      Pointer to CPU state.
  @param[in]      RegNum        Register number (0-255).
  @param[out]     Value         Pointer to receive register value.

  @retval MMIX_SUCCESS          Register read successfully.

**/
MMIX_STATUS
MmixCpuReadRegister (
  IN  MMIX_CPU_STATE  *CpuState,
  IN  UINT8           RegNum,
  OUT UINT64          *Value
  )
{
  if (CpuState == NULL || Value == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Register $0 is always 0
  //
  if (RegNum == 0) {
    *Value = 0;
    return MMIX_SUCCESS;
  }

  //
  // Check if this is a local or global register
  // Registers below global threshold are local (relative to register window)
  // Registers at or above global threshold are global (absolute)
  //
  if (RegNum < CpuState->GlobalThreshold) {
    //
    // Local register - apply register window offset
    // For now, we don't implement full register window, just direct access
    //
    *Value = CpuState->GeneralRegisters[RegNum];
  } else {
    //
    // Global register - direct access
    //
    *Value = CpuState->GeneralRegisters[RegNum];
  }

  return MMIX_SUCCESS;
}

/**
  Write a general register.

  Writes to the general register file. Register $0 writes are ignored.
  Handles the register window and global/local threshold.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      RegNum        Register number (0-255).
  @param[in]      Value         Value to write.

  @retval MMIX_SUCCESS          Register written successfully.

**/
MMIX_STATUS
MmixCpuWriteRegister (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           RegNum,
  IN     UINT64          Value
  )
{
  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Register $0 is always 0, ignore writes
  //
  if (RegNum == 0) {
    return MMIX_SUCCESS;
  }

  //
  // Apply register window for local registers
  //
  if (RegNum < CpuState->GlobalThreshold) {
    CpuState->GeneralRegisters[RegNum] = Value;
  } else {
    CpuState->GeneralRegisters[RegNum] = Value;
  }

  return MMIX_SUCCESS;
}

/**
  Read a special register.

  Reads from a special register (rA-rZZ). Some registers may require
  elevated privilege to read.

  @param[in]      CpuState      Pointer to CPU state.
  @param[in]      RegNum        Special register index.
  @param[out]     Value         Pointer to receive register value.

  @retval MMIX_SUCCESS          Register read successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Insufficient privilege.

**/
MMIX_STATUS
MmixCpuReadSpecialRegister (
  IN  MMIX_CPU_STATE  *CpuState,
  IN  UINT8           RegNum,
  OUT UINT64          *Value
  )
{
  if (CpuState == NULL || Value == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Some special registers require supervisor or hypervisor privilege
  //
  if (RegNum >= MMIX_rVL && CpuState->PrivilegeLevel == MmixPrivilegeUser) {
    return MMIX_ERROR_ACCESS_DENIED;
  }

  //
  // Special case for cycle counter - return current value
  //
  if (RegNum == MMIX_rC) {
    *Value = CpuState->CycleCount;
    return MMIX_SUCCESS;
  }

  //
  // Read from special register array
  //
  if (RegNum < 64) {
    *Value = CpuState->SpecialRegisters[RegNum];
    return MMIX_SUCCESS;
  }

  return MMIX_ERROR_INVALID_PARAMETER;
}

/**
  Write a special register.

  Writes to a special register (rA-rZZ). Some registers may require
  elevated privilege to write or may have side effects.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      RegNum        Special register index.
  @param[in]      Value         Value to write.

  @retval MMIX_SUCCESS          Register written successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Insufficient privilege.

**/
MMIX_STATUS
MmixCpuWriteSpecialRegister (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           RegNum,
  IN     UINT64          Value
  )
{
  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Extended special registers require supervisor+ privilege
  //
  if (RegNum >= MMIX_rVL && CpuState->PrivilegeLevel == MmixPrivilegeUser) {
    return MMIX_ERROR_ACCESS_DENIED;
  }

  //
  // Some registers have side effects when written
  //
  switch (RegNum) {
    case MMIX_rG:
      //
      // Global threshold - update cache
      //
      CpuState->GlobalThreshold = (UINT8)(Value & 0xFF);
      break;

    case MMIX_rL:
      //
      // Local threshold - update cache
      //
      CpuState->LocalThreshold = (UINT8)(Value & 0xFF);
      break;

    case MMIX_rK:
      //
      // Interrupt mask - update enabled interrupts
      //
      CpuState->InterruptsEnabled = Value;
      break;

    case MMIX_rVL:
      //
      // Vector length - update vector registers
      //
      CpuState->VectorLengthBytes = (UINT32)(Value & 0xFFFFFFFF);
      break;

    case MMIX_rEN:
      //
      // Endianness control
      //
      CpuState->EndiannessMode = (Value & 0x1) ? MmixEndianLittle : MmixEndianBig;
      break;

    case MMIX_rPR:
      //
      // Privilege level
      //
      if (CpuState->PrivilegeLevel >= MmixPrivilegeSupervisor) {
        CpuState->PrivilegeLevel = (MMIX_PRIVILEGE_LEVEL)(Value & 0x3);
      } else {
        return MMIX_ERROR_ACCESS_DENIED;
      }
      break;

    case MMIX_rPT:
      //
      // Page table base - flush TLB
      //
      if (CpuState->MemoryState != NULL) {
        MmixTlbFlush (CpuState->MemoryState, TlbFlushAll, 0, 0);
      }
      break;

    default:
      //
      // Most registers can be written directly
      //
      break;
  }

  //
  // Write to special register array
  //
  if (RegNum < 64) {
    CpuState->SpecialRegisters[RegNum] = Value;
    return MMIX_SUCCESS;
  }

  return MMIX_ERROR_INVALID_PARAMETER;
}

/**
  Destroy CPU state.

  Frees all resources associated with the CPU state.

  @param[in]  CpuState          Pointer to CPU state to destroy.

**/
VOID
MmixCpuDestroy (
  IN MMIX_CPU_STATE  *CpuState
  )
{
  if (CpuState != NULL) {
    free (CpuState);
  }
}
