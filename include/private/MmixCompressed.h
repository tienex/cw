/** @file
  MMIX Compressed instruction private interface.

  Internal interface for 16-bit compressed instruction execution.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_COMPRESSED_PRIVATE_H__
#define __MMIX_COMPRESSED_PRIVATE_H__

#include "../MmixCore.h"

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
  );

#endif // __MMIX_COMPRESSED_PRIVATE_H__
