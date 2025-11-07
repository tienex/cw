/** @file
  Embedded System Architecture Register Context Structures.

  Includes AVR, MSP430, 6502, Z80, H8, V850, and other microcontroller
  and embedded processor architectures.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_EMBEDDED_H__
#define __CORE_CONTEXT_EMBEDDED_H__

//
// Context flags for embedded architectures
//
#define CONTEXT_AVR                     0x60000001
#define CONTEXT_MSP430                  0x60000002
#define CONTEXT_6502                    0x60000003
#define CONTEXT_65816                   0x60000004
#define CONTEXT_Z80                     0x60000005
#define CONTEXT_H8300                   0x60000006
#define CONTEXT_V850                    0x60000007
#define CONTEXT_MCORE                   0x60000008
#define CONTEXT_AVR32                   0x60000009
#define CONTEXT_MICROBLAZE              0x6000000A
#define CONTEXT_NIOS2                   0x6000000B
#define CONTEXT_OPENRISC                0x6000000C

///
/// Atmel AVR 8-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(1) _AVR_CONTEXT {
  UINT8   ContextFlags;

  // General purpose registers R0-R31
  UINT8   R[32];  ///< R26-R27=X, R28-R29=Y, R30-R31=Z

  // Status register
  UINT8   Sreg;

  // Stack pointer (16-bit)
  UINT16  Sp;

  // Program counter (can be 16-bit or 22-bit depending on device)
  UINT32  Pc;
} AVR_CONTEXT, *PAVR_CONTEXT;

///
/// TI MSP430 16-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(2) _MSP430_CONTEXT {
  UINT16  ContextFlags;

  // General purpose registers R0-R15
  UINT16  R[16];  ///< R0=PC, R1=SP, R2=SR/CG1, R3=CG2, R4-R15=GP

  // Status register (same as R2)
  UINT16  Sr;
} MSP430_CONTEXT, *PMSP430_CONTEXT;

///
/// MOS 6502 8-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(1) _M6502_CONTEXT {
  UINT8   ContextFlags;

  // Accumulator
  UINT8   A;

  // Index registers
  UINT8   X;
  UINT8   Y;

  // Stack pointer (page 1: 0x0100-0x01FF)
  UINT8   S;

  // Processor status
  UINT8   P;

  // Program counter
  UINT16  Pc;
} M6502_CONTEXT, *PM6502_CONTEXT;

///
/// WDC 65816 16-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(2) _M65816_CONTEXT {
  UINT16  ContextFlags;

  // Accumulator (16-bit in native mode)
  UINT16  A;

  // Index registers (16-bit in native mode)
  UINT16  X;
  UINT16  Y;

  // Stack pointer
  UINT16  S;

  // Direct page register
  UINT16  D;

  // Data bank register
  UINT8   Db;

  // Program bank register
  UINT8   Pb;

  // Processor status
  UINT8   P;

  // Program counter
  UINT16  Pc;
} M65816_CONTEXT, *PM65816_CONTEXT;

///
/// Zilog Z80 8-bit CONTEXT structure
///
typedef struct ALIGNED_STRUCT(1) _Z80_CONTEXT {
  UINT8   ContextFlags;

  // Main register set
  UINT8   A, F;    ///< Accumulator and flags
  UINT8   B, C;
  UINT8   D, E;
  UINT8   H, L;

  // Alternate register set
  UINT8   A_alt, F_alt;
  UINT8   B_alt, C_alt;
  UINT8   D_alt, E_alt;
  UINT8   H_alt, L_alt;

  // Index registers
  UINT16  Ix;
  UINT16  Iy;

  // Stack pointer
  UINT16  Sp;

  // Program counter
  UINT16  Pc;

  // Special registers
  UINT8   I;       ///< Interrupt vector
  UINT8   R;       ///< Memory refresh
  UINT8   Iff1;    ///< Interrupt flip-flop 1
  UINT8   Iff2;    ///< Interrupt flip-flop 2
} Z80_CONTEXT, *PZ80_CONTEXT;

///
/// Hitachi H8/300 CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _H8300_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R7
  UINT32  R[8];  ///< R7=SP

  // Program counter
  UINT32  Pc;

  // Condition code register
  UINT8   Ccr;
  UINT8   Padding[3];
} H8300_CONTEXT, *PH8300_CONTEXT;

///
/// NEC V850 CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _V850_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R31
  UINT32  R[32];  ///< R0=zero, R3=SP, R31=LP

  // Program counter
  UINT32  Pc;

  // Program status word
  UINT32  Psw;
} V850_CONTEXT, *PV850_CONTEXT;

//
// Stub structures for additional embedded architectures
//

typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } MCORE_CONTEXT, *PMCORE_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } AVR32_CONTEXT, *PAVR32_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } MICROBLAZE_CONTEXT, *PMICROBLAZE_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } NIOS2_CONTEXT, *PNIOS2_CONTEXT;
typedef struct { UINT32 ContextFlags; UINT32 Reserved[32]; } OPENRISC_CONTEXT, *POPENRISC_CONTEXT;

#endif // __CORE_CONTEXT_EMBEDDED_H__
