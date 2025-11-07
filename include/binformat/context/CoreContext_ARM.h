/** @file
  ARM Family Register Context Structures.

  Includes ARM (32-bit), ARM64 (AArch64), and extensions (NEON, VFP, SVE,
  SME, Wireless MMX).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_ARM_H__
#define __CORE_CONTEXT_ARM_H__

//
// ARM (32-bit) Context Flags
//
#define CONTEXT_ARM                     0x00200000
#define CONTEXT_ARM_CONTROL             (CONTEXT_ARM | 0x00000001)  ///< SP, LR, PC, CPSR
#define CONTEXT_ARM_INTEGER             (CONTEXT_ARM | 0x00000002)  ///< R0-R12
#define CONTEXT_ARM_FLOATING_POINT      (CONTEXT_ARM | 0x00000004)  ///< VFP/NEON
#define CONTEXT_ARM_DEBUG_REGISTERS     (CONTEXT_ARM | 0x00000008)  ///< Breakpoint/watchpoint
#define CONTEXT_ARM_FULL                (CONTEXT_ARM_CONTROL | CONTEXT_ARM_INTEGER)
#define CONTEXT_ARM_ALL                 (CONTEXT_ARM_FULL | CONTEXT_ARM_FLOATING_POINT | CONTEXT_ARM_DEBUG_REGISTERS)

//
// ARM64 (AArch64) Context Flags
//
#define CONTEXT_ARM64                   0x00400000
#define CONTEXT_ARM64_CONTROL           (CONTEXT_ARM64 | 0x00000001)  ///< SP, PC, CPSR
#define CONTEXT_ARM64_INTEGER           (CONTEXT_ARM64 | 0x00000002)  ///< X0-X30
#define CONTEXT_ARM64_FLOATING_POINT    (CONTEXT_ARM64 | 0x00000004)  ///< V0-V31, FPCR, FPSR
#define CONTEXT_ARM64_DEBUG_REGISTERS   (CONTEXT_ARM64 | 0x00000008)  ///< Breakpoint/watchpoint
#define CONTEXT_ARM64_FULL              (CONTEXT_ARM64_CONTROL | CONTEXT_ARM64_INTEGER)
#define CONTEXT_ARM64_ALL               (CONTEXT_ARM64_FULL | CONTEXT_ARM64_FLOATING_POINT | CONTEXT_ARM64_DEBUG_REGISTERS)

//
// ARM Extension Context Flags (32-bit)
//
#define CONTEXT_ARM_FPA                 0x00000010  ///< Floating Point Accelerator
#define CONTEXT_ARM_VFPv1               0x00000020  ///< VFPv1
#define CONTEXT_ARM_VFPv2               0x00000040  ///< VFPv2
#define CONTEXT_ARM_VFPv3               0x00000080  ///< VFPv3
#define CONTEXT_ARM_VFPv4               0x00000100  ///< VFPv4
#define CONTEXT_ARM_WMMX                0x00000200  ///< Intel Wireless MMX

//
// ARM64 Extension Context Flags
//
#define CONTEXT_ARM64_SVE               0x00000010  ///< SVE vector/predicate registers
#define CONTEXT_ARM64_SME               0x00000020  ///< SME matrix array
#define CONTEXT_ARM64_SME2              0x00000040  ///< SME2 (includes ZT0)

///
/// Helper structure for 128-bit NEON registers (ARM)
///
typedef union _ARM_NEON128 {
  struct {
    UINT64  Low;
    INT64   High;
  };
  FLOAT64   D[2];
  UINT32    S[4];
  UINT16    H[8];
  UINT8     B[16];
} ARM_NEON128, *PARM_NEON128;

///
/// Helper structure for 128-bit NEON registers (ARM64)
///
typedef union ALIGNED_STRUCT(16) _ARM64_NT_NEON128 {
  struct {
    UINT64  Low;
    INT64   High;
  };
  FLOAT64   D[2];
  FLOAT32   S[4];
  UINT16    H[8];
  UINT8     B[16];
} ARM64_NT_NEON128, *PARM64_NT_NEON128;

///
/// ARM (32-bit) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _ARM_CONTEXT {
  //
  // Context flags
  //
  UINT32         ContextFlags;

  //
  // Integer registers
  //
  UINT32         R0;
  UINT32         R1;
  UINT32         R2;
  UINT32         R3;
  UINT32         R4;
  UINT32         R5;
  UINT32         R6;
  UINT32         R7;
  UINT32         R8;
  UINT32         R9;
  UINT32         R10;
  UINT32         R11;
  UINT32         R12;

  //
  // Control registers
  //
  UINT32         Sp;   ///< Stack pointer (R13)
  UINT32         Lr;   ///< Link register (R14)
  UINT32         Pc;   ///< Program counter (R15)
  UINT32         Cpsr; ///< Current program status register

  //
  // Floating-point status and control
  //
  UINT32         Fpscr;
  UINT32         Padding;

  //
  // VFP/NEON registers (can be accessed as Q, D, or S registers)
  //
  union {
    ARM_NEON128  Q[16];   ///< 128-bit NEON registers
    UINT64       D[32];   ///< 64-bit VFP double registers
    UINT32       S[32];   ///< 32-bit VFP single registers
  };

  //
  // Debug registers
  //
  UINT32         Bvr[8];   ///< Breakpoint value registers
  UINT32         Bcr[8];   ///< Breakpoint control registers
  UINT32         Wvr[1];   ///< Watchpoint value register
  UINT32         Wcr[1];   ///< Watchpoint control register
  UINT32         Padding2[2];
} ARM_CONTEXT, *PARM_CONTEXT;

///
/// ARM64 (AArch64) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(16) _ARM64_NT_CONTEXT {
  //
  // Context flags
  //
  UINT32              ContextFlags;
  UINT32              Cpsr; ///< Current program status register

  //
  // Integer registers (X0-X30)
  //
  UINT64              X0;
  UINT64              X1;
  UINT64              X2;
  UINT64              X3;
  UINT64              X4;
  UINT64              X5;
  UINT64              X6;
  UINT64              X7;
  UINT64              X8;
  UINT64              X9;
  UINT64              X10;
  UINT64              X11;
  UINT64              X12;
  UINT64              X13;
  UINT64              X14;
  UINT64              X15;
  UINT64              X16;
  UINT64              X17;
  UINT64              X18;
  UINT64              X19;
  UINT64              X20;
  UINT64              X21;
  UINT64              X22;
  UINT64              X23;
  UINT64              X24;
  UINT64              X25;
  UINT64              X26;
  UINT64              X27;
  UINT64              X28;
  UINT64              X29; ///< Frame pointer
  UINT64              X30; ///< Link register

  //
  // Control registers
  //
  UINT64              Sp;  ///< Stack pointer
  UINT64              Pc;  ///< Program counter

  //
  // NEON/FP registers (V0-V31, 128-bit each)
  //
  ARM64_NT_NEON128    V[32];

  //
  // Floating-point control and status
  //
  UINT32              Fpcr; ///< Floating-point control register
  UINT32              Fpsr; ///< Floating-point status register

  //
  // Debug registers
  //
  UINT32              Bcr[8];   ///< Breakpoint control registers
  UINT64              Bvr[8];   ///< Breakpoint value registers
  UINT32              Wcr[2];   ///< Watchpoint control registers
  UINT64              Wvr[2];   ///< Watchpoint value registers
} ARM64_NT_CONTEXT, *PARM64_NT_CONTEXT;

///
/// ARM FPA (Floating Point Accelerator) - original ARM FP coprocessor
///
typedef struct _ARM_FPA_CONTEXT {
  UINT8   F0[12];   ///< 96-bit extended precision
  UINT8   F1[12];
  UINT8   F2[12];
  UINT8   F3[12];
  UINT8   F4[12];
  UINT8   F5[12];
  UINT8   F6[12];
  UINT8   F7[12];
  UINT32  Fpsr;     ///< FP status register
  UINT32  Fpcr;     ///< FP control register
} ARM_FPA_CONTEXT, *PARM_FPA_CONTEXT;

///
/// ARM VFPv1/v2 Context - 16 double-precision registers
///
typedef struct ALIGNED_STRUCT(8) _ARM_VFPV2_CONTEXT {
  UINT64  D[16];    ///< D0-D15 (or S0-S31 as 32 singles)
  UINT32  Fpscr;    ///< FP status and control register
  UINT32  Fpexc;    ///< FP exception register
} ARM_VFPV2_CONTEXT, *PARM_VFPV2_CONTEXT;

///
/// ARM VFPv3/v4 Context - 32 double-precision registers
///
typedef struct ALIGNED_STRUCT(8) _ARM_VFPV3_CONTEXT {
  UINT64  D[32];    ///< D0-D31 (or S0-S63 as 64 singles)
  UINT32  Fpscr;    ///< FP status and control register
  UINT32  Fpexc;    ///< FP exception register
} ARM_VFPV3_CONTEXT, *PARM_VFPV3_CONTEXT;

///
/// Intel Wireless MMX (WMMX) Context
/// Used in XScale processors
///
typedef struct ALIGNED_STRUCT(8) _ARM_WMMX_CONTEXT {
  UINT64  Wr[16];       ///< wR0-wR15: 64-bit multimedia registers
  UINT32  Wcgr[4];      ///< wCGR0-wCGR3: control/general registers
  UINT32  Wcssf;        ///< wCSSF: SIMD status flags
  UINT32  Wcasf;        ///< wCASF: accumulator status flags
  UINT32  Wcid;         ///< wCID: coprocessor ID
  UINT32  Wcon;         ///< wCon: control register
} ARM_WMMX_CONTEXT, *PARM_WMMX_CONTEXT;

///
/// ARM SVE (Scalable Vector Extension)
/// Vector length is implementation-defined from 128 to 2048 bits (16 to 256 bytes)
/// This structure uses maximum size; actual size determined by VL register
///
typedef struct ALIGNED_STRUCT(16) _SVE_CONTEXT {
  //
  // Vector length in bytes (16, 32, 64, 128, or 256)
  //
  UINT16  Vl;           ///< Vector length in bytes
  UINT16  Reserved;
  UINT32  Padding;

  //
  // 32 scalable vector registers Z0-Z31 (max 256 bytes each)
  //
  UINT8   Z[32][256];

  //
  // 16 predicate registers P0-P15 (max 32 bytes each, 1 bit per vector byte)
  //
  UINT8   P[16][32];

  //
  // First-Fault Register (FFR) - special predicate for fault-tolerant loads
  //
  UINT8   Ffr[32];
} SVE_CONTEXT, *PSVE_CONTEXT;

///
/// ARM SME (Scalable Matrix Extension)
/// ZA array size is (SVL/8) × (SVL/8) bytes, max 64×64 = 4096 bytes
///
typedef struct ALIGNED_STRUCT(16) _SME_CONTEXT {
  //
  // Streaming vector length in bytes
  //
  UINT16  Svl;          ///< Streaming vector length in bytes
  UINT16  Reserved;

  //
  // PSTATE.SM and PSTATE.ZA bits
  //
  UINT8   PstateSm;     ///< Streaming mode active
  UINT8   PstateZa;     ///< ZA storage active
  UINT16  Padding;

  //
  // ZA matrix array register (max 64×64 = 4096 bytes)
  //
  UINT8   Za[4096];
} SME_CONTEXT, *PSME_CONTEXT;

///
/// ARM SME2 Extensions - ZT0 lookup table register
///
typedef struct ALIGNED_STRUCT(16) _SME2_CONTEXT {
  //
  // ZT0: 512-bit lookup table register (16 entries × 32 bits)
  //
  UINT8   Zt0[64];
} SME2_CONTEXT, *PSME2_CONTEXT;

#endif // __CORE_CONTEXT_ARM_H__
