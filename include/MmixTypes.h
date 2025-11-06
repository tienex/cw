/** @file
  MMIX Emulator type definitions.

  This file contains all fundamental type definitions, constants, and
  structures used throughout the MMIX emulator implementation. It follows
  NT coding conventions with UEFI-style documentation.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_TYPES_H__
#define __MMIX_TYPES_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

//
// Basic type definitions following NT style
//

typedef uint8_t   UINT8;
typedef uint16_t  UINT16;
typedef uint32_t  UINT32;
typedef uint64_t  UINT64;
typedef int8_t    INT8;
typedef int16_t   INT16;
typedef int32_t   INT32;
typedef int64_t   INT64;
typedef float     FLOAT32;
typedef double    FLOAT64;
typedef bool      BOOLEAN;
typedef void      VOID;
typedef char      CHAR8;
typedef uint16_t  CHAR16;

#ifndef TRUE
#define TRUE   ((BOOLEAN)1)
#endif

#ifndef FALSE
#define FALSE  ((BOOLEAN)0)
#endif

#ifndef NULL
#define NULL   ((VOID *)0)
#endif

//
// Parameter direction attributes
//

#define IN
#define OUT
#define OPTIONAL
#define CONST const

//
// Function scope qualifiers
//

#define STATIC static
#define EXTERN extern

//
// Return status codes
//

typedef UINT64 MMIX_STATUS;

#define MMIX_SUCCESS                    0x0000000000000000ULL
#define MMIX_ERROR_INVALID_PARAMETER    0x8000000000000001ULL
#define MMIX_ERROR_OUT_OF_MEMORY        0x8000000000000002ULL
#define MMIX_ERROR_DEVICE_ERROR         0x8000000000000003ULL
#define MMIX_ERROR_WRITE_PROTECTED      0x8000000000000004ULL
#define MMIX_ERROR_OUT_OF_RESOURCES     0x8000000000000005ULL
#define MMIX_ERROR_UNSUPPORTED          0x8000000000000006ULL
#define MMIX_ERROR_NOT_FOUND            0x8000000000000007ULL
#define MMIX_ERROR_TIMEOUT              0x8000000000000008ULL
#define MMIX_ERROR_ABORTED              0x8000000000000009ULL
#define MMIX_ERROR_ACCESS_DENIED        0x800000000000000AULL
#define MMIX_ERROR_BUFFER_TOO_SMALL     0x800000000000000BULL
#define MMIX_ERROR_NOT_IMPLEMENTED      0x800000000000000CULL

#define MMIX_IS_ERROR(Status)  ((Status) & 0x8000000000000000ULL)

//
// MMIX register file size constants
//

#define MMIX_GENERAL_REGISTER_COUNT     256
#define MMIX_SPECIAL_REGISTER_COUNT     32
#define MMIX_FLOATING_REGISTER_COUNT    256
#define MMIX_VECTOR_REGISTER_COUNT      256
#define MMIX_PREDICATE_REGISTER_COUNT   64
#define MMIX_MATRIX_TILE_COUNT          8

//
// Special register indices
//

typedef enum {
  MMIX_rA = 0,    ///< Arithmetic status register
  MMIX_rB = 1,    ///< Bootstrap register
  MMIX_rC = 2,    ///< Cycle counter
  MMIX_rD = 3,    ///< Dividend register
  MMIX_rE = 4,    ///< Epsilon register
  MMIX_rF = 5,    ///< Failure location register
  MMIX_rG = 6,    ///< Global threshold register
  MMIX_rH = 7,    ///< High multiply result register
  MMIX_rI = 8,    ///< Interval counter
  MMIX_rJ = 9,    ///< Return jump register
  MMIX_rK = 10,   ///< Interrupt mask register
  MMIX_rL = 11,   ///< Local threshold register
  MMIX_rM = 12,   ///< Multiplex mask register
  MMIX_rN = 13,   ///< Serial number register
  MMIX_rO = 14,   ///< Register stack offset
  MMIX_rP = 15,   ///< Prediction register
  MMIX_rQ = 16,   ///< Interrupt request register
  MMIX_rR = 17,   ///< Remainder register
  MMIX_rS = 18,   ///< Register stack pointer
  MMIX_rT = 19,   ///< Trap address register
  MMIX_rU = 20,   ///< Usage counter
  MMIX_rV = 21,   ///< Virtual translation register
  MMIX_rW = 22,   ///< Where interrupted register
  MMIX_rX = 23,   ///< Execution register
  MMIX_rY = 24,   ///< Y operand register
  MMIX_rZ = 25,   ///< Z operand register
  MMIX_rBB = 26,  ///< Bootstrap backup
  MMIX_rTT = 27,  ///< Dynamic trap address
  MMIX_rWW = 28,  ///< Alternative where interrupted
  MMIX_rXX = 29,  ///< Execution backup
  MMIX_rYY = 30,  ///< Y backup
  MMIX_rZZ = 31   ///< Z backup
} MMIX_SPECIAL_REGISTER;

//
// Extended special registers (for emulator extensions)
//

typedef enum {
  MMIX_rVL = 32,  ///< Vector length register
  MMIX_rVT = 33,  ///< Vector type register
  MMIX_rMT = 34,  ///< Matrix tile configuration
  MMIX_rEN = 35,  ///< Endianness control
  MMIX_rPR = 36,  ///< Privilege level register
  MMIX_rPT = 37,  ///< Page table base register
  MMIX_rAS = 38,  ///< Address space ID
  MMIX_rGC0 = 39, ///< Guest control 0
  MMIX_rGC1 = 40, ///< Guest control 1
  MMIX_rGC2 = 41, ///< Guest control 2
  MMIX_rGC3 = 42  ///< Guest control 3
} MMIX_EXTENDED_SPECIAL_REGISTER;

//
// Privilege levels - KESU 4-ring model (VMS-style)
//

typedef enum {
  MmixPrivilegeUser = 3,        ///< Ring 3: User mode (least privileged)
  MmixPrivilegeSupervisor = 2,  ///< Ring 2: Supervisor mode
  MmixPrivilegeExecutive = 1,   ///< Ring 1: Executive mode (VMS outer executive)
  MmixPrivilegeKernel = 0,      ///< Ring 0: Kernel mode (most privileged)
  MmixPrivilegeHypervisor = 4   ///< Hypervisor mode (outside ring model, optional)
} MMIX_PRIVILEGE_LEVEL;

#define MMIX_RING_COUNT  4  ///< Number of protection rings (K, E, S, U)

//
// Endianness modes
//

typedef enum {
  MmixEndianBig = 0,    ///< Big-endian (MMIX traditional)
  MmixEndianLittle = 1  ///< Little-endian (modern systems)
} MMIX_ENDIAN_MODE;

//
// Stack growth direction
//

typedef enum {
  MmixStackGrowsDown = 0,  ///< Stack grows toward lower addresses (x86-style)
  MmixStackGrowsUp = 1     ///< Stack grows toward higher addresses (PA-RISC style)
} MMIX_STACK_DIRECTION;

//
// Per-ring configuration structure
//

/**
  Configuration for each protection ring.

  Each ring can have independent settings for endianness,
  stack growth direction, and page table base. This enables
  VMS-style operating system architectures with different
  execution environments per privilege level.
**/
typedef struct {
  ///
  /// Endianness mode for this ring
  ///
  MMIX_ENDIAN_MODE  EndiannessMode;

  ///
  /// Stack growth direction for this ring
  ///
  MMIX_STACK_DIRECTION  StackDirection;

  ///
  /// Page table base physical address for this ring
  ///
  UINT64  PageTableBase;

  ///
  /// Stack pointer for this ring (saved on ring transitions)
  ///
  UINT64  StackPointer;

  ///
  /// Ring is enabled (allows per-ring disabling)
  ///
  BOOLEAN  Enabled;
} MMIX_RING_CONFIG;

