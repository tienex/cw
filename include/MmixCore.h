/** @file
  MMIX Core CPU definitions and interfaces.

  This file defines the CPU state structure and core execution functions
  for the MMIX emulator. It includes all register files, execution state,
  and vector/matrix extension state.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_CORE_H__
#define __MMIX_CORE_H__

#include "MmixTypes.h"

//
// CPU execution modes
//

typedef enum {
  MmixExecutionModeNormal = 0,     ///< Normal execution
  MmixExecutionModeStepping = 1,   ///< Single-step mode
  MmixExecutionModeHalted = 2,     ///< CPU halted
  MmixExecutionModeWaitForInt = 3  ///< Waiting for interrupt
} MMIX_EXECUTION_MODE;

//
// Vector register state
//

/**
  Vector register structure.

  Supports scalable vector lengths from 128 to 2048 bits.
  Elements are stored in native machine byte order and converted
  as needed based on endianness settings.
**/
typedef struct {
  ///
  /// Vector data storage (maximum size)
  ///
  UINT8   Data[MMIX_MAX_VECTOR_LENGTH_BYTES];

  ///
  /// Current vector length in bytes (may be less than maximum)
  ///
  UINT32  LengthBytes;

  ///
  /// Element type for this vector
  ///
  MMIX_VECTOR_ELEMENT_TYPE  ElementType;
} MMIX_VECTOR_REGISTER;

//
// Predicate register state
//

/**
  Predicate register for masked vector operations.

  Each bit controls one vector element operation. The number of active
  bits depends on the current vector length and element size.
**/
typedef struct {
  ///
  /// Predicate bits (one per vector element)
  ///
  UINT8   Bits[MMIX_MAX_VECTOR_LENGTH_BYTES / 8];
} MMIX_PREDICATE_REGISTER;

//
// Matrix tile state
//

/**
  Matrix tile register for SME operations.

  Tiles can be configured for different sizes and element types.
  Operations are performed on entire tiles atomically.
**/
typedef struct {
  ///
  /// Tile data storage
  ///
  UINT8   Data[MMIX_MAX_MATRIX_TILE_SIZE * MMIX_MAX_MATRIX_TILE_SIZE / 8];

  ///
  /// Number of rows in tile
  ///
  UINT32  Rows;

  ///
  /// Number of columns in tile
  ///
  UINT32  Columns;

  ///
  /// Element type for this tile
  ///
  MMIX_VECTOR_ELEMENT_TYPE  ElementType;

  ///
  /// Tile is in use (streaming SVE mode)
  ///
  BOOLEAN Active;
} MMIX_MATRIX_TILE;

//
// Floating-point register state
//

/**
  Floating-point register (128-bit for quad precision).

  All floating-point formats are stored in these registers, with
  smaller formats occupying the lower bits.
**/
typedef union {
  UINT8   Bytes[16];      ///< Raw bytes
  UINT16  Words[8];       ///< 16-bit words
  UINT32  Dwords[4];      ///< 32-bit doublewords
  UINT64  Qwords[2];      ///< 64-bit quadwords
  float   Float32[4];     ///< 32-bit floats
  double  Float64[2];     ///< 64-bit doubles
} MMIX_FP_REGISTER;

//
// CPU state structure
//

/**
  Complete MMIX CPU state.

  This structure contains all architectural state for a single
  MMIX processor core, including general registers, special registers,
  vector and matrix extensions, and execution state.
**/
struct _MMIX_CPU_STATE {
  //
  // General-purpose registers ($0-$255)
  //
  UINT64  GeneralRegisters[MMIX_GENERAL_REGISTER_COUNT];

  //
  // Special registers (rA-rZZ)
  //
  UINT64  SpecialRegisters[64];  // Includes both standard and extended

  //
  // Floating-point registers (F0-F255)
  //
  MMIX_FP_REGISTER  FpRegisters[MMIX_FLOATING_REGISTER_COUNT];

  //
  // Vector registers (V0-V255) for SVE
  //
  MMIX_VECTOR_REGISTER  VectorRegisters[MMIX_VECTOR_REGISTER_COUNT];

  //
  // Predicate registers (P0-P63) for SVE
  //
  MMIX_PREDICATE_REGISTER  PredicateRegisters[MMIX_PREDICATE_REGISTER_COUNT];

  //
  // Matrix tile registers (ZA0-ZA7) for SME
  //
  MMIX_MATRIX_TILE  MatrixTiles[MMIX_MATRIX_TILE_COUNT];

  //
  // Program Counter
  //
  UINT64  Pc;

  //
  // Current privilege level (ring)
  //
  MMIX_PRIVILEGE_LEVEL  PrivilegeLevel;

  //
  // Execution mode
  //
  MMIX_EXECUTION_MODE  ExecutionMode;

  //
  // Per-ring configuration (KESU 4-ring protection)
  //
  MMIX_RING_CONFIG  RingConfig[MMIX_RING_COUNT];

  //
  // KESU extension enabled (if FALSE, uses legacy 2-ring K/U model)
  //
  BOOLEAN  KesuExtensionEnabled;

  //
  // FPR aliased to GPR mode (compatibility mode)
  // When TRUE: F0-F255 are aliased to $0-$255 (no separate FP registers)
  // When FALSE: F0-F255 are independent 128-bit FP registers
  //
  BOOLEAN  FprAliasedToGpr;

