/** @file
  PowerPC Family Register Context Structures.

  Includes PowerPC 32-bit, PowerPC 64-bit, and AltiVec/VMX/VSX extensions.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_POWERPC_H__
#define __CORE_CONTEXT_POWERPC_H__

//
// PowerPC (32-bit) Context Flags
//
#define CONTEXT_PPC32                   0x08000000
#define CONTEXT_PPC32_CONTROL           (CONTEXT_PPC32 | 0x00000001)  ///< PC, MSR, CR, LR, CTR, XER
#define CONTEXT_PPC32_INTEGER           (CONTEXT_PPC32 | 0x00000002)  ///< r0-r31
#define CONTEXT_PPC32_FLOATING_POINT    (CONTEXT_PPC32 | 0x00000004)  ///< f0-f31, FPSCR
#define CONTEXT_PPC32_VECTOR            (CONTEXT_PPC32 | 0x00000008)  ///< v0-v31, VSCR, VRSAVE (AltiVec)
#define CONTEXT_PPC32_DEBUG_REGISTERS   (CONTEXT_PPC32 | 0x00000010)  ///< Debug registers
#define CONTEXT_PPC32_FULL              (CONTEXT_PPC32_CONTROL | CONTEXT_PPC32_INTEGER)
#define CONTEXT_PPC32_ALL               (CONTEXT_PPC32_FULL | CONTEXT_PPC32_FLOATING_POINT | \
                                         CONTEXT_PPC32_VECTOR | CONTEXT_PPC32_DEBUG_REGISTERS)

//
// PowerPC (64-bit) Context Flags
//
#define CONTEXT_PPC64                   0x10000000
#define CONTEXT_PPC64_CONTROL           (CONTEXT_PPC64 | 0x00000001)  ///< PC, MSR, CR, LR, CTR, XER
#define CONTEXT_PPC64_INTEGER           (CONTEXT_PPC64 | 0x00000002)  ///< r0-r31
#define CONTEXT_PPC64_FLOATING_POINT    (CONTEXT_PPC64 | 0x00000004)  ///< f0-f31, FPSCR
#define CONTEXT_PPC64_VECTOR            (CONTEXT_PPC64 | 0x00000008)  ///< v0-v31, VSCR, VRSAVE (AltiVec)
#define CONTEXT_PPC64_DEBUG_REGISTERS   (CONTEXT_PPC64 | 0x00000010)  ///< Debug registers
#define CONTEXT_PPC64_FULL              (CONTEXT_PPC64_CONTROL | CONTEXT_PPC64_INTEGER)
#define CONTEXT_PPC64_ALL               (CONTEXT_PPC64_FULL | CONTEXT_PPC64_FLOATING_POINT | \
                                         CONTEXT_PPC64_VECTOR | CONTEXT_PPC64_DEBUG_REGISTERS)

///
/// PowerPC 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _PPC32_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;

  //
  // Integer registers (R0-R31)
  //
  UINT32  R0;
  UINT32  R1;   ///< Stack pointer
  UINT32  R2;   ///< TOC pointer / System reserved
  UINT32  R3;   ///< First argument / Return value
  UINT32  R4;   ///< Second argument / Return value
  UINT32  R5;   ///< Argument
  UINT32  R6;   ///< Argument
  UINT32  R7;   ///< Argument
  UINT32  R8;   ///< Argument
  UINT32  R9;   ///< Argument
  UINT32  R10;  ///< Argument
  UINT32  R11;  ///< Pointer to env / Scratch
  UINT32  R12;  ///< Scratch / Exception handling
  UINT32  R13;  ///< Thread pointer / Small data pointer
  UINT32  R14;  ///< Local variable
  UINT32  R15;  ///< Local variable
  UINT32  R16;  ///< Local variable
  UINT32  R17;  ///< Local variable
  UINT32  R18;  ///< Local variable
  UINT32  R19;  ///< Local variable
  UINT32  R20;  ///< Local variable
  UINT32  R21;  ///< Local variable
  UINT32  R22;  ///< Local variable
  UINT32  R23;  ///< Local variable
  UINT32  R24;  ///< Local variable
  UINT32  R25;  ///< Local variable
  UINT32  R26;  ///< Local variable
  UINT32  R27;  ///< Local variable
  UINT32  R28;  ///< Local variable
  UINT32  R29;  ///< Local variable
  UINT32  R30;  ///< Local variable
  UINT32  R31;  ///< Local variable / Frame pointer

  //
  // Special registers
  //
  UINT32  Pc;   ///< Program counter (NIP - Next Instruction Pointer)
  UINT32  Msr;  ///< Machine state register
  UINT32  Cr;   ///< Condition register
  UINT32  Lr;   ///< Link register
  UINT32  Ctr;  ///< Count register
  UINT32  Xer;  ///< Fixed-point exception register

  //
  // Floating-point registers (F0-F31)
  //
  FLOAT64  FloatReg[32];

  //
  // Floating-point status and control register
  //
  UINT64  Fpscr;

  //
  // AltiVec/VMX vector registers (V0-V31)
  //
  M128A   V[32];

  //
  // Vector status and control register
  //
  UINT32  Vscr;
  UINT32  Vrsave;

  //
  // Debug registers
  //
  UINT32  Dar;   ///< Data address register
  UINT32  Dsisr; ///< DSI status register
} PPC32_CONTEXT, *PPPC32_CONTEXT;

///
/// PowerPC 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _PPC64_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;
  UINT32  Padding;

  //
  // Integer registers (R0-R31)
  //
  UINT64  R0;
  UINT64  R1;   ///< Stack pointer
  UINT64  R2;   ///< TOC pointer
  UINT64  R3;   ///< First argument / Return value
  UINT64  R4;   ///< Second argument / Return value
  UINT64  R5;   ///< Argument
  UINT64  R6;   ///< Argument
  UINT64  R7;   ///< Argument
  UINT64  R8;   ///< Argument
  UINT64  R9;   ///< Argument
  UINT64  R10;  ///< Argument
  UINT64  R11;  ///< Pointer to env / Scratch
  UINT64  R12;  ///< Scratch / Exception handling
  UINT64  R13;  ///< Thread pointer
  UINT64  R14;  ///< Local variable
  UINT64  R15;  ///< Local variable
  UINT64  R16;  ///< Local variable
  UINT64  R17;  ///< Local variable
  UINT64  R18;  ///< Local variable
  UINT64  R19;  ///< Local variable
  UINT64  R20;  ///< Local variable
  UINT64  R21;  ///< Local variable
  UINT64  R22;  ///< Local variable
  UINT64  R23;  ///< Local variable
  UINT64  R24;  ///< Local variable
  UINT64  R25;  ///< Local variable
  UINT64  R26;  ///< Local variable
  UINT64  R27;  ///< Local variable
  UINT64  R28;  ///< Local variable
  UINT64  R29;  ///< Local variable
  UINT64  R30;  ///< Local variable
  UINT64  R31;  ///< Local variable / Frame pointer

  //
  // Special registers
  //
  UINT64  Pc;   ///< Program counter (NIP - Next Instruction Pointer)
  UINT64  Msr;  ///< Machine state register
  UINT64  Cr;   ///< Condition register
  UINT64  Lr;   ///< Link register
  UINT64  Ctr;  ///< Count register
  UINT64  Xer;  ///< Fixed-point exception register

  //
  // Floating-point registers (F0-F31)
  //
  FLOAT64  FloatReg[32];

  //
  // Floating-point status and control register
  //
  UINT64  Fpscr;

  //
  // AltiVec/VMX vector registers (V0-V31)
  //
  M128A   V[32];

  //
  // Vector status and control register
  //
  UINT32  Vscr;
  UINT32  Vrsave;

  //
  // VSX (Vector-Scalar Extension) registers
  //
  UINT64  Vsr[32];  ///< Additional VSX registers (VSR32-VSR63)

  //
  // Debug registers
  //
  UINT64  Dar;   ///< Data address register
  UINT64  Dsisr; ///< DSI status register
} PPC64_CONTEXT, *PPPC64_CONTEXT;

#endif // __CORE_CONTEXT_POWERPC_H__
