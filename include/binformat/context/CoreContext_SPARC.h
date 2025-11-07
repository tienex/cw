/** @file
  SPARC Family Register Context Structures.

  Includes SPARC V8 (32-bit) and SPARC V9 (64-bit) with register windows.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_SPARC_H__
#define __CORE_CONTEXT_SPARC_H__

//
// SPARC (32-bit) Context Flags
//
#define CONTEXT_SPARC32                 0x20000000
#define CONTEXT_SPARC32_CONTROL         (CONTEXT_SPARC32 | 0x00000001)  ///< PC, nPC, PSR, Y
#define CONTEXT_SPARC32_INTEGER         (CONTEXT_SPARC32 | 0x00000002)  ///< g0-g7, o0-o7, l0-l7, i0-i7
#define CONTEXT_SPARC32_FLOATING_POINT  (CONTEXT_SPARC32 | 0x00000004)  ///< f0-f31, FSR
#define CONTEXT_SPARC32_FULL            (CONTEXT_SPARC32_CONTROL | CONTEXT_SPARC32_INTEGER)
#define CONTEXT_SPARC32_ALL             (CONTEXT_SPARC32_FULL | CONTEXT_SPARC32_FLOATING_POINT)

//
// SPARC (64-bit) Context Flags
//
#define CONTEXT_SPARC64                 0x40000000
#define CONTEXT_SPARC64_CONTROL         (CONTEXT_SPARC64 | 0x00000001)  ///< PC, nPC, TSTATE, Y
#define CONTEXT_SPARC64_INTEGER         (CONTEXT_SPARC64 | 0x00000002)  ///< g0-g7, o0-o7, l0-l7, i0-i7
#define CONTEXT_SPARC64_FLOATING_POINT  (CONTEXT_SPARC64 | 0x00000004)  ///< f0-f63, FSR
#define CONTEXT_SPARC64_FULL            (CONTEXT_SPARC64_CONTROL | CONTEXT_SPARC64_INTEGER)
#define CONTEXT_SPARC64_ALL             (CONTEXT_SPARC64_FULL | CONTEXT_SPARC64_FLOATING_POINT)

///
/// SPARC 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _SPARC32_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;

  //
  // Global registers (G0-G7)
  //
  UINT32  G0;   ///< Always 0
  UINT32  G1;   ///< Temporary
  UINT32  G2;   ///< Application register
  UINT32  G3;   ///< Application register
  UINT32  G4;   ///< Application register
  UINT32  G5;   ///< Reserved
  UINT32  G6;   ///< Reserved
  UINT32  G7;   ///< Reserved

  //
  // Output registers (O0-O7)
  //
  UINT32  O0;   ///< Output / Return value
  UINT32  O1;   ///< Output / Return value
  UINT32  O2;   ///< Output
  UINT32  O3;   ///< Output
  UINT32  O4;   ///< Output
  UINT32  O5;   ///< Output
  UINT32  O6;   ///< Stack pointer
  UINT32  O7;   ///< Return address - 8

  //
  // Local registers (L0-L7) - in register window
  //
  UINT32  L0;
  UINT32  L1;
  UINT32  L2;
  UINT32  L3;
  UINT32  L4;
  UINT32  L5;
  UINT32  L6;
  UINT32  L7;

  //
  // Input registers (I0-I7) - in register window
  //
  UINT32  I0;   ///< Input
  UINT32  I1;   ///< Input
  UINT32  I2;   ///< Input
  UINT32  I3;   ///< Input
  UINT32  I4;   ///< Input
  UINT32  I5;   ///< Input
  UINT32  I6;   ///< Frame pointer
  UINT32  I7;   ///< Return address

  //
  // Special registers
  //
  UINT32  Pc;   ///< Program counter
  UINT32  Npc;  ///< Next program counter
  UINT32  Psr;  ///< Processor state register
  UINT32  Y;    ///< Multiply/divide register
  UINT32  Wim;  ///< Window invalid mask
  UINT32  Tbr;  ///< Trap base register

  //
  // Floating-point registers (F0-F31)
  //
  UINT32  FloatReg[32];

  //
  // Floating-point status register
  //
  UINT32  Fsr;
} SPARC32_CONTEXT, *PSPARC32_CONTEXT;

///
/// SPARC 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _SPARC64_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;
  UINT32  Padding;

  //
  // Global registers (G0-G7)
  //
  UINT64  G0;   ///< Always 0
  UINT64  G1;   ///< Temporary
  UINT64  G2;   ///< Application register
  UINT64  G3;   ///< Application register
  UINT64  G4;   ///< Application register
  UINT64  G5;   ///< Reserved
  UINT64  G6;   ///< Reserved
  UINT64  G7;   ///< Reserved

  //
  // Output registers (O0-O7)
  //
  UINT64  O0;   ///< Output / Return value
  UINT64  O1;   ///< Output / Return value
  UINT64  O2;   ///< Output
  UINT64  O3;   ///< Output
  UINT64  O4;   ///< Output
  UINT64  O5;   ///< Output
  UINT64  O6;   ///< Stack pointer
  UINT64  O7;   ///< Return address - 8

  //
  // Local registers (L0-L7) - in register window
  //
  UINT64  L0;
  UINT64  L1;
  UINT64  L2;
  UINT64  L3;
  UINT64  L4;
  UINT64  L5;
  UINT64  L6;
  UINT64  L7;

  //
  // Input registers (I0-I7) - in register window
  //
  UINT64  I0;   ///< Input
  UINT64  I1;   ///< Input
  UINT64  I2;   ///< Input
  UINT64  I3;   ///< Input
  UINT64  I4;   ///< Input
  UINT64  I5;   ///< Input
  UINT64  I6;   ///< Frame pointer
  UINT64  I7;   ///< Return address

  //
  // Special registers
  //
  UINT64  Pc;     ///< Program counter
  UINT64  Npc;    ///< Next program counter
  UINT64  Tstate; ///< Trap state register
  UINT64  Y;      ///< Multiply/divide register
  UINT64  Fprs;   ///< FP registers state
  UINT64  Tba;    ///< Trap base address

  //
  // Floating-point registers (F0-F63) - doubles
  //
  UINT64  F[64];

  //
  // Floating-point status register
  //
  UINT64  Fsr;
} SPARC64_CONTEXT, *PSPARC64_CONTEXT;

#endif // __CORE_CONTEXT_SPARC_H__