//
// Arithmetic status register (rA) flags
//

#define MMIX_ARITH_FLAG_OVERFLOW        0x01
#define MMIX_ARITH_FLAG_DIVIDE_BY_ZERO  0x02
#define MMIX_ARITH_FLAG_INVALID_OP      0x04
#define MMIX_ARITH_FLAG_INEXACT         0x08
#define MMIX_ARITH_FLAG_UNDERFLOW       0x10
#define MMIX_ARITH_FLAG_FP_OVERFLOW     0x20

//
// Floating-point rounding modes
//

typedef enum {
  MmixRoundNearestEven = 0,   ///< Round to nearest, ties to even
  MmixRoundTowardZero = 1,    ///< Round toward zero
  MmixRoundTowardNegInf = 2,  ///< Round toward negative infinity
  MmixRoundTowardPosInf = 3,  ///< Round toward positive infinity
  MmixRoundNearestAway = 4    ///< Round to nearest, ties away from zero
} MMIX_ROUNDING_MODE;

//
// Floating-point format types
//

typedef enum {
  MmixFpFormatFp8E5M2 = 0,    ///< 8-bit float (5 exp, 2 mantissa)
  MmixFpFormatFp8E4M3 = 1,    ///< 8-bit float (4 exp, 3 mantissa)
  MmixFpFormatBFloat16 = 2,   ///< Brain float 16-bit
  MmixFpFormatBinary16 = 3,   ///< IEEE 754 binary16
  MmixFpFormatBinary32 = 4,   ///< IEEE 754 binary32
  MmixFpFormatBinary64 = 5,   ///< IEEE 754 binary64
  MmixFpFormatBinary128 = 6,  ///< IEEE 754 binary128
  MmixFpFormatDecimal32 = 7,  ///< IEEE 754 decimal32
  MmixFpFormatDecimal64 = 8,  ///< IEEE 754 decimal64
  MmixFpFormatDecimal128 = 9  ///< IEEE 754 decimal128
} MMIX_FP_FORMAT;

