/** @file
  Modern Architecture Register Context Structures.

  Includes WebAssembly, BPF/eBPF, and other recent/specialized architectures.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __CORE_CONTEXT_MODERN_H__
#define __CORE_CONTEXT_MODERN_H__

//
// Context flags for modern architectures
//
#define CONTEXT_WASM                    0x90000001
#define CONTEXT_BPF                     0x90000002
#define CONTEXT_EBPF                    0x90000003
#define CONTEXT_LANAI                   0x90000004
#define CONTEXT_KVX                     0x90000005
#define CONTEXT_ELBRUS2K                0x90000006

///
/// WebAssembly CONTEXT structure
/// WASM uses a stack machine with local variables
///
typedef struct ALIGNED_STRUCT(8) _WASM_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // Program counter (instruction index)
  UINT64  Pc;

  // Stack pointer (operand stack)
  UINT64  Sp;

  // Frame pointer (call frame)
  UINT64  Fp;

  // Local variables (depends on function, max common size)
  UINT64  Locals[64];

  // Operand stack (value stack)
  UINT64  Stack[256];

  // Stack depth
  UINT32  StackDepth;
  UINT32  Padding2;
} WASM_CONTEXT, *PWASM_CONTEXT;

///
/// Classic BPF (Berkeley Packet Filter) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _BPF_CONTEXT {
  UINT32  ContextFlags;

  // Accumulator
  UINT32  A;

  // Index register
  UINT32  X;

  // Program counter
  UINT32  Pc;

  // Memory (M[0]-M[15])
  UINT32  M[16];
} BPF_CONTEXT, *PBPF_CONTEXT;

///
/// Extended BPF (eBPF) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _EBPF_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // General registers R0-R10
  UINT64  R[11];   ///< R0=return, R1-R5=args, R6-R9=callee-saved, R10=FP

  // Program counter
  UINT64  Pc;
} EBPF_CONTEXT, *PEBPF_CONTEXT;

///
/// Google Lanai CONTEXT structure
///
typedef struct ALIGNED_STRUCT(4) _LANAI_CONTEXT {
  UINT32  ContextFlags;

  // General registers R0-R31
  UINT32  R[32];   ///< R2=SP, R3=FP, R31=RCA (return continuation address)

  // Program counter
  UINT32  Pc;

  // Status register
  UINT32  Ps;
} LANAI_CONTEXT, *PLANAI_CONTEXT;

///
/// Kalray KVX (VLIW) CONTEXT structure
///
typedef struct ALIGNED_STRUCT(8) _KVX_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // General registers R0-R63
  UINT64  R[64];   ///< R12=SP

  // Program counter
  UINT64  Pc;

  // System registers
  UINT64  Ps;      ///< Processor status
  UINT64  Pcr;     ///< Processor control register
} KVX_CONTEXT, *PKVX_CONTEXT;

///
/// Elbrus 2000 (e2k) CONTEXT structure
/// Russian VLIW architecture
///
typedef struct ALIGNED_STRUCT(8) _ELBRUS2K_CONTEXT {
  UINT32  ContextFlags;
  UINT32  Padding;

  // General registers R0-R127 (register file)
  UINT64  R[128];

  // Based registers B0-B15
  UINT64  B[16];

  // Global registers G0-G15
  UINT64  G[16];

  // Program counter
  UINT64  Pc;

  // Processor status register
  UINT64  Psr;
} ELBRUS2K_CONTEXT, *PELBRUS2K_CONTEXT;

#endif // __CORE_CONTEXT_MODERN_H__
