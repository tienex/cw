/** @file
  ELF Core Dump Structures and Functions.

  This header defines structures for reading ELF core dumps (core files)
  and mapping NT_PRSTATUS notes to CONTEXT structures for debugging.

  Based on Linux kernel's elfcore.h and GDB's core file reading.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __ELF_CORE_H__
#define __ELF_CORE_H__

#include "BinFormat.h"
#include "CoreContext.h"

//
// ELF Core Note Types (from Linux kernel elf.h)
//
#define NT_PRSTATUS     1   ///< Process status (elf_prstatus)
#define NT_PRFPREG      2   ///< Floating point registers (elf_prfpreg)
#define NT_PRPSINFO     3   ///< Process info (elf_prpsinfo)
#define NT_TASKSTRUCT   4   ///< Task structure (obsolete)
#define NT_AUXV         6   ///< Auxiliary vector
#define NT_SIGINFO      0x53494749  ///< Signal info (siginfo_t)
#define NT_FILE         0x46494c45  ///< Mapped files
#define NT_PRXFPREG     0x46e62b7f  ///< SSE registers (x86)
#define NT_PPC_VMX      0x100       ///< PowerPC Altivec/VMX
#define NT_PPC_SPE      0x101       ///< PowerPC SPE
#define NT_PPC_VSX      0x102       ///< PowerPC VSX
#define NT_386_TLS      0x200       ///< i386 TLS
#define NT_386_IOPERM   0x201       ///< i386 IO permission
#define NT_X86_XSTATE   0x202       ///< x86 XSAVE extended state
#define NT_S390_HIGH_GPRS 0x300     ///< S/390 high GPRs
#define NT_S390_TIMER   0x301       ///< S/390 timer
#define NT_S390_TODCMP  0x302       ///< S/390 TOD comparator
#define NT_S390_TODPREG 0x303       ///< S/390 TOD programmable register
#define NT_S390_CTRS    0x304       ///< S/390 control registers
#define NT_S390_PREFIX  0x305       ///< S/390 prefix register
#define NT_S390_LAST_BREAK  0x306   ///< S/390 breaking event address
#define NT_S390_SYSTEM_CALL 0x307   ///< S/390 system call restart data
#define NT_S390_TDB     0x308       ///< S/390 transaction diagnostic block
#define NT_ARM_VFP      0x400       ///< ARM VFP registers
#define NT_ARM_TLS      0x401       ///< ARM TLS register
#define NT_ARM_HW_BREAK 0x402       ///< ARM hardware breakpoint
#define NT_ARM_HW_WATCH 0x403       ///< ARM hardware watchpoint
#define NT_ARM_SYSTEM_CALL 0x404    ///< ARM system call number
#define NT_ARM_SVE      0x405       ///< ARM SVE registers
#define NT_ARM_PAC_MASK 0x406       ///< ARM pointer authentication masks
#define NT_ARM_PACA_KEYS 0x407      ///< ARM pointer authentication address keys
#define NT_ARM_PACG_KEYS 0x408      ///< ARM pointer authentication generic keys
#define NT_ARM_TAGGED_ADDR_CTRL 0x409 ///< ARM tagged address control
#define NT_ARM_PAC_ENABLED_KEYS 0x40a ///< ARM pointer authentication enabled keys
#define NT_ARM_SSVE     0x40b       ///< ARM streaming SVE
#define NT_ARM_ZA       0x40c       ///< ARM SME ZA register
#define NT_ARM_ZT       0x40d       ///< ARM SME ZT registers
#define NT_ARM_FPMR     0x40e       ///< ARM floating-point mode register

//
// Signal info structure (from kernel siginfo.h)
//
typedef struct {
  INT32   si_signo;  ///< Signal number
  INT32   si_code;   ///< Signal code
  INT32   si_errno;  ///< Error number
} ELF_SIGINFO;

//
// Timeval structure
//
typedef struct {
  INT64   tv_sec;    ///< Seconds
  INT64   tv_usec;   ///< Microseconds
} ELF_TIMEVAL;

//
// Common portion of elf_prstatus (architecture-independent)
//
typedef struct {
  ELF_SIGINFO   pr_info;     ///< Info associated with signal
  INT16         pr_cursig;   ///< Current signal
  UINT16        padding1;
  UINT32        padding2;
  UINT64        pr_sigpend;  ///< Set of pending signals
  UINT64        pr_sighold;  ///< Set of held signals
  INT32         pr_pid;      ///< Process ID
  INT32         pr_ppid;     ///< Parent process ID
  INT32         pr_pgrp;     ///< Process group ID
  INT32         pr_sid;      ///< Session ID
  ELF_TIMEVAL   pr_utime;    ///< User time
  ELF_TIMEVAL   pr_stime;    ///< System time
  ELF_TIMEVAL   pr_cutime;   ///< Cumulative user time
  ELF_TIMEVAL   pr_cstime;   ///< Cumulative system time
} ELF_PRSTATUS_COMMON;

//
// x86-64 register set (elf_gregset_t for x86-64)
// Follows Linux kernel's user_regs_struct layout
//
typedef struct ALIGNED_STRUCT(8) {
  UINT64  r15;
  UINT64  r14;
  UINT64  r13;
  UINT64  r12;
  UINT64  rbp;
  UINT64  rbx;
  UINT64  r11;
  UINT64  r10;
  UINT64  r9;
  UINT64  r8;
  UINT64  rax;
  UINT64  rcx;
  UINT64  rdx;
  UINT64  rsi;
  UINT64  rdi;
  UINT64  orig_rax;  ///< Original syscall number
  UINT64  rip;
  UINT64  cs;
  UINT64  eflags;
  UINT64  rsp;
  UINT64  ss;
  UINT64  fs_base;
  UINT64  gs_base;
  UINT64  ds;
  UINT64  es;
  UINT64  fs;
  UINT64  gs;
} ELF_GREGSET_X86_64;

//
// i386 register set (elf_gregset_t for i386)
//
typedef struct ALIGNED_STRUCT(4) {
  UINT32  ebx;
  UINT32  ecx;
  UINT32  edx;
  UINT32  esi;
  UINT32  edi;
  UINT32  ebp;
  UINT32  eax;
  UINT32  xds;
  UINT32  xes;
  UINT32  xfs;
  UINT32  xgs;
  UINT32  orig_eax;
  UINT32  eip;
  UINT32  xcs;
  UINT32  eflags;
  UINT32  esp;
  UINT32  xss;
} ELF_GREGSET_I386;

//
// ARM64 register set (elf_gregset_t for AArch64)
// Follows Linux kernel's user_pt_regs layout
//
typedef struct ALIGNED_STRUCT(8) {
  UINT64  regs[31];  ///< x0-x30
  UINT64  sp;
  UINT64  pc;
  UINT64  pstate;
  UINT64  orig_x0;   ///< Original x0 value (for syscall restart)
} ELF_GREGSET_ARM64;

//
// ARM 32-bit register set (elf_gregset_t for ARM)
//
typedef struct ALIGNED_STRUCT(4) {
  UINT32  regs[18];  ///< r0-r15, cpsr, orig_r0
} ELF_GREGSET_ARM;

//
// Complete NT_PRSTATUS structure for x86-64
//
typedef struct ALIGNED_STRUCT(8) {
  ELF_PRSTATUS_COMMON  common;
  ELF_GREGSET_X86_64   pr_reg;
  INT32                pr_fpvalid;  ///< True if FPU is used
  INT32                padding;
} ELF_PRSTATUS_X86_64;

//
// Complete NT_PRSTATUS structure for i386
//
typedef struct ALIGNED_STRUCT(4) {
  ELF_PRSTATUS_COMMON  common;
  ELF_GREGSET_I386     pr_reg;
  INT32                pr_fpvalid;
} ELF_PRSTATUS_I386;

//
// Complete NT_PRSTATUS structure for ARM64
//
typedef struct ALIGNED_STRUCT(8) {
  ELF_PRSTATUS_COMMON  common;
  ELF_GREGSET_ARM64    pr_reg;
  INT32                pr_fpvalid;
  INT32                padding;
} ELF_PRSTATUS_ARM64;

//
// Complete NT_PRSTATUS structure for ARM
//
typedef struct ALIGNED_STRUCT(4) {
  ELF_PRSTATUS_COMMON  common;
  ELF_GREGSET_ARM      pr_reg;
  INT32                pr_fpvalid;
} ELF_PRSTATUS_ARM;

//
// Function prototypes for converting between ELF core and CONTEXT structures
//

/**
  Convert x86-64 NT_PRSTATUS to AMD64_CONTEXT.

  @param[in]  Prstatus  Pointer to ELF_PRSTATUS_X86_64 from core dump
  @param[out] Context   Pointer to AMD64_CONTEXT to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfPrstatusToContextX64 (
  IN  CONST ELF_PRSTATUS_X86_64  *Prstatus,
  OUT AMD64_CONTEXT              *Context
  );

/**
  Convert AMD64_CONTEXT to x86-64 NT_PRSTATUS.

  @param[in]  Context   Pointer to AMD64_CONTEXT
  @param[out] Prstatus  Pointer to ELF_PRSTATUS_X86_64 to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfContextToPrstatusX64 (
  IN  CONST AMD64_CONTEXT        *Context,
  OUT ELF_PRSTATUS_X86_64        *Prstatus
  );

/**
  Convert i386 NT_PRSTATUS to I386_CONTEXT.

  @param[in]  Prstatus  Pointer to ELF_PRSTATUS_I386 from core dump
  @param[out] Context   Pointer to I386_CONTEXT to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfPrstatusToContextI386 (
  IN  CONST ELF_PRSTATUS_I386    *Prstatus,
  OUT I386_CONTEXT               *Context
  );

/**
  Convert I386_CONTEXT to i386 NT_PRSTATUS.

  @param[in]  Context   Pointer to I386_CONTEXT
  @param[out] Prstatus  Pointer to ELF_PRSTATUS_I386 to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfContextToPrstatusI386 (
  IN  CONST I386_CONTEXT         *Context,
  OUT ELF_PRSTATUS_I386          *Prstatus
  );

/**
  Convert ARM64 NT_PRSTATUS to ARM64_NT_CONTEXT.

  @param[in]  Prstatus  Pointer to ELF_PRSTATUS_ARM64 from core dump
  @param[out] Context   Pointer to ARM64_NT_CONTEXT to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfPrstatusToContextArm64 (
  IN  CONST ELF_PRSTATUS_ARM64   *Prstatus,
  OUT ARM64_NT_CONTEXT           *Context
  );

/**
  Convert ARM64_NT_CONTEXT to ARM64 NT_PRSTATUS.

  @param[in]  Context   Pointer to ARM64_NT_CONTEXT
  @param[out] Prstatus  Pointer to ELF_PRSTATUS_ARM64 to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfContextToPrstatusArm64 (
  IN  CONST ARM64_NT_CONTEXT     *Context,
  OUT ELF_PRSTATUS_ARM64         *Prstatus
  );

/**
  Convert ARM NT_PRSTATUS to ARM_CONTEXT.

  @param[in]  Prstatus  Pointer to ELF_PRSTATUS_ARM from core dump
  @param[out] Context   Pointer to ARM_CONTEXT to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfPrstatusToContextArm (
  IN  CONST ELF_PRSTATUS_ARM     *Prstatus,
  OUT ARM_CONTEXT                *Context
  );

/**
  Convert ARM_CONTEXT to ARM NT_PRSTATUS.

  @param[in]  Context   Pointer to ARM_CONTEXT
  @param[out] Prstatus  Pointer to ELF_PRSTATUS_ARM to fill

  @retval BINFORMAT_SUCCESS           Successfully converted
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
**/
BINFORMAT_STATUS
ElfContextToPrstatusArm (
  IN  CONST ARM_CONTEXT          *Context,
  OUT ELF_PRSTATUS_ARM           *Prstatus
  );