//
// Vector and matrix configuration
//

#define MMIX_MAX_VECTOR_LENGTH_BYTES    256  ///< Maximum vector length
#define MMIX_MIN_VECTOR_LENGTH_BYTES    16   ///< Minimum vector length
#define MMIX_MAX_MATRIX_TILE_SIZE       2048 ///< Maximum tile dimension

/**
  Vector element type encoding.
**/
typedef enum {
  MmixVectorTypeInt8 = 0,    ///< 8-bit integer
  MmixVectorTypeInt16 = 1,   ///< 16-bit integer
  MmixVectorTypeInt32 = 2,   ///< 32-bit integer
  MmixVectorTypeInt64 = 3,   ///< 64-bit integer
  MmixVectorTypeFp16 = 4,    ///< 16-bit float
  MmixVectorTypeFp32 = 5,    ///< 32-bit float
  MmixVectorTypeFp64 = 6,    ///< 64-bit float
  MmixVectorTypeBf16 = 7,    ///< BFloat16
  MmixVectorTypeFp8 = 8      ///< 8-bit float
} MMIX_VECTOR_ELEMENT_TYPE;

//
// Page table entry format
//

typedef union {
  struct {
    UINT64 Present : 1;           ///< Page is present in memory
    UINT64 ReadWrite : 1;         ///< Write permission
    UINT64 UserSupervisor : 1;    ///< User mode access allowed
    UINT64 WriteThrough : 1;      ///< Write-through caching
    UINT64 CacheDisable : 1;      ///< Caching disabled
    UINT64 Accessed : 1;          ///< Page has been accessed
    UINT64 Dirty : 1;             ///< Page has been written
    UINT64 PageSize : 1;          ///< Large page (1) or standard (0)
    UINT64 Global : 1;            ///< Global page (TLB not flushed on CR3 write)
    UINT64 Available1 : 3;        ///< Available for OS use
    UINT64 PhysicalAddress : 40;  ///< Physical page address (bits 51:12)
    UINT64 Available2 : 11;       ///< Available for OS use
    UINT64 ExecuteDisable : 1;    ///< Execution disabled
  } Bits;
  UINT64 Uint64;
} PAGE_TABLE_ENTRY;

//
// TLB entry structure
//

typedef struct {
  UINT64  VirtualAddress;     ///< Virtual page address
  UINT64  PhysicalAddress;    ///< Physical page address
  UINT16  Asid;               ///< Address space ID
  UINT8   Ring;               ///< Protection ring (0-3 for KESU, 4 for hypervisor)
  UINT8   PageSize;           ///< Page size encoding (0=4KB, 1=2MB, 2=1GB)
  UINT8   Flags;              ///< Permission and attribute flags
  BOOLEAN Valid;              ///< Entry is valid
} TLB_ENTRY;

//
// Instruction format
//

typedef union {
  struct {
    UINT32 Z : 8;       ///< Z operand or low immediate
    UINT32 Y : 8;       ///< Y operand or high immediate
    UINT32 X : 8;       ///< X operand (destination)
    UINT32 Opcode : 8;  ///< Operation code
  } Fields;
  struct {
    UINT32 YZ : 16;     ///< 16-bit immediate
    UINT32 X : 8;       ///< X operand
    UINT32 Opcode : 8;  ///< Operation code
  } Immediate16;
  struct {
    UINT32 XYZ : 24;    ///< 24-bit immediate
    UINT32 Opcode : 8;  ///< Operation code
  } Immediate24;
  UINT32 Uint32;
} MMIX_INSTRUCTION;

//
// Compressed instruction format (16-bit)
//

