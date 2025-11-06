/** @file
  MIX Compatibility Layer Implementation.

  This file implements the MIX compatibility layer for running
  Donald Knuth's original MIX programs on MMIX.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "MmixMix.h"
#include <string.h>

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
  )
{
  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Enable MIX compatibility mode
  //
  CpuState->MixCompatibilityMode = TRUE;

  //
  // Initialize MIX registers to zero
  // A, X, I1-I6, J, comparison indicator, overflow
  //
  CpuState->GeneralRegisters[MIX_REG_A]   = 0;  // Accumulator
  CpuState->GeneralRegisters[MIX_REG_X]   = 0;  // Extension
  CpuState->GeneralRegisters[MIX_REG_I1]  = 0;  // Index 1
  CpuState->GeneralRegisters[MIX_REG_I2]  = 0;  // Index 2
  CpuState->GeneralRegisters[MIX_REG_I3]  = 0;  // Index 3
  CpuState->GeneralRegisters[MIX_REG_I4]  = 0;  // Index 4
  CpuState->GeneralRegisters[MIX_REG_I5]  = 0;  // Index 5
  CpuState->GeneralRegisters[MIX_REG_I6]  = 0;  // Index 6
  CpuState->GeneralRegisters[MIX_REG_J]   = 0;  // Jump address
  CpuState->GeneralRegisters[MIX_REG_CMP] = MixCompEqual;  // Comparison indicator
  CpuState->GeneralRegisters[MIX_REG_OVR] = 0;  // Overflow toggle

  //
  // MIX uses word-addressable memory (4000 words)
  // Map this to MMIX byte-addressable memory starting at address 0
  // Each MIX word occupies 8 bytes in MMIX memory (for alignment)
  //

  return MMIX_SUCCESS;
}

/**
  Disable MIX compatibility mode.

  Returns the CPU to native MMIX operation.

  @param[in,out]  CpuState    Pointer to CPU state.

  @retval MMIX_SUCCESS         MIX mode disabled successfully.
**/
MMIX_STATUS
MmixDisableMixMode (
  IN OUT MMIX_CPU_STATE  *CpuState
  )
{
  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Disable MIX compatibility mode
  //
  CpuState->MixCompatibilityMode = FALSE;

  return MMIX_SUCCESS;
}

/**
  Convert MIX word to MMIX 64-bit value.

  Converts a MIX 5-byte word (30 bits + sign) to MMIX format.
  MIX uses 6-bit bytes (0-63), so the value is packed as:
  sign * (byte1*64^4 + byte2*64^3 + byte3*64^2 + byte4*64 + byte5)

  @param[in]   MixWord    MIX word to convert.

  @return  64-bit MMIX representation.
**/
UINT64
MmixMixWordToUint64 (
  IN CONST MIX_WORD  *MixWord
  )
{
  UINT64  Value;
  INT64   SignedValue;

  //
  // Calculate magnitude from 5 bytes (each 6 bits)
  // Value = byte1*64^4 + byte2*64^3 + byte3*64^2 + byte4*64 + byte5
  //
  Value = ((UINT64)(MixWord->Bytes[0] & 0x3F) << 24) |  // byte1 * 64^4
          ((UINT64)(MixWord->Bytes[1] & 0x3F) << 18) |  // byte2 * 64^3
          ((UINT64)(MixWord->Bytes[2] & 0x3F) << 12) |  // byte3 * 64^2
          ((UINT64)(MixWord->Bytes[3] & 0x3F) << 6)  |  // byte4 * 64
          ((UINT64)(MixWord->Bytes[4] & 0x3F));         // byte5

  //
  // Apply sign
  //
  if (MixWord->Sign) {
    SignedValue = -(INT64)Value;
    return (UINT64)SignedValue;
  }

  return Value;
}

/**
  Convert MMIX 64-bit value to MIX word.

  Converts an MMIX 64-bit value to MIX 5-byte word format.
  Only the lower 30 bits of magnitude are preserved.

  @param[in]   Value      64-bit value to convert.
  @param[out]  MixWord    Resulting MIX word.
**/
VOID
MmixUint64ToMixWord (
  IN  UINT64    Value,
  OUT MIX_WORD  *MixWord
  )
{
  UINT64  Magnitude;
  INT64   SignedValue;

  //
  // Extract sign
  //
  SignedValue = (INT64)Value;
  if (SignedValue < 0) {
    MixWord->Sign = TRUE;
    Magnitude = (UINT64)(-SignedValue);
  } else {
    MixWord->Sign = FALSE;
    Magnitude = Value;
  }

  //
  // Extract 5 bytes (30 bits total, 6 bits each)
  // Mask to 30 bits to fit in MIX word
  //
  Magnitude &= 0x3FFFFFFF;  // 30 bits

  MixWord->Bytes[0] = (UINT8)((Magnitude >> 24) & 0x3F);  // byte1 (bits 29-24)
  MixWord->Bytes[1] = (UINT8)((Magnitude >> 18) & 0x3F);  // byte2 (bits 23-18)
  MixWord->Bytes[2] = (UINT8)((Magnitude >> 12) & 0x3F);  // byte3 (bits 17-12)
  MixWord->Bytes[3] = (UINT8)((Magnitude >> 6)  & 0x3F);  // byte4 (bits 11-6)
  MixWord->Bytes[4] = (UINT8)(Magnitude & 0x3F);          // byte5 (bits 5-0)
}

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
  )
{
  if (CpuState == NULL || Value == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Verify MIX mode is enabled
  //
  if (!CpuState->MixCompatibilityMode) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Validate register number
  //
  if (Register > MIX_REG_OVR) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Read from mapped MMIX register
  //
  *Value = CpuState->GeneralRegisters[Register];

  return MMIX_SUCCESS;
}

/**
  Write MIX register.

  Writes to a MIX register (A, X, I1-I6, J) in the CPU state.
  Index registers (I1-I6) and J are limited to 2 bytes.

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
  )
{
  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Verify MIX mode is enabled
  //
  if (!CpuState->MixCompatibilityMode) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Validate register number
  //
  if (Register > MIX_REG_OVR) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Apply constraints based on register type
  //
  if (Register >= MIX_REG_I1 && Register <= MIX_REG_J) {
    //
    // Index registers and J are 2 bytes (12 bits in MIX notation)
    // Limit to 0-4095
    //
    Value &= 0xFFF;
  } else if (Register == MIX_REG_A || Register == MIX_REG_X) {
    //
    // A and X are full 5-byte words (30 bits + sign)
    // Limit to 30-bit magnitude
    //
    INT64 Signed = (INT64)Value;
    if (Signed < 0) {
      Value = (UINT64)((-Signed) & 0x3FFFFFFF) | 0x8000000000000000ULL;
    } else {
      Value &= 0x3FFFFFFF;
    }
  }

  //
  // Write to mapped MMIX register
  //
  CpuState->GeneralRegisters[Register] = Value;

  return MMIX_SUCCESS;
}