//
// Core file reading support structures
//

///
/// Opaque handle to a core file reader
///
typedef VOID *ELF_CORE_HANDLE;

///
/// Thread information from a core file
///
typedef struct {
  INT32   ThreadId;      ///< Thread ID (pid)
  INT32   SignalNumber;  ///< Signal that caused the dump
  VOID    *Context;      ///< Pointer to architecture-specific CONTEXT structure
  UINT32  ContextSize;   ///< Size of context structure
} ELF_CORE_THREAD;

//
// Core file reading functions
//

/**
  Read ELF core file and extract thread contexts.

  This function parses an ELF core dump file and extracts register contexts
  for all threads. The contexts are converted from ELF NT_PRSTATUS format
  to the appropriate CONTEXT structures based on the architecture.

  @param[in]  FilePath      Path to the core file
  @param[out] ThreadCount   Number of threads found
  @param[out] Threads       Array of thread information (caller must free)

  @retval BINFORMAT_SUCCESS               Successfully read core file
  @retval BINFORMAT_ERROR_INVALID_PARAMETER NULL pointer
  @retval BINFORMAT_ERROR_IO              Could not read file
  @retval BINFORMAT_ERROR_INVALID_FORMAT  Not a valid ELF core file
  @retval BINFORMAT_ERROR_UNSUPPORTED     Unsupported architecture
**/
BINFORMAT_STATUS
ElfReadCoreFile (
  IN  CONST CHAR8         *FilePath,
  OUT UINT32              *ThreadCount,
  OUT ELF_CORE_THREAD     **Threads
  );

/**
  Free resources allocated by ElfReadCoreFile.

  @param[in]  Threads       Array of threads to free
  @param[in]  ThreadCount   Number of threads in array
**/
VOID
ElfFreeCoreThreads (
  IN  ELF_CORE_THREAD     *Threads,
  IN  UINT32              ThreadCount
  );

#endif // __ELF_CORE_H__
