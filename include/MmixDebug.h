/** @file
  MMIX Debugger interface.

  This file provides the debugger interface for interactive debugging
  of MMIX programs.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef MMIX_DEBUG_H_
#define MMIX_DEBUG_H_

#include "MmixTypes.h"
#include "MmixEmulator.h"

#define MMIX_MAX_BREAKPOINTS  256
#define MMIX_MAX_WATCHPOINTS  64

/**
  Breakpoint types
**/
typedef enum {
  MmixBreakpointNone,
  MmixBreakpointSoftware,
  MmixBreakpointHardware
} MMIX_BREAKPOINT_TYPE;

/**
  Watchpoint types
**/
typedef enum {
  MmixWatchpointRead,
  MmixWatchpointWrite,
  MmixWatchpointAccess
} MMIX_WATCHPOINT_TYPE;

/**
  Breakpoint entry
**/
typedef struct {
  UINT64                  Address;
  MMIX_BREAKPOINT_TYPE    Type;
  BOOLEAN                 Enabled;
  UINT32                  OriginalInstruction;
  UINT64                  HitCount;
} MMIX_BREAKPOINT;

/**
  Watchpoint entry
**/
typedef struct {
  UINT64                Address;
  UINT64                Size;
  MMIX_WATCHPOINT_TYPE  Type;
  BOOLEAN               Enabled;
  UINT64                HitCount;
  UINT64                OldValue;
} MMIX_WATCHPOINT;

/**
  Debug context
**/
typedef struct {
  MMIX_EMULATOR_CONTEXT  *Emulator;
  MMIX_BREAKPOINT        Breakpoints[MMIX_MAX_BREAKPOINTS];
  UINT32                 BreakpointCount;
  MMIX_WATCHPOINT        Watchpoints[MMIX_MAX_WATCHPOINTS];
  UINT32                 WatchpointCount;
  BOOLEAN                SingleStep;
  BOOLEAN                TraceEnabled;
  UINT64                 StepCount;
  CHAR8                  LastCommand[256];
} MMIX_DEBUG_CONTEXT;

/**
  Create a debug context.

  @param[in]      Emulator      Emulator context to debug.
  @param[out]     Context       Pointer to receive debug context.

  @retval MMIX_SUCCESS          Debug context created.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugCreate (
  IN  MMIX_EMULATOR_CONTEXT  *Emulator,
  OUT MMIX_DEBUG_CONTEXT     **Context
  );

/**
  Destroy a debug context.

  @param[in]      Context       Debug context.

**/
VOID
MmixDebugDestroy (
  IN  MMIX_DEBUG_CONTEXT  *Context
  );

/**
  Add a breakpoint.

  @param[in,out]  Context       Debug context.
  @param[in]      Address       Breakpoint address.
  @param[in]      Type          Breakpoint type.

  @retval MMIX_SUCCESS          Breakpoint added.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugAddBreakpoint (
  IN OUT MMIX_DEBUG_CONTEXT    *Context,
  IN     UINT64                Address,
  IN     MMIX_BREAKPOINT_TYPE  Type
  );

/**
  Remove a breakpoint.

  @param[in,out]  Context       Debug context.
  @param[in]      Address       Breakpoint address.

  @retval MMIX_SUCCESS          Breakpoint removed.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugRemoveBreakpoint (
  IN OUT MMIX_DEBUG_CONTEXT  *Context,
  IN     UINT64              Address
  );

/**
  Add a watchpoint.

  @param[in,out]  Context       Debug context.
  @param[in]      Address       Watchpoint address.
  @param[in]      Size          Size to watch.
  @param[in]      Type          Watchpoint type.

  @retval MMIX_SUCCESS          Watchpoint added.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugAddWatchpoint (
  IN OUT MMIX_DEBUG_CONTEXT     *Context,
  IN     UINT64                 Address,
  IN     UINT64                 Size,
  IN     MMIX_WATCHPOINT_TYPE   Type
  );

/**
  Remove a watchpoint.

  @param[in,out]  Context       Debug context.
  @param[in]      Address       Watchpoint address.

  @retval MMIX_SUCCESS          Watchpoint removed.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugRemoveWatchpoint (
  IN OUT MMIX_DEBUG_CONTEXT  *Context,
  IN     UINT64              Address
  );

/**
  Run debugger interactive session.

  @param[in,out]  Context       Debug context.

  @retval MMIX_SUCCESS          Session completed.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugRun (
  IN OUT MMIX_DEBUG_CONTEXT  *Context
  );

/**
  Check if execution should break.

  @param[in,out]  Context       Debug context.

  @return  TRUE if should break, FALSE otherwise.

**/
BOOLEAN
MmixDebugShouldBreak (
  IN OUT MMIX_DEBUG_CONTEXT  *Context
  );

/**
  Print disassembly at current PC.

  @param[in]      Context       Debug context.
  @param[in]      Count         Number of instructions to disassemble.

**/
VOID
MmixDebugDisassemble (
  IN  MMIX_DEBUG_CONTEXT  *Context,
  IN  UINT32              Count
  );

/**
  Print register values.

  @param[in]      Context       Debug context.

**/
VOID
MmixDebugPrintRegisters (
  IN  MMIX_DEBUG_CONTEXT  *Context
  );

/**
  Print memory contents.

  @param[in]      Context       Debug context.
  @param[in]      Address       Start address.
  @param[in]      Size          Number of bytes to print.

**/
VOID
MmixDebugPrintMemory (
  IN  MMIX_DEBUG_CONTEXT  *Context,
  IN  UINT64              Address,
  IN  UINT64              Size
  );

#endif // MMIX_DEBUG_H_
