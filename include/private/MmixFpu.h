/** @file
  MMIX FPU private interface.

  Internal interface for floating-point unit operations.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_FPU_PRIVATE_H__
#define __MMIX_FPU_PRIVATE_H__

#include "../MmixCore.h"

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
  );

#endif // __MMIX_FPU_PRIVATE_H__
