/** @file
  Legacy Architecture Register Context Structures.

  Includes VAX, PDP-11, PDP-10, M68K, M88K, NS32K, and other historical
  architectures from 1960s-1990s.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_LEGACY_H__
#define __CORE_CONTEXT_LEGACY_H__

//
// Context flags for legacy architectures
//
#define CONTEXT_VAX                     0x50000001
#define CONTEXT_PDP11                   0x50000002
#define CONTEXT_PDP10                   0x50000003
#define CONTEXT_M68K                    0x50000004
#define CONTEXT_M88K                    0x50000005
#define CONTEXT_NS32K                   0x50000006
#define CONTEXT_TAHOE                   0x50000007
#define CONTEXT_WE32K                   0x50000008
#define CONTEXT_CONVEX                  0x50000009
#define CONTEXT_PYRAMID                 0x5000000A
#define CONTEXT_CLIPPER                 0x5000000B

///
/// DEC VAX CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _VAX_CONTEXT {
  UINT32  ContextFlags;

  // General purpose registers R0-R15
  UINT32  R[16];  ///< R12=AP, R13=FP, R14=SP, R15=PC

  // Processor status longword
  UINT32  Psl;
} VAX_CONTEXT, *PVAX_CONTEXT;

///
/// DEC PDP-11 CONTEXT structure
///
typedef struct ALIGNED_STRUCT(2) _PDP11_CONTEXT {
  UINT16  ContextFlags;

  // General purpose registers R0-R7
  UINT16  R[8];  ///< R6=SP, R7=PC

  // Processor status word
  UINT16  Psw;
} PDP11_CONTEXT, *PPDP11_CONTEXT;

///
/// Motorola 68000 family CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _M68K_CONTEXT {
  UINT32  ContextFlags;

  // Data registers D0-D7
  UINT32  D[8];

  // Address registers A0-A7
  UINT32  A[8];  ///< A7=SP

  // Program counter
  UINT32  Pc;

  // Status register
  UINT16  Sr;
  UINT16  Padding;
} M68K_CONTEXT, *PM68K_CONTEXT;

///
/// Motorola 88000 family CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _M88K_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R31
  UINT32  R[32];  ///< R0 always 0, R1=SP, R31=return link

  // Program counter
  UINT32  Pc;

  // Processor status register
  UINT32  Psr;
} M88K_CONTEXT, *PM88K_CONTEXT;

///
/// National Semiconductor 32032 CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _NS32K_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R7
  UINT32  R[8];  ///< R6=FP, R7=SP

  // Program counter
  UINT32  Pc;

  // Processor status register
  UINT16  Psr;
  UINT16  Padding;
} NS32K_CONTEXT, *PNS32K_CONTEXT;

//
// Stub structures for less common legacy architectures
// These can be expanded as needed
//

typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } TAHOE_CONTEXT, *PTAHOE_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } WE32K_CONTEXT, *PWE32K_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT64 Reserved[32]; } CONVEX_CONTEXT, *PCONVEX_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } PYRAMID_CONTEXT, *PPYRAMID_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } CLIPPER_CONTEXT, *PCLIPPER_CONTEXT;

#endif // __CORE_CONTEXT_LEGACY_H__
