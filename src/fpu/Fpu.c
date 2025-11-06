/** @file
  MMIX Floating-point unit implementation.

  This file implements floating-point operations for the MMIX emulator
  including IEEE 754-2008 binary formats and conversions.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <fenv.h>
#include "../../include/MmixCore.h"

//
// Floating-point helper macros
//

#define FP_QUIET_NAN      (0x7FF8000000000000ULL)
#define FP_POS_INFINITY   (0x7FF0000000000000ULL)
#define FP_NEG_INFINITY   (0xFFF0000000000000ULL)
#define FP_POS_ZERO       (0x0000000000000000ULL)
#define FP_NEG_ZERO       (0x8000000000000000ULL)

/**
  Set rounding mode.

  @param[in]  RoundingMode      MMIX rounding mode.

  @retval Previous rounding mode.

**/
STATIC
INT32
MmixFpuSetRoundingMode (
  IN MMIX_ROUNDING_MODE  RoundingMode
  )
{
  INT32  OldMode;

  OldMode = fegetround ();

  switch (RoundingMode) {
    case MmixRoundNearestEven:
      fesetround (FE_TONEAREST);
      break;

    case MmixRoundTowardZero:
      fesetround (FE_TOWARDZERO);
      break;

    case MmixRoundTowardNegInf:
      fesetround (FE_DOWNWARD);
      break;

    case MmixRoundTowardPosInf:
      fesetround (FE_UPWARD);
      break;

    default:
      fesetround (FE_TONEAREST);
      break;
  }

  return OldMode;
}

/**
  Check and update floating-point exception flags.

  @param[in,out]  CpuState      Pointer to CPU state.

**/
STATIC
VOID
MmixFpuUpdateFlags (
  IN OUT MMIX_CPU_STATE  *CpuState
  )
{
  INT32  Exceptions;

  //
  // Get FP exceptions
  //
  Exceptions = fetestexcept (FE_ALL_EXCEPT);

  //
  // Update rA register
  //
  if (Exceptions & FE_INEXACT) {
    CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_INEXACT;
  }
  if (Exceptions & FE_UNDERFLOW) {
    CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_UNDERFLOW;
  }
  if (Exceptions & FE_OVERFLOW) {
    CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_FP_OVERFLOW;
  }
  if (Exceptions & FE_DIVBYZERO) {
    CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_DIVIDE_BY_ZERO;
  }
  if (Exceptions & FE_INVALID) {
    CpuState->SpecialRegisters[MMIX_rA] |= MMIX_ARITH_FLAG_INVALID_OP;
  }

  //
  // Clear FP exceptions for next operation
  //
  feclearexcept (FE_ALL_EXCEPT);
}

/**
  Convert UINT64 to double (as bit pattern).

  @param[in]  Value             64-bit value.

  @return Double value.

**/
STATIC
double
MmixFpuUint64ToDouble (
  IN UINT64  Value
  )
{
  union {
    UINT64  u;
    double  d;
  } Convert;

  Convert.u = Value;
  return Convert.d;
}

/**
  Convert double to UINT64 (as bit pattern).

  @param[in]  Value             Double value.

  @return 64-bit value.

**/
STATIC
UINT64
MmixFpuDoubleToUint64 (
  IN double  Value
  )
{
  union {
    double  d;
    UINT64  u;
  } Convert;

  Convert.d = Value;
  return Convert.u;
}