  //
  // MIX compatibility mode (for running legacy MIX programs)
  // When TRUE: Emulates Donald Knuth's original MIX computer
  // When FALSE: Native MMIX mode
  //
  BOOLEAN  MixCompatibilityMode;

  //
  // Floating-point rounding mode
  //
  MMIX_ROUNDING_MODE  RoundingMode;

  //
  // Vector length in bytes (for SVE operations)
  //
  UINT32  VectorLengthBytes;

  //
  // Global register threshold (register $rG)
  //
  UINT8   GlobalThreshold;

  //
  // Local register threshold (register $rL)
  //
  UINT8   LocalThreshold;

  //
  // Interrupts pending (bitmask)
  //
  UINT64  InterruptsPending;

  //
  // Interrupts enabled (bitmask)
  //
  UINT64  InterruptsEnabled;

  //
  // Guest mode active (hypervisor extension)
  //
  BOOLEAN  GuestMode;

  //
  // Instruction count (for performance monitoring)
  //
  UINT64  InstructionCount;

  //
  // Cycle count
  //
  UINT64  CycleCount;

  //
  // Pointer to memory state
  //
  MMIX_MEMORY_STATE  *MemoryState;

  //
  // Pointer to emulator context
  //
  MMIX_EMULATOR_CONTEXT  *EmulatorContext;
};

//
// Core CPU functions
//

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Destroy CPU state.

  Frees all resources associated with the CPU state.

  @param[in]  CpuState          Pointer to CPU state to destroy.

**/
VOID
MmixCpuDestroy (
  IN MMIX_CPU_STATE  *CpuState
  );

//
// Floating-point register access functions
//

/**
  Read a floating-point register.

  Reads from the FP register file. If FprAliasedToGpr mode is enabled,
  reads from the corresponding GPR instead.

  @param[in]      CpuState      Pointer to CPU state.
  @param[in]      RegNum        FP register number (0-255).
  @param[out]     Value         Pointer to receive register value (64-bit).

  @retval MMIX_SUCCESS          Register read successfully.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid register number.

**/
MMIX_STATUS
MmixCpuReadFpRegister (
  IN  MMIX_CPU_STATE  *CpuState,
  IN  UINT8           RegNum,
  OUT UINT64          *Value
  );

/**
  Write a floating-point register.

  Writes to the FP register file. If FprAliasedToGpr mode is enabled,
  writes to the corresponding GPR instead.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      RegNum        FP register number (0-255).
  @param[in]      Value         Value to write (64-bit).

  @retval MMIX_SUCCESS          Register written successfully.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid register number.

**/
MMIX_STATUS
MmixCpuWriteFpRegister (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           RegNum,
  IN     UINT64          Value
  );

//
// KESU 4-ring protection functions
//

/**
  Transition to a different protection ring.

  Performs a ring transition with stack switching and context saving.
  Validates the transition is allowed (can only move to more privileged
  rings via call gates, and to less privileged rings via return).

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      NewRing       Target ring (0-3).
  @param[in]      IsCall        TRUE if ring transition via call, FALSE if return.

  @retval MMIX_SUCCESS          Ring transition completed successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Transition not allowed.

**/
MMIX_STATUS
MmixCpuTransitionRing (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           NewRing,
  IN     BOOLEAN         IsCall
  );

/**
  Get the current endianness mode.

  Returns the endianness mode for the current privilege ring.
  If KESU extension is disabled, returns big-endian (MMIX default).

  @param[in]      CpuState      Pointer to CPU state.

  @return  Current endianness mode.

**/
MMIX_ENDIAN_MODE
MmixCpuGetEndianness (
  IN MMIX_CPU_STATE  *CpuState
  );

/**
  Get the current stack direction.

  Returns the stack growth direction for the current privilege ring.

  @param[in]      CpuState      Pointer to CPU state.

  @return  Current stack direction.

**/
MMIX_STACK_DIRECTION
MmixCpuGetStackDirection (
  IN MMIX_CPU_STATE  *CpuState
  );

/**
  Get the page table base for the current ring.

  Returns the page table base physical address for the current
  privilege level. Used by memory management for address translation.

  @param[in]      CpuState      Pointer to CPU state.

  @return  Page table base physical address.

**/
UINT64
MmixCpuGetPageTableBase (
  IN MMIX_CPU_STATE  *CpuState
  );

/**
  Configure a protection ring.

  Sets the endianness, stack direction, and page table base for
  a specific protection ring. Requires kernel privilege.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Ring          Ring number (0-3).
  @param[in]      Endianness    Endianness mode for this ring.
  @param[in]      StackDir      Stack growth direction.
  @param[in]      PageTableBase Page table base physical address.

  @retval MMIX_SUCCESS          Ring configured successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Insufficient privilege.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid ring number.

**/
MMIX_STATUS
MmixCpuConfigureRing (
  IN OUT MMIX_CPU_STATE       *CpuState,
  IN     UINT8                Ring,
  IN     MMIX_ENDIAN_MODE     Endianness,
  IN     MMIX_STACK_DIRECTION StackDir,
  IN     UINT64               PageTableBase
  );

#endif // __MMIX_CORE_H__
