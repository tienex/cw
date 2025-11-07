/** @file
  Alpha and Sunway Register Context Structures.

  Includes DEC Alpha AXP and Sunway SW64 (Alpha-derived architecture).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_ALPHA_H__
#define __CORE_CONTEXT_ALPHA_H__

//
// Alpha/Sunway Context Flags
//
#define CONTEXT_ALPHA64                 0x00010000
#define CONTEXT_ALPHA64_CONTROL         (CONTEXT_ALPHA64 | 0x00000001)
#define CONTEXT_ALPHA64_INTEGER         (CONTEXT_ALPHA64 | 0x00000002)
#define CONTEXT_ALPHA64_FLOATING_POINT  (CONTEXT_ALPHA64 | 0x00000004)
#define CONTEXT_ALPHA64_FULL            (CONTEXT_ALPHA64_CONTROL | CONTEXT_ALPHA64_INTEGER)
#define CONTEXT_ALPHA64_ALL             (CONTEXT_ALPHA64_FULL | CONTEXT_ALPHA64_FLOATING_POINT)

#define CONTEXT_SUNWAY64                0x00020000
#define CONTEXT_SUNWAY64_CONTROL        (CONTEXT_SUNWAY64 | 0x00000001)
#define CONTEXT_SUNWAY64_INTEGER        (CONTEXT_SUNWAY64 | 0x00000002)
#define CONTEXT_SUNWAY64_FLOATING_POINT (CONTEXT_SUNWAY64 | 0x00000004)
#define CONTEXT_SUNWAY64_VECTOR         (CONTEXT_SUNWAY64 | 0x00000008)  ///< 512-bit vectors
#define CONTEXT_SUNWAY64_FULL           (CONTEXT_SUNWAY64_CONTROL | CONTEXT_SUNWAY64_INTEGER)
#define CONTEXT_SUNWAY64_ALL            (CONTEXT_SUNWAY64_FULL | CONTEXT_SUNWAY64_FLOATING_POINT | \
                                         CONTEXT_SUNWAY64_VECTOR)

///
/// DEC Alpha AXP Context Structure
/// Note: Alpha is always 64-bit; "Alpha32" refers to 32-bit pointers in NT,
/// not a processor mode. All registers are full 64-bit.
///
typedef struct ALIGNED_STRUCT(8) _ALPHA64_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Reserved;

  //
  // Integer registers V0-T12 (R0-R31)
  //
  UINT64  V0;       ///< R0: Return value/temp
  UINT64  T0;       ///< R1: Temporary
  UINT64  T1;       ///< R2: Temporary
  UINT64  T2;       ///< R3: Temporary
  UINT64  T3;       ///< R4: Temporary
  UINT64  T4;       ///< R5: Temporary
  UINT64  T5;       ///< R6: Temporary
  UINT64  T6;       ///< R7: Temporary
  UINT64  T7;       ///< R8: Temporary
  UINT64  S0;       ///< R9: Saved
  UINT64  S1;       ///< R10: Saved
  UINT64  S2;       ///< R11: Saved
  UINT64  S3;       ///< R12: Saved
  UINT64  S4;       ///< R13: Saved
  UINT64  S5;       ///< R14: Saved
  UINT64  Fp;       ///< R15: Frame pointer
  UINT64  A0;       ///< R16: Argument 0
  UINT64  A1;       ///< R17: Argument 1
  UINT64  A2;       ///< R18: Argument 2
  UINT64  A3;       ///< R19: Argument 3
  UINT64  A4;       ///< R20: Argument 4
  UINT64  A5;       ///< R21: Argument 5
  UINT64  T8;       ///< R22: Temporary
  UINT64  T9;       ///< R23: Temporary
  UINT64  T10;      ///< R24: Temporary
  UINT64  T11;      ///< R25: Temporary
  UINT64  Ra;       ///< R26: Return address
  UINT64  T12;      ///< R27: Temporary (PV)
  UINT64  At;       ///< R28: Assembler temp
  UINT64  Gp;       ///< R29: Global pointer
  UINT64  Sp;       ///< R30: Stack pointer
  UINT64  Zero;     ///< R31: Always zero

  //
  // Floating-point registers F0-F31
  //
  UINT64  F0;       ///< FP return value
  UINT64  F1;       ///< FP temporary
  UINT64  F2;       ///< FP saved
  UINT64  F3;       ///< FP saved
  UINT64  F4;       ///< FP saved
  UINT64  F5;       ///< FP saved
  UINT64  F6;       ///< FP saved
  UINT64  F7;       ///< FP saved
  UINT64  F8;       ///< FP saved
  UINT64  F9;       ///< FP saved
  UINT64  F10;      ///< FP temporary
  UINT64  F11;      ///< FP temporary
  UINT64  F12;      ///< FP temporary
  UINT64  F13;      ///< FP temporary
  UINT64  F14;      ///< FP temporary
  UINT64  F15;      ///< FP temporary
  UINT64  F16;      ///< FP argument 0
  UINT64  F17;      ///< FP argument 1
  UINT64  F18;      ///< FP argument 2
  UINT64  F19;      ///< FP argument 3
  UINT64  F20;      ///< FP argument 4
  UINT64  F21;      ///< FP argument 5
  UINT64  F22;      ///< FP temporary
  UINT64  F23;      ///< FP temporary
  UINT64  F24;      ///< FP temporary
  UINT64  F25;      ///< FP temporary
  UINT64  F26;      ///< FP temporary
  UINT64  F27;      ///< FP temporary
  UINT64  F28;      ///< FP temporary
  UINT64  F29;      ///< FP temporary
  UINT64  F30;      ///< FP temporary
  UINT64  Fzero;    ///< F31: Always +0.0

  //
  // Control registers
  //
  UINT64  Pc;       ///< Program counter
  UINT64  Fpcr;     ///< FP control register
  UINT64  SoftFpcr; ///< Software FP control
} ALPHA64_CONTEXT, *PALPHA64_CONTEXT;

///
/// Sunway SW64 Context Structure
/// Based on Alpha-like architecture with 512-bit vector extensions
///
typedef struct ALIGNED_STRUCT(8) _SUNWAY64_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Reserved;

  //
  // Integer registers R0-R31 (Alpha-like naming)
  //
  UINT64  R0;       ///< Return value
  UINT64  R1;       ///< Temporary
  UINT64  R2;       ///< Temporary
  UINT64  R3;       ///< Temporary
  UINT64  R4;       ///< Temporary
  UINT64  R5;       ///< Temporary
  UINT64  R6;       ///< Temporary
  UINT64  R7;       ///< Temporary
  UINT64  R8;       ///< Temporary
  UINT64  R9;       ///< Saved
  UINT64  R10;      ///< Saved
  UINT64  R11;      ///< Saved
  UINT64  R12;      ///< Saved
  UINT64  R13;      ///< Saved
  UINT64  R14;      ///< Saved
  UINT64  R15;      ///< Frame pointer
  UINT64  R16;      ///< Argument 0
  UINT64  R17;      ///< Argument 1
  UINT64  R18;      ///< Argument 2
  UINT64  R19;      ///< Argument 3
  UINT64  R20;      ///< Argument 4
  UINT64  R21;      ///< Argument 5
  UINT64  R22;      ///< Temporary
  UINT64  R23;      ///< Temporary
  UINT64  R24;      ///< Temporary
  UINT64  R25;      ///< Temporary
  UINT64  R26;      ///< Return address
  UINT64  R27;      ///< Temporary
  UINT64  R28;      ///< Assembler temp
  UINT64  R29;      ///< Global pointer
  UINT64  R30;      ///< Stack pointer
  UINT64  R31;      ///< Always zero

  //
  // Floating-point registers F0-F31
  //
  UINT64  FloatReg[32];

  //
  // 512-bit vector registers V0-V31 (CPE cores)
  //
  UINT8   V[32][64];

  //
  // Control registers
  //
  UINT64  Pc;
  UINT64  Fpcr;
} SUNWAY64_CONTEXT, *PSUNWAY64_CONTEXT;

#endif // __CORE_CONTEXT_ALPHA_H__