/**
  Execute floating-point add instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Source register 1.
  @param[in]      Z             Source register 2.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuAdd (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  YValue, ZValue;
  double  YFloat, ZFloat, Result;
  INT32   OldRound;

  //
  // Read operands
  //
  MmixCpuReadRegister (CpuState, Y, &YValue);
  MmixCpuReadRegister (CpuState, Z, &ZValue);

  //
  // Convert to double
  //
  YFloat = MmixFpuUint64ToDouble (YValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  //
  // Set rounding mode
  //
  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);

  //
  // Perform addition
  //
  Result = YFloat + ZFloat;

  //
  // Restore rounding mode
  //
  fesetround (OldRound);

  //
  // Update flags
  //
  MmixFpuUpdateFlags (CpuState);

  //
  // Write result
  //
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Execute floating-point subtract instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Source register 1.
  @param[in]      Z             Source register 2.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuSub (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  YValue, ZValue;
  double  YFloat, ZFloat, Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, Y, &YValue);
  MmixCpuReadRegister (CpuState, Z, &ZValue);

  YFloat = MmixFpuUint64ToDouble (YValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);
  Result = YFloat - ZFloat;
  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Execute floating-point multiply instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Source register 1.
  @param[in]      Z             Source register 2.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuMul (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  YValue, ZValue;
  double  YFloat, ZFloat, Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, Y, &YValue);
  MmixCpuReadRegister (CpuState, Z, &ZValue);

  YFloat = MmixFpuUint64ToDouble (YValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);
  Result = YFloat * ZFloat;
  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Execute floating-point divide instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Source register 1.
  @param[in]      Z             Source register 2.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuDiv (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  YValue, ZValue;
  double  YFloat, ZFloat, Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, Y, &YValue);
  MmixCpuReadRegister (CpuState, Z, &ZValue);

  YFloat = MmixFpuUint64ToDouble (YValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);
  Result = YFloat / ZFloat;
  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Execute floating-point square root instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Source register (unused).
  @param[in]      Z             Source register.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuSqrt (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  ZValue;
  double  ZFloat, Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, Z, &ZValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);
  Result = sqrt (ZFloat);
  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Execute floating-point compare instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Source register 1.
  @param[in]      Z             Source register 2.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuCompare (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  YValue, ZValue;
  double  YFloat, ZFloat;
  UINT64  Result;

  MmixCpuReadRegister (CpuState, Y, &YValue);
  MmixCpuReadRegister (CpuState, Z, &ZValue);

  YFloat = MmixFpuUint64ToDouble (YValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  //
  // Check for NaN
  //
  if (isnan (YFloat) || isnan (ZFloat)) {
    Result = 2;  // Unordered
  } else if (YFloat < ZFloat) {
    Result = (UINT64)-1;  // Less than
  } else if (YFloat == ZFloat) {
    Result = 0;  // Equal
  } else {
    Result = 1;  // Greater than
  }

  MmixCpuWriteRegister (CpuState, X, Result);
  return MMIX_SUCCESS;
}

/**
  Convert integer to floating-point.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Unused.
  @param[in]      Z             Source register.
  @param[in]      Unsigned      TRUE if unsigned conversion.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuIntToFloat (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z,
  IN     BOOLEAN         Unsigned
  )
{
  UINT64  ZValue;
  double  Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, Z, &ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);

  if (Unsigned) {
    Result = (double)ZValue;
  } else {
    Result = (double)(INT64)ZValue;
  }

  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Convert floating-point to integer.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register.
  @param[in]      Y             Unused.
  @param[in]      Z             Source register.
  @param[in]      Unsigned      TRUE if unsigned conversion.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuFloatToInt (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z,
  IN     BOOLEAN         Unsigned
  )
{
  UINT64  ZValue;
  double  ZFloat;
  UINT64  Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, Z, &ZValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);

  if (Unsigned) {
    //
    // Convert to unsigned
    //
    if (ZFloat < 0.0) {
      Result = 0;
    } else if (ZFloat >= (double)UINT64_MAX) {
      Result = UINT64_MAX;
    } else {
      Result = (UINT64)ZFloat;
    }
  } else {
    //
    // Convert to signed
    //
    Result = (UINT64)(INT64)ZFloat;
  }

  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, Result);

  return MMIX_SUCCESS;
}

/**
  Execute floating-point fused multiply-add.

  Computes (X * Y) + Z with single rounding.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      X             Destination register (also source).
  @param[in]      Y             Source register 1.
  @param[in]      Z             Source register 2.

  @retval MMIX_SUCCESS          Operation completed.

**/
STATIC
MMIX_STATUS
MmixFpuFma (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT8           X,
  IN     UINT8           Y,
  IN     UINT8           Z
  )
{
  UINT64  XValue, YValue, ZValue;
  double  XFloat, YFloat, ZFloat, Result;
  INT32   OldRound;

  MmixCpuReadRegister (CpuState, X, &XValue);
  MmixCpuReadRegister (CpuState, Y, &YValue);
  MmixCpuReadRegister (CpuState, Z, &ZValue);

  XFloat = MmixFpuUint64ToDouble (XValue);
  YFloat = MmixFpuUint64ToDouble (YValue);
  ZFloat = MmixFpuUint64ToDouble (ZValue);

  OldRound = MmixFpuSetRoundingMode (CpuState->RoundingMode);
  Result = fma (YFloat, ZFloat, XFloat);  // (Y * Z) + X
  fesetround (OldRound);

  MmixFpuUpdateFlags (CpuState);
  MmixCpuWriteRegister (CpuState, X, MmixFpuDoubleToUint64 (Result));

  return MMIX_SUCCESS;
}

/**
  Execute floating-point instruction.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Inst          The instruction.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Exception occurred.

**/
MMIX_STATUS
MmixFpuExecute (
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Inst
  )
{
  UINT8  Opcode;

  if (CpuState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Opcode = Inst.Fields.Opcode;

  //
  // Clear FP exceptions before operation
  //
  feclearexcept (FE_ALL_EXCEPT);

  //
  // Dispatch to appropriate FP operation
  //
  switch (Opcode) {
    case 0x60: // FADD
      return MmixFpuAdd (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    case 0x61: // FSUB
      return MmixFpuSub (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    case 0x62: // FMUL
      return MmixFpuMul (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    case 0x63: // FDIV
      return MmixFpuDiv (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    case 0x65: // FSQRT
      return MmixFpuSqrt (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    case 0x69: // FLOT - signed int to float
      return MmixFpuIntToFloat (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z, FALSE);

    case 0x6A: // FLOTU - unsigned int to float
      return MmixFpuIntToFloat (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z, TRUE);

    case 0x67: // FIX - float to signed int
      return MmixFpuFloatToInt (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z, FALSE);

    case 0x68: // FIXU - float to unsigned int
      return MmixFpuFloatToInt (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z, TRUE);

    case 0x6D: // FCMP - floating-point compare
      return MmixFpuCompare (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    case 0x75: // FMA - fused multiply-add
      return MmixFpuFma (CpuState, Inst.Fields.X, Inst.Fields.Y, Inst.Fields.Z);

    default:
      //
      // Unimplemented FP instruction
      //
      return MmixCpuRaiseException (CpuState, MmixExceptionInvalidOperation, 0);
  }
}
