/** @file
  Universal Core Dump Register Context Structures.

  This header defines NT-style CONTEXT structures for all architectures,
  providing a universal format for register contexts from core dumps
  (ELF, Mach-O, PE minidump, etc.).

  For Windows-supported architectures (x86, x64, ARM, ARM64), we use the
  standard NT CONTEXT structures. For other architectures, we define
  similar structures following the same naming and layout patterns.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_H__
#define __CORE_CONTEXT_H__

#include "BinFormat.h"

///
/// Alignment macro for structures
/// For GCC, use after struct/union keyword: typedef struct ALIGNED_STRUCT(16) _Name { ... } Name;
/// For MSVC, use before struct: typedef ALIGNED_STRUCT(16) struct _Name { ... } Name;
///
#if defined(__GNUC__)
  #define ALIGNED_STRUCT(x)  __attribute__((aligned(x)))
  #define DECLSPEC_ALIGN(x)  __attribute__((aligned(x)))
#elif defined(_MSC_VER)
  #define ALIGNED_STRUCT(x)  __declspec(align(x))
  #define DECLSPEC_ALIGN(x)  __declspec(align(x))
#else
  #define ALIGNED_STRUCT(x)
  #define DECLSPEC_ALIGN(x)
#endif

///
/// Context flag bit definitions for specifying which register groups are valid
///

//
// x86 (i386) Context Flags
//
#define CONTEXT_I386                    0x00010000
#define CONTEXT_I386_CONTROL            (CONTEXT_I386 | 0x00000001)  ///< SS:SP, CS:IP, FLAGS, BP
#define CONTEXT_I386_INTEGER            (CONTEXT_I386 | 0x00000002)  ///< AX, BX, CX, DX, SI, DI
#define CONTEXT_I386_SEGMENTS           (CONTEXT_I386 | 0x00000004)  ///< DS, ES, FS, GS
#define CONTEXT_I386_FLOATING_POINT     (CONTEXT_I386 | 0x00000008)  ///< 387 state
#define CONTEXT_I386_DEBUG_REGISTERS    (CONTEXT_I386 | 0x00000010)  ///< DB 0-3,6,7
#define CONTEXT_I386_EXTENDED_REGISTERS (CONTEXT_I386 | 0x00000020)  ///< SSE registers
#define CONTEXT_I386_XSTATE             (CONTEXT_I386 | 0x00000040)  ///< AVX/AVX-512 state
#define CONTEXT_I386_FULL               (CONTEXT_I386_CONTROL | CONTEXT_I386_INTEGER | CONTEXT_I386_SEGMENTS)
#define CONTEXT_I386_ALL                (CONTEXT_I386_FULL | CONTEXT_I386_FLOATING_POINT | \
                                         CONTEXT_I386_DEBUG_REGISTERS | CONTEXT_I386_EXTENDED_REGISTERS)

//
// AMD64 (x64) Context Flags
//
#define CONTEXT_AMD64                    0x00100000
#define CONTEXT_AMD64_CONTROL            (CONTEXT_AMD64 | 0x00000001)  ///< SS, RSP, CS, RIP, RFLAGS
#define CONTEXT_AMD64_INTEGER            (CONTEXT_AMD64 | 0x00000002)  ///< RAX-R15
#define CONTEXT_AMD64_SEGMENTS           (CONTEXT_AMD64 | 0x00000004)  ///< DS, ES, FS, GS
#define CONTEXT_AMD64_FLOATING_POINT     (CONTEXT_AMD64 | 0x00000008)  ///< XMM0-XMM15
#define CONTEXT_AMD64_DEBUG_REGISTERS    (CONTEXT_AMD64 | 0x00000010)  ///< DR0-DR7
#define CONTEXT_AMD64_XSTATE             (CONTEXT_AMD64 | 0x00000040)  ///< AVX/AVX-512 state
#define CONTEXT_AMD64_FULL               (CONTEXT_AMD64_CONTROL | CONTEXT_AMD64_INTEGER | CONTEXT_AMD64_SEGMENTS)
#define CONTEXT_AMD64_ALL                (CONTEXT_AMD64_FULL | CONTEXT_AMD64_FLOATING_POINT | CONTEXT_AMD64_DEBUG_REGISTERS)

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
/// Helper structure for 128-bit values (XMM, vector registers)
///
typedef struct _M128A {
  UINT64  Low;
  INT64   High;
} M128A, *PM128A;

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
/// x86 (i386) Floating-point save area
///
typedef struct _I386_FLOATING_SAVE_AREA {
  UINT32  ControlWord;
  UINT32  StatusWord;
  UINT32  TagWord;
  UINT32  ErrorOffset;
  UINT32  ErrorSelector;
  UINT32  DataOffset;
  UINT32  DataSelector;
  UINT8   RegisterArea[80];
  UINT32  Cr0NpxState;
} I386_FLOATING_SAVE_AREA, *PI386_FLOATING_SAVE_AREA;

///
/// AMD64 (x64) XMM save area
///
typedef struct ALIGNED_STRUCT(16) _XMM_SAVE_AREA32 {
  UINT16   ControlWord;
  UINT16   StatusWord;
  UINT8    TagWord;
  UINT8    Reserved1;
  UINT16   ErrorOpcode;
  UINT32   ErrorOffset;
  UINT16   ErrorSelector;
  UINT16   Reserved2;
  UINT32   DataOffset;
  UINT16   DataSelector;
  UINT16   Reserved3;
  UINT32   MxCsr;
  UINT32   MxCsr_Mask;
  M128A    FloatRegisters[8];
  M128A    XmmRegisters[16];
  UINT8    Reserved4[96];
} XMM_SAVE_AREA32, *PXMM_SAVE_AREA32;

///
/// x86 (i386) CONTEXT structure
///
typedef struct _I386_CONTEXT {
  //
  // Context flags specifying which register groups are valid
  //
  UINT32                   ContextFlags;

  //
  // Debug registers
  //
  UINT32                   Dr0;
  UINT32                   Dr1;
  UINT32                   Dr2;
  UINT32                   Dr3;
  UINT32                   Dr6;
  UINT32                   Dr7;

  //
  // Floating-point state
  //
  I386_FLOATING_SAVE_AREA  FloatSave;

  //
  // Segment registers
  //
  UINT32                   SegGs;
  UINT32                   SegFs;
  UINT32                   SegEs;
  UINT32                   SegDs;

  //
  // Integer registers
  //
  UINT32                   Edi;
  UINT32                   Esi;
  UINT32                   Ebx;
  UINT32                   Edx;
  UINT32                   Ecx;
  UINT32                   Eax;

  //
  // Control registers
  //
  UINT32                   Ebp;
  UINT32                   Eip;
  UINT32                   SegCs;
  UINT32                   EFlags;
  UINT32                   Esp;
  UINT32                   SegSs;

  //
  // Extended registers (SSE)
  //
  UINT8                    ExtendedRegisters[512];
} I386_CONTEXT, *PI386_CONTEXT;

///
/// AMD64 (x64) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(16) _AMD64_CONTEXT {
  //
  // Parameter home addresses (for non-volatile registers)
  //
  UINT64            P1Home;
  UINT64            P2Home;
  UINT64            P3Home;
  UINT64            P4Home;
  UINT64            P5Home;
  UINT64            P6Home;

  //
  // Context flags and MXCSR
  //
  UINT32            ContextFlags;
  UINT32            MxCsr;

  //
  // Segment registers
  //
  UINT16            SegCs;
  UINT16            SegDs;
  UINT16            SegEs;
  UINT16            SegFs;
  UINT16            SegGs;
  UINT16            SegSs;
  UINT32            EFlags;

  //
  // Debug registers
  //
  UINT64            Dr0;
  UINT64            Dr1;
  UINT64            Dr2;
  UINT64            Dr3;
  UINT64            Dr6;
  UINT64            Dr7;

  //
  // Integer registers
  //
  UINT64            Rax;
  UINT64            Rcx;
  UINT64            Rdx;
  UINT64            Rbx;
  UINT64            Rsp;
  UINT64            Rbp;
  UINT64            Rsi;
  UINT64            Rdi;
  UINT64            R8;
  UINT64            R9;
  UINT64            R10;
  UINT64            R11;
  UINT64            R12;
  UINT64            R13;
  UINT64            R14;
  UINT64            R15;

  //
  // Program counter
  //
  UINT64            Rip;

  //
  // Floating-point and XMM registers
  //
  XMM_SAVE_AREA32   FltSave;

  //
  // Vector registers (AVX)
  //
  M128A             VectorRegister[26];
  UINT64            VectorControl;

  //
  // Debug control registers
  //
  UINT64            DebugControl;
  UINT64            LastBranchToRip;
  UINT64            LastBranchFromRip;
  UINT64            LastExceptionToRip;
  UINT64            LastExceptionFromRip;
} AMD64_CONTEXT, *PAMD64_CONTEXT;

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

#endif // __CORE_CONTEXT_H__
