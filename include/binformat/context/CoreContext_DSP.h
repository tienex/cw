/** @file
  DSP Architecture Register Context Structures.

  Includes TI DSPs (C54x, C55x, C6x), Analog Devices Blackfin, Qualcomm
  Hexagon, and other digital signal processor architectures.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_DSP_H__
#define __CORE_CONTEXT_DSP_H__

//
// Context flags for DSP architectures
//
#define CONTEXT_TIC54X                  0x70000001
#define CONTEXT_TIC55X                  0x70000002
#define CONTEXT_TIC6X                   0x70000003
#define CONTEXT_BLACKFIN                0x70000004
#define CONTEXT_HEXAGON                 0x70000005
#define CONTEXT_EPIPHANY                0x70000006

///
/// TI TMS320C54x DSP CONTEXT structure
///
typedef struct ALIGNED_STRUCT(2) _TIC54X_CONTEXT {
  UINT16  ContextFlags;

  // Accumulators
  UINT32  Acc[2];  ///< A and B accumulators (40-bit, stored as 32-bit)

  // Auxiliary registers
  UINT16  Ar[8];   ///< AR0-AR7

  // Other registers
  UINT16  Pc;      ///< Program counter
  UINT16  Sp;      ///< Stack pointer
  UINT16  St0;     ///< Status register 0
  UINT16  St1;     ///< Status register 1
} TIC54X_CONTEXT, *PTIC54X_CONTEXT;

///
/// TI TMS320C55x DSP CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _TIC55X_CONTEXT {
  UINT32  ContextFlags;

  // Accumulators
  UINT32  Ac[4];   ///< AC0-AC3 (40-bit, stored as 32-bit)

  // Extended auxiliary registers
  UINT32  Xar[8];  ///< XAR0-XAR7

  // Program counter
  UINT32  Pc;

  // Stack pointers
  UINT32  Ssp;     ///< System stack pointer
  UINT32  Sp;      ///< Data stack pointer

  // Status registers
  UINT32  St0;
  UINT32  St1;
  UINT32  St2;
  UINT32  St3;
} TIC55X_CONTEXT, *PTIC55X_CONTEXT;

///
/// TI TMS320C6x DSP CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _TIC6X_CONTEXT {
  UINT32  ContextFlags;

  // General registers A0-A15, B0-B15
  UINT32  A[16];
  UINT32  B[16];

  // Program counter
  UINT32  Pc;

  // Control registers
  UINT32  Csr;     ///< Control status register
  UINT32  Icr;     ///< Interrupt clear register
  UINT32  Ier;     ///< Interrupt enable register
  UINT32  Istp;    ///< Interrupt service table pointer
} TIC6X_CONTEXT, *PTIC6X_CONTEXT;

///
/// Analog Devices Blackfin DSP CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _BLACKFIN_CONTEXT {
  UINT32  ContextFlags;

  // Data registers
  UINT32  R[8];    ///< R0-R7

  // Pointer registers
  UINT32  P[6];    ///< P0-P5

  // Index registers
  UINT32  I[4];    ///< I0-I3

  // Modify registers
  UINT32  M[4];    ///< M0-M3

  // Base registers
  UINT32  B[4];    ///< B0-B3

  // Length registers
  UINT32  L[4];    ///< L0-L3

  // Accumulators
  UINT64  A0;      ///< Accumulator 0 (40-bit)
  UINT64  A1;      ///< Accumulator 1 (40-bit)

  // Program counter
  UINT32  Pc;

  // Status registers
  UINT32  Astat;   ///< ALU status
  UINT32  Seqstat; ///< Sequencer status
} BLACKFIN_CONTEXT, *PBLACKFIN_CONTEXT;

///
/// Qualcomm Hexagon DSP CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _HEXAGON_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R31
  UINT32  R[32];   ///< R29=SP, R30=FP, R31=LR

  // Predicate registers P0-P3
  UINT8   P[4];

  // Program counter
  UINT32  Pc;

  // Status registers
  UINT32  Sr;      ///< Status register
  UINT32  Usr;     ///< User status register
} HEXAGON_CONTEXT, *PHEXAGON_CONTEXT;

//
// Stub structures for additional DSP architectures
//

typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } EPIPHANY_CONTEXT, *PEPIPHANY_CONTEXT;

#endif // __CORE_CONTEXT_DSP_H__
