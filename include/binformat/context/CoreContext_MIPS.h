/** @file
  MIPS Family Register Context Structures.

  Includes MIPS32, MIPS64, LoongArch, and MIPS extensions (MDMX, MIPS-3D,
  DSP ASE, Loongson extensions).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_MIPS_H__
#define __CORE_CONTEXT_MIPS_H__

//
// MIPS (32-bit) Context Flags
//
#define CONTEXT_MIPS32                  0x02000000
#define CONTEXT_MIPS32_CONTROL          (CONTEXT_MIPS32 | 0x00000001)  ///< PC, HI, LO, Status
#define CONTEXT_MIPS32_INTEGER          (CONTEXT_MIPS32 | 0x00000002)  ///< r0-r31
#define CONTEXT_MIPS32_FLOATING_POINT   (CONTEXT_MIPS32 | 0x00000004)  ///< f0-f31, fcsr
#define CONTEXT_MIPS32_DSP              (CONTEXT_MIPS32 | 0x00000008)  ///< DSP ASE registers
#define CONTEXT_MIPS32_FULL             (CONTEXT_MIPS32_CONTROL | CONTEXT_MIPS32_INTEGER)
#define CONTEXT_MIPS32_ALL              (CONTEXT_MIPS32_FULL | CONTEXT_MIPS32_FLOATING_POINT | CONTEXT_MIPS32_DSP)

//
// MIPS (64-bit) Context Flags
//
#define CONTEXT_MIPS64                  0x04000000
#define CONTEXT_MIPS64_CONTROL          (CONTEXT_MIPS64 | 0x00000001)  ///< PC, HI, LO, Status
#define CONTEXT_MIPS64_INTEGER          (CONTEXT_MIPS64 | 0x00000002)  ///< r0-r31
#define CONTEXT_MIPS64_FLOATING_POINT   (CONTEXT_MIPS64 | 0x00000004)  ///< f0-f31, fcsr
#define CONTEXT_MIPS64_DSP              (CONTEXT_MIPS64 | 0x00000008)  ///< DSP ASE registers
#define CONTEXT_MIPS64_FULL             (CONTEXT_MIPS64_CONTROL | CONTEXT_MIPS64_INTEGER)
#define CONTEXT_MIPS64_ALL              (CONTEXT_MIPS64_FULL | CONTEXT_MIPS64_FLOATING_POINT | CONTEXT_MIPS64_DSP)

//
// MIPS Extension Context Flags
//
#define CONTEXT_MIPS_MDMX               0x00000010  ///< MDMX SIMD extension
#define CONTEXT_MIPS_3D                 0x00000020  ///< MIPS-3D extension
#define CONTEXT_MIPS_DSP                0x00000040  ///< DSP ASE
#define CONTEXT_MIPS_DSP2               0x00000080  ///< DSP ASE Release 2
#define CONTEXT_MIPS_DSP3               0x00000100  ///< DSP ASE Release 3
#define CONTEXT_MIPS_LOONGSON_MMI       0x00000200  ///< Loongson MMI
#define CONTEXT_MIPS_LOONGSON_CAM       0x00000400  ///< Loongson CAM
#define CONTEXT_MIPS_LOONGSON_EXT       0x00000800  ///< Loongson EXT
#define CONTEXT_MIPS_LOONGSON_EXT2      0x00001000  ///< Loongson EXT2

//
// LoongArch Context Flags
//
#define CONTEXT_LOONGARCH32             0x04800000
#define CONTEXT_LOONGARCH32_CONTROL     (CONTEXT_LOONGARCH32 | 0x00000001)
#define CONTEXT_LOONGARCH32_INTEGER     (CONTEXT_LOONGARCH32 | 0x00000002)
#define CONTEXT_LOONGARCH32_FLOATING_POINT (CONTEXT_LOONGARCH32 | 0x00000004)
#define CONTEXT_LOONGARCH32_VECTOR      (CONTEXT_LOONGARCH32 | 0x00000008)
#define CONTEXT_LOONGARCH32_FULL        (CONTEXT_LOONGARCH32_CONTROL | CONTEXT_LOONGARCH32_INTEGER)
#define CONTEXT_LOONGARCH32_ALL         (CONTEXT_LOONGARCH32_FULL | CONTEXT_LOONGARCH32_FLOATING_POINT | CONTEXT_LOONGARCH32_VECTOR)

#define CONTEXT_LOONGARCH64             0x05000000
#define CONTEXT_LOONGARCH64_CONTROL     (CONTEXT_LOONGARCH64 | 0x00000001)
#define CONTEXT_LOONGARCH64_INTEGER     (CONTEXT_LOONGARCH64 | 0x00000002)
#define CONTEXT_LOONGARCH64_FLOATING_POINT (CONTEXT_LOONGARCH64 | 0x00000004)
#define CONTEXT_LOONGARCH64_VECTOR      (CONTEXT_LOONGARCH64 | 0x00000008)
#define CONTEXT_LOONGARCH64_FULL        (CONTEXT_LOONGARCH64_CONTROL | CONTEXT_LOONGARCH64_INTEGER)
#define CONTEXT_LOONGARCH64_ALL         (CONTEXT_LOONGARCH64_FULL | CONTEXT_LOONGARCH64_FLOATING_POINT | CONTEXT_LOONGARCH64_VECTOR)

///
/// MIPS 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _MIPS32_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;

  //
  // Integer registers (R0-R31)
  //
  UINT32  R0;   ///< Zero register (always 0)
  UINT32  R1;   ///< Assembler temporary (at)
  UINT32  R2;   ///< Return value (v0)
  UINT32  R3;   ///< Return value (v1)
  UINT32  R4;   ///< Argument (a0)
  UINT32  R5;   ///< Argument (a1)
  UINT32  R6;   ///< Argument (a2)
  UINT32  R7;   ///< Argument (a3)
  UINT32  R8;   ///< Temporary (t0)
  UINT32  R9;   ///< Temporary (t1)
  UINT32  R10;  ///< Temporary (t2)
  UINT32  R11;  ///< Temporary (t3)
  UINT32  R12;  ///< Temporary (t4)
  UINT32  R13;  ///< Temporary (t5)
  UINT32  R14;  ///< Temporary (t6)
  UINT32  R15;  ///< Temporary (t7)
  UINT32  R16;  ///< Saved register (s0)
  UINT32  R17;  ///< Saved register (s1)
  UINT32  R18;  ///< Saved register (s2)
  UINT32  R19;  ///< Saved register (s3)
  UINT32  R20;  ///< Saved register (s4)
  UINT32  R21;  ///< Saved register (s5)
  UINT32  R22;  ///< Saved register (s6)
  UINT32  R23;  ///< Saved register (s7)
  UINT32  R24;  ///< Temporary (t8)
  UINT32  R25;  ///< Temporary (t9)
  UINT32  R26;  ///< Kernel reserved (k0)
  UINT32  R27;  ///< Kernel reserved (k1)
  UINT32  R28;  ///< Global pointer (gp)
  UINT32  R29;  ///< Stack pointer (sp)
  UINT32  R30;  ///< Frame pointer (fp/s8)
  UINT32  R31;  ///< Return address (ra)

  //
  // Special registers
  //
  UINT32  Pc;     ///< Program counter
  UINT32  Hi;     ///< Multiply/divide HI result
  UINT32  Lo;     ///< Multiply/divide LO result
  UINT32  Status; ///< Coprocessor 0 Status register

  //
  // Floating-point registers (F0-F31)
  //
  UINT32  FloatReg[32];

  //
  // Floating-point control and status register
  //
  UINT32  Fcsr;
  UINT32  Fir;

  //
  // DSP ASE registers (optional)
  //
  UINT32  DspControl;
  UINT32  HI1;
  UINT32  LO1;
  UINT32  HI2;
  UINT32  LO2;
  UINT32  HI3;
  UINT32  LO3;
} MIPS32_CONTEXT, *PMIPS32_CONTEXT;

///
/// MIPS 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _MIPS64_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;
  UINT32  Padding;

  //
  // Integer registers (R0-R31)
  //
  UINT64  R0;   ///< Zero register (always 0)
  UINT64  R1;   ///< Assembler temporary (at)
  UINT64  R2;   ///< Return value (v0)
  UINT64  R3;   ///< Return value (v1)
  UINT64  R4;   ///< Argument (a0)
  UINT64  R5;   ///< Argument (a1)
  UINT64  R6;   ///< Argument (a2)
  UINT64  R7;   ///< Argument (a3)
  UINT64  R8;   ///< Argument (a4)
  UINT64  R9;   ///< Argument (a5)
  UINT64  R10;  ///< Argument (a6)
  UINT64  R11;  ///< Argument (a7)
  UINT64  R12;  ///< Temporary (t0)
  UINT64  R13;  ///< Temporary (t1)
  UINT64  R14;  ///< Temporary (t2)
  UINT64  R15;  ///< Temporary (t3)
  UINT64  R16;  ///< Saved register (s0)
  UINT64  R17;  ///< Saved register (s1)
  UINT64  R18;  ///< Saved register (s2)
  UINT64  R19;  ///< Saved register (s3)
  UINT64  R20;  ///< Saved register (s4)
  UINT64  R21;  ///< Saved register (s5)
  UINT64  R22;  ///< Saved register (s6)
  UINT64  R23;  ///< Saved register (s7)
  UINT64  R24;  ///< Temporary (t8)
  UINT64  R25;  ///< Temporary (t9)
  UINT64  R26;  ///< Kernel reserved (k0)
  UINT64  R27;  ///< Kernel reserved (k1)
  UINT64  R28;  ///< Global pointer (gp)
  UINT64  R29;  ///< Stack pointer (sp)
  UINT64  R30;  ///< Frame pointer (fp/s8)
  UINT64  R31;  ///< Return address (ra)

  //
  // Special registers
  //
  UINT64  Pc;     ///< Program counter
  UINT64  Hi;     ///< Multiply/divide HI result
  UINT64  Lo;     ///< Multiply/divide LO result
  UINT64  Status; ///< Coprocessor 0 Status register

  //
  // Floating-point registers (F0-F31)
  //
  UINT64  FloatReg[32];

  //
  // Floating-point control and status register
  //
  UINT32  Fcsr;
  UINT32  Fir;

  //
  // DSP ASE registers (optional)
  //
  UINT32  DspControl;
  UINT32  Padding2;
  UINT64  HI1;
  UINT64  LO1;
  UINT64  HI2;
  UINT64  LO2;
  UINT64  HI3;
  UINT64  LO3;
} MIPS64_CONTEXT, *PMIPS64_CONTEXT;

///
/// MIPS MDMX (MIPS Digital Media eXtension) Context
/// Uses FP register file for 64-bit SIMD operations
/// Deprecated in Release 5, removed in Release 6
///
typedef struct _MDMX_CONTEXT {
  //
  // OB format (Octet Byte) - 8×8-bit operations
  // QH format (Quad Halfword) - 4×16-bit operations
  // Uses existing FP registers, no additional state
  //
  UINT32  MdmxControl;  ///< MDMX control/status
} MDMX_CONTEXT, *PMDMX_CONTEXT;

///
/// MIPS-3D Extension Context
/// Floating-point SIMD for 3D graphics
///
typedef struct _MIPS3D_CONTEXT {
  UINT32  Mips3dControl;  ///< MIPS-3D control/status
} MIPS3D_CONTEXT, *PMIPS3D_CONTEXT;

///
/// MIPS DSP ASE Context (Release 1, 2, 3)
///
typedef struct ALIGNED_STRUCT(8) _MIPS_DSP_CONTEXT {
  //
  // DSPControl register
  //
  UINT32  DspControl;
  UINT32  Reserved;

  //
  // Six 64-bit accumulators (ac0-ac5) for DSP operations
  //
  UINT64  Ac0;
  UINT64  Ac1;
  UINT64  Ac2;
  UINT64  Ac3;
  UINT64  Ac4;  ///< DSP R2+
  UINT64  Ac5;  ///< DSP R2+
} MIPS_DSP_CONTEXT, *PMIPS_DSP_CONTEXT;

///
/// Loongson MMI (MultiMedia Instructions) Context
///
typedef struct _LOONGSON_MMI_CONTEXT {
  UINT32  MmiControl;
  UINT32  Reserved;
} LOONGSON_MMI_CONTEXT, *PLOONGSON_MMI_CONTEXT;

///
/// Loongson CAM (Content Addressable Memory) Context
///
typedef struct _LOONGSON_CAM_CONTEXT {
  UINT32  CamControl;
  UINT32  Reserved;
} LOONGSON_CAM_CONTEXT, *PLOONGSON_CAM_CONTEXT;

///
/// Loongson EXT/EXT2 Context
///
typedef struct _LOONGSON_EXT_CONTEXT {
  UINT32  ExtControl;
  UINT32  Reserved;
} LOONGSON_EXT_CONTEXT, *PLOONGSON_EXT_CONTEXT;

///
/// LoongArch 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _LOONGARCH32_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;

  //
  // General purpose registers (R0-R31)
  //
  UINT32  R0;   ///< Zero register (always 0)
  UINT32  R1;   ///< Return address (ra)
  UINT32  R2;   ///< Thread pointer (tp)
  UINT32  R3;   ///< Stack pointer (sp)
  UINT32  R4;   ///< Argument/return value (a0)
  UINT32  R5;   ///< Argument/return value (a1)
  UINT32  R6;   ///< Argument (a2)
  UINT32  R7;   ///< Argument (a3)
  UINT32  R8;   ///< Argument (a4)
  UINT32  R9;   ///< Argument (a5)
  UINT32  R10;  ///< Argument (a6)
  UINT32  R11;  ///< Argument (a7)
  UINT32  R12;  ///< Temporary (t0)
  UINT32  R13;  ///< Temporary (t1)
  UINT32  R14;  ///< Temporary (t2)
  UINT32  R15;  ///< Temporary (t3)
  UINT32  R16;  ///< Temporary (t4)
  UINT32  R17;  ///< Temporary (t5)
  UINT32  R18;  ///< Temporary (t6)
  UINT32  R19;  ///< Temporary (t7)
  UINT32  R20;  ///< Temporary (t8)
  UINT32  R21;  ///< Reserved
  UINT32  R22;  ///< Frame pointer (fp/s9)
  UINT32  R23;  ///< Saved register (s0)
  UINT32  R24;  ///< Saved register (s1)
  UINT32  R25;  ///< Saved register (s2)
  UINT32  R26;  ///< Saved register (s3)
  UINT32  R27;  ///< Saved register (s4)
  UINT32  R28;  ///< Saved register (s5)
  UINT32  R29;  ///< Saved register (s6)
  UINT32  R30;  ///< Saved register (s7)
  UINT32  R31;  ///< Saved register (s8)

  //
  // Program counter
  //
  UINT32  Pc;

  //
  // Floating-point registers (F0-F31)
  //
  UINT32  FloatReg[32];

  //
  // Floating-point control and status register
  //
  UINT32  Fcsr;

  //
  // LSX/LASX vector registers (V0-V31)
  //
  UINT32  VectorReg[32][4];  ///< 128-bit LSX vectors
} LOONGARCH32_CONTEXT, *PLOONGARCH32_CONTEXT;

///
/// LoongArch 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _LOONGARCH64_CONTEXT {
  //
  // Context flags
  //
  UINT32  ContextFlags;
  UINT32  Padding;

  //
  // General purpose registers (R0-R31)
  //
  UINT64  R0;   ///< Zero register (always 0)
  UINT64  R1;   ///< Return address (ra)
  UINT64  R2;   ///< Thread pointer (tp)
  UINT64  R3;   ///< Stack pointer (sp)
  UINT64  R4;   ///< Argument/return value (a0)
  UINT64  R5;   ///< Argument/return value (a1)
  UINT64  R6;   ///< Argument (a2)
  UINT64  R7;   ///< Argument (a3)
  UINT64  R8;   ///< Argument (a4)
  UINT64  R9;   ///< Argument (a5)
  UINT64  R10;  ///< Argument (a6)
  UINT64  R11;  ///< Argument (a7)
  UINT64  R12;  ///< Temporary (t0)
  UINT64  R13;  ///< Temporary (t1)
  UINT64  R14;  ///< Temporary (t2)
  UINT64  R15;  ///< Temporary (t3)
  UINT64  R16;  ///< Temporary (t4)
  UINT64  R17;  ///< Temporary (t5)
  UINT64  R18;  ///< Temporary (t6)
  UINT64  R19;  ///< Temporary (t7)
  UINT64  R20;  ///< Temporary (t8)
  UINT64  R21;  ///< Reserved
  UINT64  R22;  ///< Frame pointer (fp/s9)
  UINT64  R23;  ///< Saved register (s0)
  UINT64  R24;  ///< Saved register (s1)
  UINT64  R25;  ///< Saved register (s2)
  UINT64  R26;  ///< Saved register (s3)
  UINT64  R27;  ///< Saved register (s4)
  UINT64  R28;  ///< Saved register (s5)
  UINT64  R29;  ///< Saved register (s6)
  UINT64  R30;  ///< Saved register (s7)
  UINT64  R31;  ///< Saved register (s8)

  //
  // Program counter
  //
  UINT64  Pc;

  //
  // Floating-point registers (F0-F31)
  //
  UINT64  FloatReg[32];

  //
  // Floating-point control and status register
  //
  UINT32  Fcsr;
  UINT32  Padding2;

  //
  // LSX/LASX vector registers (V0-V31)
  //
  UINT64  VectorReg[32][4];  ///< 256-bit LASX vectors
} LOONGARCH64_CONTEXT, *PLOONGARCH64_CONTEXT;

#endif // __CORE_CONTEXT_MIPS_H__
