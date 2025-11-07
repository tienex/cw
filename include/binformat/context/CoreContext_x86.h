/** @file
  x86 Family Register Context Structures.

  Includes x86 (i386), x64 (AMD64), i8086, i80286, and modern extensions
  (APX, AMX).

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_X86_H__
#define __CORE_CONTEXT_X86_H__

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
// i8086 Context Flags
//
#define CONTEXT_I8086                    0x00008000
#define CONTEXT_I8086_CONTROL            (CONTEXT_I8086 | 0x00000001)  ///< SS:SP, CS:IP, FLAGS
#define CONTEXT_I8086_INTEGER            (CONTEXT_I8086 | 0x00000002)  ///< AX, BX, CX, DX, SI, DI, BP
#define CONTEXT_I8086_SEGMENTS           (CONTEXT_I8086 | 0x00000004)  ///< DS, ES
#define CONTEXT_I8086_FULL               (CONTEXT_I8086_CONTROL | CONTEXT_I8086_INTEGER | CONTEXT_I8086_SEGMENTS)
#define CONTEXT_I8086_ALL                CONTEXT_I8086_FULL

//
// i80286 Context Flags
//
#define CONTEXT_I80286                   0x00004000
#define CONTEXT_I80286_CONTROL           (CONTEXT_I80286 | 0x00000001)  ///< SS:SP, CS:IP, FLAGS, MSW
#define CONTEXT_I80286_INTEGER           (CONTEXT_I80286 | 0x00000002)  ///< AX, BX, CX, DX, SI, DI, BP
#define CONTEXT_I80286_SEGMENTS          (CONTEXT_I80286 | 0x00000004)  ///< DS, ES
#define CONTEXT_I80286_FLOATING_POINT    (CONTEXT_I80286 | 0x00000008)  ///< 80287 state (if present)
#define CONTEXT_I80286_FULL              (CONTEXT_I80286_CONTROL | CONTEXT_I80286_INTEGER | CONTEXT_I80286_SEGMENTS)
#define CONTEXT_I80286_ALL               (CONTEXT_I80286_FULL | CONTEXT_I80286_FLOATING_POINT)

//
// x86/x64 Extension Context Flags
//
#define CONTEXT_XSTATE_APX               0x00000080  ///< APX extended GPRs (R16-R31)
#define CONTEXT_XSTATE_AMX               0x00040000  ///< AMX tile registers

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
/// i8086 CONTEXT structure (Intel 8086/8088)
///
typedef struct ALIGNED_STRUCT(2) _I8086_CONTEXT {
  //
  // Context flags
  //
  UINT16  ContextFlags;

  //
  // General purpose registers
  //
  UINT16  Ax;
  UINT16  Bx;
  UINT16  Cx;
  UINT16  Dx;
  UINT16  Si;
  UINT16  Di;
  UINT16  Bp;
  UINT16  Sp;

  //
  // Instruction pointer and flags
  //
  UINT16  Ip;
  UINT16  Flags;

  //
  // Segment registers
  //
  UINT16  Cs;
  UINT16  Ds;
  UINT16  Es;
  UINT16  Ss;
} I8086_CONTEXT, *PI8086_CONTEXT;

///
/// i80286 CONTEXT structure (Intel 80286 with protected mode)
///
typedef struct ALIGNED_STRUCT(2) _I80286_CONTEXT {
  //
  // Context flags
  //
  UINT16  ContextFlags;
  UINT16  Padding;

  //
  // General purpose registers
  //
  UINT16  Ax;
  UINT16  Bx;
  UINT16  Cx;
  UINT16  Dx;
  UINT16  Si;
  UINT16  Di;
  UINT16  Bp;
  UINT16  Sp;

  //
  // Instruction pointer and flags
  //
  UINT16  Ip;
  UINT16  Flags;

  //
  // Segment registers (can hold selectors in protected mode)
  //
  UINT16  Cs;
  UINT16  Ds;
  UINT16  Es;
  UINT16  Ss;

  //
  // Machine Status Word
  //
  UINT16  Msw;

  //
  // Optional 80287 FPU state (if coprocessor present)
  //
  UINT8   FpuState[94];  ///< 80287 FPU context (FSAVE format)
} I80286_CONTEXT, *PI80286_CONTEXT;

///
/// Intel APX (Advanced Performance Extensions) - Extended GPRs R16-R31
///
typedef struct _APX_CONTEXT {
  UINT64  R16;
  UINT64  R17;
  UINT64  R18;
  UINT64  R19;
  UINT64  R20;
  UINT64  R21;
  UINT64  R22;
  UINT64  R23;
  UINT64  R24;
  UINT64  R25;
  UINT64  R26;
  UINT64  R27;
  UINT64  R28;
  UINT64  R29;
  UINT64  R30;
  UINT64  R31;
} APX_CONTEXT, *PAPX_CONTEXT;

///
/// AMX Tile Configuration Structure
///
typedef struct _AMX_TILECFG {
  UINT8   PaletteId;        ///< Palette selector (0=init, 1=8KB across 8 tiles)
  UINT8   StartRow;         ///< Starting row
  UINT8   Reserved[14];     ///< Reserved bytes
  UINT16  Colb[16];         ///< Columns in bytes for each tile (0-15)
  UINT8   Rows[16];         ///< Rows for each tile (0-15, max 16)
} AMX_TILECFG, *PAMX_TILECFG;

///
/// Intel AMX (Advanced Matrix Extensions) - Tile Registers
/// Each tile is max 16 rows × 64 bytes = 1024 bytes
///
typedef struct ALIGNED_STRUCT(64) _AMX_CONTEXT {
  AMX_TILECFG  TileConfig;
  UINT8        Tmm0[1024];  ///< Tile 0 data
  UINT8        Tmm1[1024];  ///< Tile 1 data
  UINT8        Tmm2[1024];  ///< Tile 2 data
  UINT8        Tmm3[1024];  ///< Tile 3 data
  UINT8        Tmm4[1024];  ///< Tile 4 data
  UINT8        Tmm5[1024];  ///< Tile 5 data
  UINT8        Tmm6[1024];  ///< Tile 6 data
  UINT8        Tmm7[1024];  ///< Tile 7 data
} AMX_CONTEXT, *PAMX_CONTEXT;

#endif // __CORE_CONTEXT_X86_H__
