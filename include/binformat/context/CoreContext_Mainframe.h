/** @file
  Mainframe and High-End Workstation Architecture Register Context Structures.

  Includes S/390, zArchitecture, IA-64 (Itanium), PA-RISC, and high-end
  systems from IBM, Intel, HP, and Cray.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_MAINFRAME_H__
#define __CORE_CONTEXT_MAINFRAME_H__

//
// Context flags for mainframe architectures
//
#define CONTEXT_S390                    0x80000001
#define CONTEXT_S390X                   0x80000002
#define CONTEXT_IA64                    0x80000003
#define CONTEXT_PARISC                  0x80000004
#define CONTEXT_PARISC64                0x80000005
#define CONTEXT_CRAY                    0x80000006

///
/// IBM S/390 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _S390_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R15
  UINT32  R[16];   ///< R15=SP

  // Access registers AR0-AR15
  UINT32  Ar[16];

  // Program counter (PSW address)
  UINT32  Pswa;

  // Program status word
  UINT64  Psw;
} S390_CONTEXT, *PS390_CONTEXT;

///
/// IBM zArchitecture (S/390x) 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _S390X_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // General registers R0-R15
  UINT64  R[16];   ///< R15=SP

  // Access registers AR0-AR15
  UINT32  Ar[16];

  // Floating-point registers F0-F15
  UINT64  F[16];

  // Program counter (PSW address)
  UINT64  Pswa;

  // Program status word
  UINT64  Psw;

  // Floating-point control register
  UINT32  Fpc;
  UINT32  Padding2;
} S390X_CONTEXT, *PS390X_CONTEXT;

///
/// Intel IA-64 (Itanium) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(16) _IA64_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // General registers R0-R127
  UINT64  R[128];  ///< R0=0, R1=GP, R12=SP, many banked

  // Branch registers B0-B7
  UINT64  B[8];    ///< B0=return link

  // Predicate registers (64 1-bit predicates)
  UINT64  Pr;

  // Application registers
  UINT64  Ar_rsc;  ///< RSE configuration
  UINT64  Ar_bsp;  ///< Backing store pointer
  UINT64  Ar_bspstore;
  UINT64  Ar_rnat;
  UINT64  Ar_pfs;  ///< Previous function state
  UINT64  Ar_lc;   ///< Loop count
  UINT64  Ar_ec;   ///< Epilog count

  // Instruction pointer
  UINT64  Ip;

  // Processor status register
  UINT64  Psr;

  // Current frame marker
  UINT64  Cfm;

  // Floating-point registers F0-F127 (82-bit, 16-byte aligned)
  UINT8   F[128][16];

  // Floating-point status register
  UINT64  Fpsr;
} IA64_CONTEXT, *PIA64_CONTEXT;

///
/// HP PA-RISC 32-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _PARISC_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R31
  UINT32  R[32];   ///< R0=0, R2=RP, R27=DP, R30=SP

  // Space registers SR0-SR7
  UINT32  Sr[8];

  // Program counter (IAOQ head and tail)
  UINT32  Iaoq_head;
  UINT32  Iaoq_tail;

  // Instruction address offset queue
  UINT32  Iasq_head;
  UINT32  Iasq_tail;

  // Processor status word
  UINT32  Psw;
} PARISC_CONTEXT, *PPARISC_CONTEXT;

///
/// HP PA-RISC 64-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _PARISC64_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // General registers R0-R31
  UINT64  R[32];   ///< R0=0, R2=RP, R27=DP, R30=SP

  // Space registers SR0-SR7
  UINT64  Sr[8];

  // Program counter (IAOQ head and tail)
  UINT64  Iaoq_head;
  UINT64  Iaoq_tail;

  // Instruction address offset queue
  UINT64  Iasq_head;
  UINT64  Iasq_tail;

  // Processor status word
  UINT64  Psw;

  // Floating-point registers FR0-FR31
  UINT64  Fr[32];
} PARISC64_CONTEXT, *PPARISC64_CONTEXT;

//
// Stub structures for additional mainframe/supercomputer architectures
//

typedef struct { UINT32 ContextFlags; UINT64 Reserved[32]; } CRAY_CONTEXT, *PCRAY_CONTEXT;

#endif // __CORE_CONTEXT_MAINFRAME_H__
