/** @file
  RISC-V Register Context Structures.

  Includes RV32 and RV64 with F/D/V extensions.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_RISCV_H__
#define __CORE_CONTEXT_RISCV_H__

//
// RISC-V (32-bit) Context Flags
//
#define CONTEXT_RISCV32                 0x00800000
#define CONTEXT_RISCV32_CONTROL         (CONTEXT_RISCV32 | 0x00000001)  ///< PC, SP, CSRs
#define CONTEXT_RISCV32_INTEGER         (CONTEXT_RISCV32 | 0x00000002)  ///< x0-x31
#define CONTEXT_RISCV32_FLOATING_POINT  (CONTEXT_RISCV32 | 0x00000004)  ///< f0-f31, fcsr
#define CONTEXT_RISCV32_VECTOR          (CONTEXT_RISCV32 | 0x00000008)  ///< v0-v31 (RVV)
#define CONTEXT_RISCV32_FULL            (CONTEXT_RISCV32_CONTROL | CONTEXT_RISCV32_INTEGER)
#define CONTEXT_RISCV32_ALL             (CONTEXT_RISCV32_FULL | CONTEXT_RISCV32_FLOATING_POINT | CONTEXT_RISCV32_VECTOR)

//
// RISC-V (64-bit) Context Flags
//
#define CONTEXT_RISCV64                 0x01000000
#define CONTEXT_RISCV64_CONTROL         (CONTEXT_RISCV64 | 0x00000001)  ///< PC, SP, CSRs
#define CONTEXT_RISCV64_INTEGER         (CONTEXT_RISCV64 | 0x00000002)  ///< x0-x31
#define CONTEXT_RISCV64_FLOATING_POINT  (CONTEXT_RISCV64 | 0x00000004)  ///< f0-f31, fcsr
#define CONTEXT_RISCV64_VECTOR          (CONTEXT_RISCV64 | 0x00000008)  ///< v0-v31 (RVV)
#define CONTEXT_RISCV64_FULL            (CONTEXT_RISCV64_CONTROL | CONTEXT_RISCV64_INTEGER)
#define CONTEXT_RISCV64_ALL             (CONTEXT_RISCV64_FULL | CONTEXT_RISCV64_FLOATING_POINT | CONTEXT_RISCV64_VECTOR)

///
/// RISC-V 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _RISCV32_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;

  //
  // Integer registers (x0-x31)
  //
  UINT32  X0;   ///< Zero register (always 0)
  UINT32  X1;   ///< Return address (ra)
  UINT32  X2;   ///< Stack pointer (sp)
  UINT32  X3;   ///< Global pointer (gp)
  UINT32  X4;   ///< Thread pointer (tp)
  UINT32  X5;   ///< Temporary (t0)
  UINT32  X6;   ///< Temporary (t1)
  UINT32  X7;   ///< Temporary (t2)
  UINT32  X8;   ///< Saved register / frame pointer (s0/fp)
  UINT32  X9;   ///< Saved register (s1)
  UINT32  X10;  ///< Function argument / return value (a0)
  UINT32  X11;  ///< Function argument / return value (a1)
  UINT32  X12;  ///< Function argument (a2)
  UINT32  X13;  ///< Function argument (a3)
  UINT32  X14;  ///< Function argument (a4)
  UINT32  X15;  ///< Function argument (a5)
  UINT32  X16;  ///< Function argument (a6)
  UINT32  X17;  ///< Function argument (a7)
  UINT32  X18;  ///< Saved register (s2)
  UINT32  X19;  ///< Saved register (s3)
  UINT32  X20;  ///< Saved register (s4)
  UINT32  X21;  ///< Saved register (s5)
  UINT32  X22;  ///< Saved register (s6)
  UINT32  X23;  ///< Saved register (s7)
  UINT32  X24;  ///< Saved register (s8)
  UINT32  X25;  ///< Saved register (s9)
  UINT32  X26;  ///< Saved register (s10)
  UINT32  X27;  ///< Saved register (s11)
  UINT32  X28;  ///< Temporary (t3)
  UINT32  X29;  ///< Temporary (t4)
  UINT32  X30;  ///< Temporary (t5)
  UINT32  X31;  ///< Temporary (t6)

  //
  // Program counter
  //
  UINT32  Pc;

  //
  // Floating-point registers (F0-F31) - RV32F/D
  //
  UINT32  FloatReg[32];

  //
  // Floating-point control and status register
  //
  UINT32  Fcsr;

  //
  // Vector registers (V0-V31) - RVV extension (placeholder for VLEN)
  //
  UINT32  VectorRegister[32][4];  ///< Assumes 128-bit vectors (VLEN=128)
  UINT32  Vl;                      ///< Vector length
  UINT32  Vtype;                   ///< Vector type register
} RISCV32_CONTEXT, *PRISCV32_CONTEXT;

///
/// RISC-V 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _RISCV64_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;
  UINT32  Padding;

  //
  // Integer registers (x0-x31)
  //
  UINT64  X0;   ///< Zero register (always 0)
  UINT64  X1;   ///< Return address (ra)
  UINT64  X2;   ///< Stack pointer (sp)
  UINT64  X3;   ///< Global pointer (gp)
  UINT64  X4;   ///< Thread pointer (tp)
  UINT64  X5;   ///< Temporary (t0)
  UINT64  X6;   ///< Temporary (t1)
  UINT64  X7;   ///< Temporary (t2)
  UINT64  X8;   ///< Saved register / frame pointer (s0/fp)
  UINT64  X9;   ///< Saved register (s1)
  UINT64  X10;  ///< Function argument / return value (a0)
  UINT64  X11;  ///< Function argument / return value (a1)
  UINT64  X12;  ///< Function argument (a2)
  UINT64  X13;  ///< Function argument (a3)
  UINT64  X14;  ///< Function argument (a4)
  UINT64  X15;  ///< Function argument (a5)
  UINT64  X16;  ///< Function argument (a6)
  UINT64  X17;  ///< Function argument (a7)
  UINT64  X18;  ///< Saved register (s2)
  UINT64  X19;  ///< Saved register (s3)
  UINT64  X20;  ///< Saved register (s4)
  UINT64  X21;  ///< Saved register (s5)
  UINT64  X22;  ///< Saved register (s6)
  UINT64  X23;  ///< Saved register (s7)
  UINT64  X24;  ///< Saved register (s8)
  UINT64  X25;  ///< Saved register (s9)
  UINT64  X26;  ///< Saved register (s10)
  UINT64  X27;  ///< Saved register (s11)
  UINT64  X28;  ///< Temporary (t3)
  UINT64  X29;  ///< Temporary (t4)
  UINT64  X30;  ///< Temporary (t5)
  UINT64  X31;  ///< Temporary (t6)

  //
  // Program counter
  //
  UINT64  Pc;

  //
  // Floating-point registers (F0-F31) - RV64F/D
  //
  UINT64  FloatReg[32];

  //
  // Floating-point control and status register
  //
  UINT32  Fcsr;
  UINT32  Padding2;

  //
  // Vector registers (V0-V31) - RVV extension (placeholder for VLEN)
  //
  UINT64  VectorRegister[32][4];  ///< Assumes 256-bit vectors (VLEN=256)
  UINT64  Vl;                      ///< Vector length
  UINT64  Vtype;                   ///< Vector type register
} RISCV64_CONTEXT, *PRISCV64_CONTEXT;

#endif // __CORE_CONTEXT_RISCV_H__
