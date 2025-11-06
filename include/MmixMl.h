/** @file
  MMIX Machine Learning/AI Instruction Helpers.

  This file provides helper functions for ML/NN/AI operations,
  simplifying the implementation of tensor operations, neural network
  primitives, and activation functions.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_ML_H_
#define MMIX_ML_H_

#include "MmixTypes.h"
#include "MmixCore.h"

/**
  Activation function types.
**/
typedef enum {
  MmixActivationReLU,        ///< Rectified Linear Unit: max(0, x)
  MmixActivationLeakyReLU,   ///< Leaky ReLU: x > 0 ? x : alpha * x
  MmixActivationSigmoid,     ///< Sigmoid: 1 / (1 + exp(-x))
  MmixActivationTanh,        ///< Hyperbolic tangent
  MmixActivationSoftmax,     ///< Softmax (for vectors)
  MmixActivationGELU,        ///< Gaussian Error Linear Unit
  MmixActivationSwish,       ///< Swish: x * sigmoid(x)
} MMIX_ACTIVATION_TYPE;

/**
  Quantization types.
**/
typedef enum {
  MmixQuantInt8,      ///< 8-bit integer quantization
  MmixQuantInt16,     ///< 16-bit integer quantization
  MmixQuantFloat16,   ///< 16-bit floating-point (half precision)
  MmixQuantBFloat16,  ///< BFloat16 (Brain Floating Point)
} MMIX_QUANT_TYPE;

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Perform 1D convolution on vector data.

  @param[in,out]  CpuState      CPU state.
  @param[in]      Input         Input vector register.
  @param[in]      Kernel        Kernel/filter vector register.
  @param[out]     Output        Output vector register.
  @param[in]      Stride        Convolution stride.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlConv1D (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT32          Input,
  IN     UINT32          Kernel,
  OUT    UINT32          Output,
  IN     UINT32          Stride
  );

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
  );

/**
  Batch normalization helper.

  @param[in,out]  CpuState      CPU state.
  @param[in,out]  VecReg        Vector register to normalize.
  @param[in]      Mean          Mean value.
  @param[in]      Variance      Variance value.
  @param[in]      Gamma         Scale parameter.
  @param[in]      Beta          Shift parameter.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlBatchNorm (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN OUT UINT32          VecReg,
  IN     FLOAT64         Mean,
  IN     FLOAT64         Variance,
  IN     FLOAT64         Gamma,
  IN     FLOAT64         Beta
  );

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
  );

/**
  Perform pooling operation (max or average).

  @param[in,out]  CpuState      CPU state.
  @param[in]      Input         Input vector register.
  @param[out]     Output        Output vector register.
  @param[in]      PoolSize      Size of pooling window.
  @param[in]      IsMaxPool     TRUE for max pooling, FALSE for average.

  @retval  MMIX_SUCCESS          Success.
  @retval  MMIX_ERROR_*          Error code.

**/
MMIX_STATUS
MmixMlPool (
  IN OUT MMIX_CPU_STATE  *CpuState,
  IN     UINT32          Input,
  OUT    UINT32          Output,
  IN     UINT32          PoolSize,
  IN     BOOLEAN         IsMaxPool
  );

#endif  // MMIX_ML_H_
