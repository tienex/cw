/** @file
  MIX Compatibility Layer for MMIX Emulator.

  This file defines the compatibility layer for running Donald Knuth's
  original MIX programs on MMIX. MIX is the hypothetical computer
  described in "The Art of Computer Programming" volumes 1-3.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_MIX_H__
#define __MMIX_MIX_H__

#include "MmixTypes.h"
#include "MmixCore.h"

//
// MIX Architecture Overview
//
// MIX is a word-addressable computer with:
// - 5-byte words (30 bits + sign)
// - 4000 words of memory
// - Registers: A, X, I1-I6, J
// - Decimal and binary arithmetic
//

//
// MIX Register Mapping to MMIX
//
// MIX registers are mapped to MMIX general-purpose registers:
//
// $1  = A  (Accumulator, 5 bytes)
// $2  = X  (Extension, 5 bytes)
// $3  = I1 (Index register 1, 2 bytes)
// $4  = I2 (Index register 2, 2 bytes)
// $5  = I3 (Index register 3, 2 bytes)
// $6  = I4 (Index register 4, 2 bytes)
// $7  = I5 (Index register 5, 2 bytes)
// $8  = I6 (Index register 6, 2 bytes)
// $9  = J  (Jump address, 2 bytes)
//
// $10 = Comparison indicator (LESS, EQUAL, GREATER)
// $11 = Overflow toggle
//

#define MIX_REG_A   1   ///< Accumulator register (5 bytes)
#define MIX_REG_X   2   ///< Extension register (5 bytes)
#define MIX_REG_I1  3   ///< Index register 1 (2 bytes)
#define MIX_REG_I2  4   ///< Index register 2 (2 bytes)
#define MIX_REG_I3  5   ///< Index register 3 (2 bytes)
#define MIX_REG_I4  6   ///< Index register 4 (2 bytes)
#define MIX_REG_I5  7   ///< Index register 5 (2 bytes)
#define MIX_REG_I6  8   ///< Index register 6 (2 bytes)
#define MIX_REG_J   9   ///< Jump address register (2 bytes)
#define MIX_REG_CMP 10  ///< Comparison indicator
#define MIX_REG_OVR 11  ///< Overflow toggle

//
// MIX Memory Configuration
//

#define MIX_MEMORY_SIZE  4000  ///< 4000 words of memory
#define MIX_WORD_SIZE    5     ///< 5 bytes per word (+ sign)

//
// MIX Comparison Indicator Values
//

typedef enum {
  MixCompLess    = -1,  ///< Less than
  MixCompEqual   = 0,   ///< Equal
  MixCompGreater = 1    ///< Greater than
} MIX_COMPARISON;

//
// MIX Word Structure
//
// MIX words are 5 bytes plus a sign bit:
// [±] [Byte 1] [Byte 2] [Byte 3] [Byte 4] [Byte 5]
//
// Each byte is 6 bits (0-63), for a total of 30 bits of magnitude.
//

typedef struct {
  BOOLEAN  Sign;      ///< Sign bit (FALSE = +, TRUE = -)
  UINT8    Bytes[5];  ///< 5 bytes, each 6 bits (0-63)
} MIX_WORD;

//
// MIX Field Specification (F-field)
//
// Format: (L:R) where L is left, R is right
// Specifies which bytes to use in an operation
//

typedef struct {
  UINT8  Left;   ///< Left byte (0-5)
  UINT8  Right;  ///< Right byte (0-5)
} MIX_FIELD;

//
// Function Prototypes
//

/**
  Enable MIX compatibility mode.

  Configures the CPU to emulate MIX behavior. This sets up
  register mappings and memory layout for MIX programs.

  @param[in,out]  CpuState    Pointer to CPU state.

  @retval MMIX_SUCCESS         MIX mode enabled successfully.
  @retval MMIX_ERROR_*         Error occurred.
**/
MMIX_STATUS
MmixEnableMixMode (
  IN OUT MMIX_CPU_STATE  *CpuState
  );

/**
  Disable MIX compatibility mode.

  Returns the CPU to native MMIX operation.

  @param[in,out]  CpuState    Pointer to CPU state.

  @retval MMIX_SUCCESS         MIX mode disabled successfully.
**/
MMIX_STATUS
MmixDisableMixMode (
  IN OUT MMIX_CPU_STATE  *CpuState
  );

/**
  Convert MIX word to MMIX 64-bit value.

  Converts a MIX 5-byte word (30 bits + sign) to MMIX format.

  @param[in]   MixWord    MIX word to convert.

  @return  64-bit MMIX representation.
**/
UINT64
MmixMixWordToUint64 (
  IN CONST MIX_WORD  *MixWord
  );

/**
  Convert MMIX 64-bit value to MIX word.

  Converts an MMIX 64-bit value to MIX 5-byte word format.

  @param[in]   Value      64-bit value to convert.
  @param[out]  MixWord    Resulting MIX word.
**/
VOID
MmixUint64ToMixWord (
  IN  UINT64    Value,
  OUT MIX_WORD  *MixWord
  );

/**
  Read MIX register.

  Reads a MIX register (A, X, I1-I6, J) from the CPU state.

  @param[in]   CpuState   Pointer to CPU state.
  @param[in]   Register   MIX register number (MIX_REG_*).
  @param[out]  Value      Register value.

  @retval MMIX_SUCCESS         Register read successfully.
  @retval MMIX_ERROR_*         Error occurred.
**/
MMIX_STATUS
MmixReadMixRegister (
  IN  MMIX_CPU_STATE  *CpuState,
  IN  UINT8           Register,
  OUT UINT64          *Value
  );

/**
  Write MIX register.

  Writes to a MIX register (A, X, I1-I6, J) in the CPU state.

  @param[in,out]  CpuState   Pointer to CPU state.
  @param[in]      Register   MIX register number (MIX_REG_*).
  @param[in]      Value      Value to write.

  @retval MMIX_SUCCESS         Register written successfully.
  @retval MMIX_ERROR_*         Error occurred.
**/
MMIX_STATUS
MmixWriteMixRegister (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           Register,
  IN     UINT64          Value
  );

#endif // __MMIX_MIX_H__
