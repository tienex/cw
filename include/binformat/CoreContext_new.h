/** @file
  Universal Core Dump Register Context Structures.

  This header defines NT-style CONTEXT structures for all architectures,
  providing a universal format for register contexts from core dumps
  (ELF, Mach-O, PE minidump, etc.).

  For Windows-supported architectures (x86, x64, ARM, ARM64), we use the
  standard NT CONTEXT structures. For other architectures, we define
  similar structures following the same naming and layout patterns.

  This file has been split into architecture-specific headers for better
  organization and maintainability.

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
/// Helper structure for 128-bit values (XMM, vector registers)
///
typedef struct _M128A {
  UINT64  Low;
  INT64   High;
} M128A, *PM128A;

///
/// Helper structure for 256-bit values (YMM registers)
///
typedef struct ALIGNED_STRUCT(32) _M256 {
  UINT64  Part[4];
} M256, *PM256;

///
/// Helper structure for 512-bit values (ZMM registers)
///
typedef struct ALIGNED_STRUCT(64) _M512 {
  UINT64  Part[8];
} M512, *PM512;

//
// Include architecture-specific context structures
//
#include "context/CoreContext_x86.h"
#include "context/CoreContext_ARM.h"
#include "context/CoreContext_RISCV.h"
#include "context/CoreContext_MIPS.h"
#include "context/CoreContext_PowerPC.h"
#include "context/CoreContext_SPARC.h"
#include "context/CoreContext_Alpha.h"
#include "context/CoreContext_Legacy.h"
#include "context/CoreContext_Embedded.h"
#include "context/CoreContext_DSP.h"
#include "context/CoreContext_Mainframe.h"
#include "context/CoreContext_Modern.h"

#endif // __CORE_CONTEXT_H__
