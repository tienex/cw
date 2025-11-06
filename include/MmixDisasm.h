/** @file
  MMIX Disassembler interface.

  This file provides the disassembler interface for converting MMIX
  binary instructions into human-readable assembly code.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_DISASM_H_
#define MMIX_DISASM_H_

#include "MmixTypes.h"

/**
  Disassemble a single MMIX instruction.

  This function takes a 32-bit instruction and produces assembly text.
  For compressed instructions (top 4 bits >= 0x8), it decodes the 16-bit
  format. The output buffer must be at least 80 characters.

  @param[in]      Instruction   The 32-bit instruction to disassemble.
  @param[in]      Address       The address of the instruction (for branches).
  @param[out]     Buffer        Buffer to receive the assembly text.
  @param[in]      BufferSize    Size of the buffer in bytes.

  @retval MMIX_SUCCESS          Instruction disassembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDisassemble (
  IN  UINT32  Instruction,
  IN  UINT64  Address,
  OUT CHAR8   *Buffer,
  IN  UINT32  BufferSize
  );

/**
  Disassemble a memory region.

  This function disassembles a region of memory containing MMIX code.
  It automatically detects compressed vs standard instructions.

  @param[in]      Memory        Pointer to memory region.
  @param[in]      Size          Size of memory region in bytes.
  @param[in]      StartAddress  Virtual address of first instruction.
  @param[out]     Buffer        Buffer to receive the assembly text.
  @param[in]      BufferSize    Size of the buffer in bytes.

  @retval MMIX_SUCCESS          Region disassembled successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDisassembleRegion (
  IN  CONST UINT8  *Memory,
  IN  UINT64       Size,
  IN  UINT64       StartAddress,
  OUT CHAR8        *Buffer,
  IN  UINT32       BufferSize
  );

/**
  Get the size of an instruction at a given address.

  @param[in]      Instruction   The instruction word.

  @return  2 for compressed instructions, 4 for standard instructions.

**/
UINT32
MmixGetInstructionSize (
  IN  UINT32  Instruction
  );

#endif // MMIX_DISASM_H_
