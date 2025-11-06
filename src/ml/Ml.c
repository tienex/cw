/** @file
  MMIX Machine Learning/AI Helper Implementations.

  This file implements helper functions for ML/NN/AI operations.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../../include/MmixMl.h"

/**
  Apply activation function to a single value.

  @param[in]      Type          Activation function type.
  @param[in]      X             Input value (FP64).
  @param[in]      Alpha         Parameter for parameterized activations.

  @return  Activated value.

**/
FLOAT64
MmixMlActivate (
  IN  MMIX_ACTIVATION_TYPE  Type,
  IN  FLOAT64               X,
  IN  FLOAT64               Alpha
  )
{
  switch (Type) {
    case MmixActivationReLU:
      return (X > 0.0) ? X : 0.0;

    case MmixActivationLeakyReLU:
      return (X > 0.0) ? X : (Alpha * X);

    case MmixActivationSigmoid:
      return 1.0 / (1.0 + exp (-X));

    case MmixActivationTanh:
      return tanh (X);

    case MmixActivationGELU:
      // Approximate GELU: 0.5 * x * (1 + tanh(sqrt(2/pi) * (x + 0.044715 * x^3)))
      {
        FLOAT64  X3 = X * X * X;
        FLOAT64  Inner = sqrt (2.0 / M_PI) * (X + 0.044715 * X3);
        return 0.5 * X * (1.0 + tanh (Inner));
      }

    case MmixActivationSwish:
      return X / (1.0 + exp (-X));

    case MmixActivationSoftmax:
      // Softmax requires vector context, return exp(x) for single value
      return exp (X);

    default:
      return X;
  }
}