typedef union {
  struct {
    UINT16 Func : 4;    ///< Function code
    UINT16 Rs1 : 4;     ///< Source register 1
    UINT16 Rs2 : 4;     ///< Source register 2
    UINT16 Opcode : 4;  ///< Compressed opcode
  } CR;
  struct {
    UINT16 ImmLow : 4;  ///< Immediate low bits
    UINT16 Rs1 : 4;     ///< Source register 1
    UINT16 ImmHigh : 4; ///< Immediate high bits
    UINT16 Opcode : 4;  ///< Compressed opcode
  } CI;
  struct {
    UINT16 Offset : 12; ///< Jump/branch offset
    UINT16 Opcode : 4;  ///< Compressed opcode
  } CJ;
  UINT16 Uint16;
} MMIX_COMPRESSED_INSTRUCTION;

//
// Exception types
//

typedef enum {
  MmixExceptionReset = 0x00,              ///< Reset/power-on
  MmixExceptionArithmeticOverflow = 0x01, ///< Integer overflow
  MmixExceptionDivideByZero = 0x02,       ///< Division by zero
  MmixExceptionInvalidOperation = 0x03,   ///< Invalid instruction
  MmixExceptionPrivileged = 0x04,         ///< Privileged instruction fault
  MmixExceptionPageFault = 0x05,          ///< Page fault
  MmixExceptionProtection = 0x06,         ///< Protection violation
  MmixExceptionAlignment = 0x07,          ///< Misaligned access
  MmixExceptionFloatingPoint = 0x08,      ///< Floating-point exception
  MmixExceptionBreakpoint = 0x09,         ///< Debug breakpoint
  MmixExceptionSingleStep = 0x0A,         ///< Single-step debug
  MmixExceptionVmExit = 0x0B,             ///< Virtual machine exit
  MmixExceptionTimer = 0x10,              ///< Timer interrupt
  MmixExceptionExternal = 0x11,           ///< External interrupt
  MmixExceptionIpi = 0x12,                ///< Inter-processor interrupt
  MmixExceptionSoftware = 0x13            ///< Software interrupt
} MMIX_EXCEPTION_TYPE;

//
// Forward declarations
//

typedef struct _MMIX_CPU_STATE MMIX_CPU_STATE;
typedef struct _MMIX_MEMORY_STATE MMIX_MEMORY_STATE;
typedef struct _MMIX_DEVICE_STATE MMIX_DEVICE_STATE;
typedef struct _MMIX_EMULATOR_CONTEXT MMIX_EMULATOR_CONTEXT;

//
// Function pointer types
//

/**
  Instruction execution handler function type.

  @param[in,out]  CpuState      Pointer to CPU state.
  @param[in]      Instruction   The instruction to execute.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Error occurred during execution.

**/
typedef
MMIX_STATUS
(*MMIX_INSTRUCTION_HANDLER)(
  IN OUT MMIX_CPU_STATE   *CpuState,
  IN     MMIX_INSTRUCTION Instruction
  );

/**
  Memory access function type.

  @param[in]      MemoryState   Pointer to memory state.
  @param[in]      Address       Virtual address to access.
  @param[out]     Buffer        Buffer to read/write data.
  @param[in]      Size          Number of bytes to access.
  @param[in]      Write         TRUE for write, FALSE for read.

  @retval MMIX_SUCCESS          Access completed successfully.
  @retval Others                Error occurred during access.

**/
typedef
MMIX_STATUS
(*MMIX_MEMORY_ACCESS_HANDLER)(
  IN     MMIX_MEMORY_STATE *MemoryState,
  IN     UINT64            Address,
  IN OUT VOID              *Buffer,
  IN     UINT64            Size,
  IN     BOOLEAN           Write
  );

/**
  Device I/O handler function type.

  @param[in]      DeviceState   Pointer to device state.
  @param[in]      Offset        Register offset.
  @param[in,out]  Value         Pointer to value to read/write.
  @param[in]      Size          Access size in bytes.
  @param[in]      Write         TRUE for write, FALSE for read.

  @retval MMIX_SUCCESS          I/O completed successfully.
  @retval Others                Error occurred during I/O.

**/
typedef
MMIX_STATUS
(*MMIX_DEVICE_IO_HANDLER)(
  IN     MMIX_DEVICE_STATE *DeviceState,
  IN     UINT64            Offset,
  IN OUT VOID              *Value,
  IN     UINT32            Size,
  IN     BOOLEAN           Write
  );

#endif // __MMIX_TYPES_H__