/**
  Apply activation function to vector register.

  @param[in,out]  CpuState      CPU state.
  @param[in]      VecReg        Vector register number.
  @param[in]      Type          Activation function type.
  @param[in]      Alpha         Parameter for parameterized activations.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlActivateVector (
  IN OUT MMIX_CPU_STATE     *CpuState,
  IN     UINT32             VecReg,
  IN     MMIX_ACTIVATION_TYPE  Type,
  IN     FLOAT64            Alpha
  )
{
  if (CpuState == NULL || VecReg >= MMIX_VECTOR_REGISTER_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  MMIX_VECTOR_REGISTER  *Vec = &CpuState->VectorRegisters[VecReg];
  UINT32                ElementCount = Vec->LengthBytes / sizeof (FLOAT64);

  // Apply activation to each element
  FLOAT64  *Elements = (FLOAT64 *)Vec->Data;
  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    Elements[i] = MmixMlActivate (Type, Elements[i], Alpha);
  }

  return MMIX_SUCCESS;
}

/**
  Compute dot product of two vectors.

  @param[in,out]  CpuState      CPU state.
  @param[in]      VecA          First vector register.
  @param[in]      VecB          Second vector register.
  @param[out]     Result        Pointer to result.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlVectorDotProduct (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT32          VecA,
  IN     UINT32          VecB,
  OUT    FLOAT64         *Result
  )
{
  if (CpuState == NULL || Result == NULL ||
      VecA >= MMIX_VECTOR_REGISTER_COUNT ||
      VecB >= MMIX_VECTOR_REGISTER_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  MMIX_VECTOR_REGISTER  *Va = &CpuState->VectorRegisters[VecA];
  MMIX_VECTOR_REGISTER  *Vb = &CpuState->VectorRegisters[VecB];

  // Ensure same length
  if (Va->LengthBytes != Vb->LengthBytes) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  UINT32   ElementCount = Va->LengthBytes / sizeof (FLOAT64);
  FLOAT64  *A = (FLOAT64 *)Va->Data;
  FLOAT64  *B = (FLOAT64 *)Vb->Data;
  FLOAT64  Sum = 0.0;

  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Va->Data) / sizeof (FLOAT64)); i++) {
    Sum += A[i] * B[i];
  }

  *Result = Sum;
  return MMIX_SUCCESS;
}

/**
  Perform matrix multiplication: C = A * B.

  @param[in,out]  CpuState      CPU state.
  @param[in]      MatA          Matrix A tile register.
  @param[in]      MatB          Matrix B tile register.
  @param[out]     MatC          Matrix C tile register (result).

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlMatrixMultiply (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT32          MatA,
  IN     UINT32          MatB,
  OUT    UINT32          MatC
  )
{
  if (CpuState == NULL ||
      MatA >= MMIX_MATRIX_TILE_COUNT ||
      MatB >= MMIX_MATRIX_TILE_COUNT ||
      MatC >= MMIX_MATRIX_TILE_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  // Basic matrix multiplication implementation
  // TODO: Optimize with SIMD/blocking for cache efficiency

  MMIX_MATRIX_TILE  *A = &CpuState->MatrixTiles[MatA];
  MMIX_MATRIX_TILE  *B = &CpuState->MatrixTiles[MatB];
  MMIX_MATRIX_TILE  *C = &CpuState->MatrixTiles[MatC];

  // Zero output
  memset (C->Data, 0, sizeof (C->Data));

  // Compute C = A * B
  UINT32  Rows = A->Rows;
  UINT32  Cols = B->Columns;
  UINT32  Inner = A->Columns;

  for (UINT32 i = 0; i < Rows; i++) {
    for (UINT32 j = 0; j < Cols; j++) {
      FLOAT64  Sum = 0.0;
      for (UINT32 k = 0; k < Inner; k++) {
        // A[i][k] * B[k][j]
        UINT64  AVal = A->Data[i * A->Columns + k];
        UINT64  BVal = B->Data[k * B->Columns + j];

        // Convert to FP64 for computation
        FLOAT64  AFloat = *(FLOAT64 *)&AVal;
        FLOAT64  BFloat = *(FLOAT64 *)&BVal;

        Sum += AFloat * BFloat;
      }
      C->Data[i * C->Columns + j] = *(UINT64 *)&Sum;
    }
  }

  C->Rows = Rows;
  C->Columns = Cols;

  return MMIX_SUCCESS;
}

/**
  Fused matrix multiply-accumulate: C += A * B.

  @param[in,out]  CpuState      CPU state.
  @param[in]      MatA          Matrix A tile register.
  @param[in]      MatB          Matrix B tile register.
  @param[in,out]  MatC          Matrix C tile register (accumulator).

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlMatrixMultiplyAccumulate (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT32          MatA,
  IN     UINT32          MatB,
  IN OUT UINT32          MatC
  )
{
  if (CpuState == NULL ||
      MatA >= MMIX_MATRIX_TILE_COUNT ||
      MatB >= MMIX_MATRIX_TILE_COUNT ||
      MatC >= MMIX_MATRIX_TILE_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  MMIX_MATRIX_TILE  *A = &CpuState->MatrixTiles[MatA];
  MMIX_MATRIX_TILE  *B = &CpuState->MatrixTiles[MatB];
  MMIX_MATRIX_TILE  *C = &CpuState->MatrixTiles[MatC];

  UINT32  Rows = A->Rows;
  UINT32  Cols = B->Columns;
  UINT32  Inner = A->Columns;

  for (UINT32 i = 0; i < Rows; i++) {
    for (UINT32 j = 0; j < Cols; j++) {
      FLOAT64  Sum = 0.0;
      for (UINT32 k = 0; k < Inner; k++) {
        UINT64   AVal = A->Data[i * A->Columns + k];
        UINT64   BVal = B->Data[k * B->Columns + j];
        FLOAT64  AFloat = *(FLOAT64 *)&AVal;
        FLOAT64  BFloat = *(FLOAT64 *)&BVal;
        Sum += AFloat * BFloat;
      }

      // Accumulate into existing C value
      UINT64   CVal = C->Data[i * C->Columns + j];
      FLOAT64  CFloat = *(FLOAT64 *)&CVal;
      CFloat += Sum;
      C->Data[i * C->Columns + j] = *(UINT64 *)&CFloat;
    }
  }

  return MMIX_SUCCESS;
}

/**
  Quantize floating-point value to specified type.

  @param[in]      Value         Input floating-point value.
  @param[in]      Type          Quantization type.
  @param[out]     Quantized     Pointer to quantized result.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlQuantize (
  IN  FLOAT64         Value,
  IN  MMIX_QUANT_TYPE Type,
  OUT UINT64          *Quantized
  )
{
  if (Quantized == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  switch (Type) {
    case MmixQuantInt8:
      // Clamp to [-128, 127] and round
      {
        INT8  Result = (INT8)round (fmax (-128.0, fmin (127.0, Value)));
        *Quantized = (UINT64)(INT64)Result;
      }
      break;

    case MmixQuantInt16:
      {
        INT16  Result = (INT16)round (fmax (-32768.0, fmin (32767.0, Value)));
        *Quantized = (UINT64)(INT64)Result;
      }
      break;

    case MmixQuantFloat16:
    case MmixQuantBFloat16:
      // TODO: Implement proper FP16/BF16 conversion
      // For now, truncate mantissa
      {
        UINT64  Bits = *(UINT64 *)&Value;
        *Quantized = Bits >> 48;  // Simplified truncation
      }
      break;

    default:
      return MMIX_ERROR_INVALID_PARAMETER;
  }

  return MMIX_SUCCESS;
}

/**
  Dequantize value back to floating-point.

  @param[in]      Quantized     Quantized value.
  @param[in]      Type          Quantization type.
  @param[out]     Value         Pointer to dequantized FP value.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlDequantize (
  IN  UINT64          Quantized,
  IN  MMIX_QUANT_TYPE Type,
  OUT FLOAT64         *Value
  )
{
  if (Value == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  switch (Type) {
    case MmixQuantInt8:
      *Value = (FLOAT64)(INT8)Quantized;
      break;

    case MmixQuantInt16:
      *Value = (FLOAT64)(INT16)Quantized;
      break;

    case MmixQuantFloat16:
    case MmixQuantBFloat16:
      // TODO: Implement proper FP16/BF16 conversion
      {
        UINT64  Bits = Quantized << 48;
        *Value = *(FLOAT64 *)&Bits;
      }
      break;

    default:
      return MMIX_ERROR_INVALID_PARAMETER;
  }

  return MMIX_SUCCESS;
}

/**
  Compute softmax normalization on vector.

  @param[in,out]  CpuState      CPU state.
  @param[in,out]  VecReg        Vector register (input and output).

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlSoftmax (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN OUT UINT32          VecReg
  )
{
  if (CpuState == NULL || VecReg >= MMIX_VECTOR_REGISTER_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  MMIX_VECTOR_REGISTER  *Vec = &CpuState->VectorRegisters[VecReg];
  UINT32                ElementCount = Vec->LengthBytes / sizeof (FLOAT64);
  FLOAT64               *Elements = (FLOAT64 *)Vec->Data;

  // Find max for numerical stability
  FLOAT64  MaxVal = Elements[0];
  for (UINT32 i = 1; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    if (Elements[i] > MaxVal) {
      MaxVal = Elements[i];
    }
  }

  // Compute exp(x - max) and sum
  FLOAT64  Sum = 0.0;
  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    Elements[i] = exp (Elements[i] - MaxVal);
    Sum += Elements[i];
  }

  // Normalize
  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    Elements[i] /= Sum;
  }

  return MMIX_SUCCESS;
}

/**
  Layer normalization helper.

  @param[in,out]  CpuState      CPU state.
  @param[in,out]  VecReg        Vector register to normalize.
  @param[in]      Epsilon       Small value to prevent division by zero.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlLayerNorm (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN OUT UINT32          VecReg,
  IN     FLOAT64         Epsilon
  )
{
  if (CpuState == NULL || VecReg >= MMIX_VECTOR_REGISTER_COUNT) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  MMIX_VECTOR_REGISTER  *Vec = &CpuState->VectorRegisters[VecReg];
  UINT32                ElementCount = Vec->LengthBytes / sizeof (FLOAT64);
  FLOAT64               *Elements = (FLOAT64 *)Vec->Data;

  // Compute mean
  FLOAT64  Mean = 0.0;
  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    Mean += Elements[i];
  }
  Mean /= (FLOAT64)ElementCount;

  // Compute variance
  FLOAT64  Variance = 0.0;
  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    FLOAT64  Diff = Elements[i] - Mean;
    Variance += Diff * Diff;
  }
  Variance /= (FLOAT64)ElementCount;

  // Normalize: (x - mean) / sqrt(variance + epsilon)
  FLOAT64  StdDev = sqrt (Variance + Epsilon);
  for (UINT32 i = 0; i < ElementCount && i < (sizeof (Vec->Data) / sizeof (FLOAT64)); i++) {
    Elements[i] = (Elements[i] - Mean) / StdDev;
  }

  return MMIX_SUCCESS;
}

// Stubs for remaining functions
MMIX_STATUS MmixMlConv1D (IN OUT MMIX_CPU_STATE *CpuState, IN UINT32 Input, IN UINT32 Kernel, OUT UINT32 Output, IN UINT32 Stride) { return MMIX_ERROR_NOT_IMPLEMENTED; }
MMIX_STATUS MmixMlBatchNorm (IN OUT MMIX_CPU_STATE *CpuState, IN OUT UINT32 VecReg, IN FLOAT64 Mean, IN FLOAT64 Variance, IN FLOAT64 Gamma, IN FLOAT64 Beta) { return MMIX_ERROR_NOT_IMPLEMENTED; }
MMIX_STATUS MmixMlPool (IN OUT MMIX_CPU_STATE *CpuState, IN UINT32 Input, OUT UINT32 Output, IN UINT32 PoolSize, IN BOOLEAN IsMaxPool) { return MMIX_ERROR_NOT_IMPLEMENTED; }
