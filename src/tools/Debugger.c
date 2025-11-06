/** @file
  MMIX Debugger implementation.

  This file implements an interactive debugger for MMIX programs
  with features like breakpoints, single-stepping, and memory examination.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/MmixDebug.h"
#include "../../include/MmixDisasm.h"
#include "../../include/MmixCore.h"
#include "../../include/MmixMemory.h"

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
  )
{
  MMIX_DEBUG_CONTEXT  *Ctx;

  if (Emulator == NULL || Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Ctx = (MMIX_DEBUG_CONTEXT *)calloc (1, sizeof (MMIX_DEBUG_CONTEXT));
  if (Ctx == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  Ctx->Emulator = Emulator;
  Ctx->SingleStep = FALSE;
  Ctx->TraceEnabled = FALSE;
  Ctx->StepCount = 0;

  *Context = Ctx;
  return MMIX_SUCCESS;
}

/**
  Destroy a debug context.

  @param[in]      Context       Debug context.

**/
VOID
MmixDebugDestroy (
  IN  MMIX_DEBUG_CONTEXT  *Context
  )
{
  if (Context != NULL) {
    free (Context);
  }
}

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
  )
{
  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->BreakpointCount >= MMIX_MAX_BREAKPOINTS) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  MMIX_BREAKPOINT  *Bp = &Context->Breakpoints[Context->BreakpointCount++];
  Bp->Address = Address;
  Bp->Type = Type;
  Bp->Enabled = TRUE;
  Bp->HitCount = 0;

  printf ("Breakpoint %u at 0x%llx\n", Context->BreakpointCount, (unsigned long long)Address);

  return MMIX_SUCCESS;
}

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
  )
{
  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  for (UINT32 i = 0; i < Context->BreakpointCount; i++) {
    if (Context->Breakpoints[i].Address == Address) {
      //
      // Remove by shifting remaining entries
      //
      for (UINT32 j = i; j < Context->BreakpointCount - 1; j++) {
        Context->Breakpoints[j] = Context->Breakpoints[j + 1];
      }
      Context->BreakpointCount--;
      printf ("Breakpoint at 0x%llx removed\n", (unsigned long long)Address);
      return MMIX_SUCCESS;
    }
  }

  return MMIX_ERROR_NOT_FOUND;
}

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
  )
{
  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  if (Context->WatchpointCount >= MMIX_MAX_WATCHPOINTS) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  MMIX_WATCHPOINT  *Wp = &Context->Watchpoints[Context->WatchpointCount++];
  Wp->Address = Address;
  Wp->Size = Size;
  Wp->Type = Type;
  Wp->Enabled = TRUE;
  Wp->HitCount = 0;

  printf ("Watchpoint %u at 0x%llx (size %llu)\n",
         Context->WatchpointCount, (unsigned long long)Address, (unsigned long long)Size);

  return MMIX_SUCCESS;
}

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
  )
{
  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  for (UINT32 i = 0; i < Context->WatchpointCount; i++) {
    if (Context->Watchpoints[i].Address == Address) {
      //
      // Remove by shifting
      //
      for (UINT32 j = i; j < Context->WatchpointCount - 1; j++) {
        Context->Watchpoints[j] = Context->Watchpoints[j + 1];
      }
      Context->WatchpointCount--;
      printf ("Watchpoint at 0x%llx removed\n", (unsigned long long)Address);
      return MMIX_SUCCESS;
    }
  }

  return MMIX_ERROR_NOT_FOUND;
}

/**
  Check if execution should break.

  @param[in,out]  Context       Debug context.

  @return  TRUE if should break, FALSE otherwise.

**/
BOOLEAN
MmixDebugShouldBreak (
  IN OUT MMIX_DEBUG_CONTEXT  *Context
  )
{
  UINT64  Pc;

  if (Context == NULL || Context->Emulator == NULL) {
    return FALSE;
  }

  Pc = Context->Emulator->CpuState->Pc;

  //
  // Check single-step mode
  //
  if (Context->SingleStep) {
    Context->SingleStep = FALSE;
    return TRUE;
  }

  //
  // Check breakpoints
  //
  for (UINT32 i = 0; i < Context->BreakpointCount; i++) {
    if (Context->Breakpoints[i].Enabled && Context->Breakpoints[i].Address == Pc) {
      Context->Breakpoints[i].HitCount++;
      printf ("\nBreakpoint %u hit at 0x%llx (hit count: %llu)\n",
             i + 1, (unsigned long long)Pc, (unsigned long long)Context->Breakpoints[i].HitCount);
      return TRUE;
    }
  }

  return FALSE;
}

/**
  Print disassembly at current PC.

  @param[in]      Context       Debug context.
  @param[in]      Count         Number of instructions to disassemble.

**/
VOID
MmixDebugDisassemble (
  IN  MMIX_DEBUG_CONTEXT  *Context,
  IN  UINT32              Count
  )
{
  UINT64       Pc;
  UINT32       Instruction;
  CHAR8        AsmBuffer[128];
  MMIX_STATUS  Status;
  UINT32       InstSize;

  if (Context == NULL || Context->Emulator == NULL) {
    return;
  }

  Pc = Context->Emulator->CpuState->Pc;

  for (UINT32 i = 0; i < Count; i++) {
    //
    // Read instruction from memory
    //
    Status = MmixMemoryRead (Context->Emulator, Pc, 4, (UINT8 *)&Instruction);
    if (Status != MMIX_SUCCESS) {
      printf ("  0x%016llx: <invalid>\n", (unsigned long long)Pc);
      break;
    }

    //
    // Disassemble
    //
    Status = MmixDisassemble (Instruction, Pc, AsmBuffer, sizeof (AsmBuffer));
    if (Status == MMIX_SUCCESS) {
      printf ("  0x%016llx: %s\n", (unsigned long long)Pc, AsmBuffer);
      InstSize = MmixGetInstructionSize (Instruction);
      Pc += InstSize;
    } else {
      printf ("  0x%016llx: <decode error>\n", (unsigned long long)Pc);
      Pc += 4;
    }
  }
}

/**
  Print register values.

  @param[in]      Context       Debug context.

**/
VOID
MmixDebugPrintRegisters (
  IN  MMIX_DEBUG_CONTEXT  *Context
  )
{
  if (Context == NULL || Context->Emulator == NULL) {
    return;
  }

  MMIX_CPU_STATE  *Cpu = Context->Emulator->CpuState;

  printf ("PC:  0x%016llx\n", (unsigned long long)Cpu->Pc);
  printf ("\nGeneral Registers:\n");

  for (UINT32 i = 0; i < 32; i += 4) {
    printf ("  $%-2u: 0x%016llx  $%-2u: 0x%016llx  $%-2u: 0x%016llx  $%-2u: 0x%016llx\n",
           i,     (unsigned long long)Cpu->GeneralRegisters[i],
           i + 1, (unsigned long long)Cpu->GeneralRegisters[i + 1],
           i + 2, (unsigned long long)Cpu->GeneralRegisters[i + 2],
           i + 3, (unsigned long long)Cpu->GeneralRegisters[i + 3]);
  }

  printf ("\nSpecial Registers:\n");
  printf ("  rA:  0x%016llx  rB:  0x%016llx  rC:  0x%016llx\n",
         (unsigned long long)Cpu->SpecialRegisters[0],
         (unsigned long long)Cpu->SpecialRegisters[1],
         (unsigned long long)Cpu->SpecialRegisters[2]);
}

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
  )
{
  UINT8        Byte;
  MMIX_STATUS  Status;

  if (Context == NULL || Context->Emulator == NULL) {
    return;
  }

  for (UINT64 i = 0; i < Size; i += 16) {
    printf ("0x%016llx: ", (unsigned long long)(Address + i));

    //
    // Hex bytes
    //
    for (UINT64 j = 0; j < 16 && (i + j) < Size; j++) {
      Status = MmixMemoryRead (Context->Emulator, Address + i + j, 1, &Byte);
      if (Status == MMIX_SUCCESS) {
        printf ("%02x ", Byte);
      } else {
        printf ("?? ");
      }
      if (j == 7) {
        printf (" ");
      }
    }

    //
    // ASCII
    //
    printf (" |");
    for (UINT64 j = 0; j < 16 && (i + j) < Size; j++) {
      Status = MmixMemoryRead (Context->Emulator, Address + i + j, 1, &Byte);
      if (Status == MMIX_SUCCESS) {
        printf ("%c", (Byte >= 32 && Byte < 127) ? Byte : '.');
      } else {
        printf ("?");
      }
    }
    printf ("|\n");
  }
}

/**
  Process debug command.

  @param[in,out]  Context       Debug context.
  @param[in]      Command       Command string.

  @return  TRUE to continue debugging, FALSE to exit.

**/
STATIC
BOOLEAN
ProcessCommand (
  IN OUT MMIX_DEBUG_CONTEXT  *Context,
  IN     CONST CHAR8         *Command
  )
{
  CHAR8  Cmd[256];
  CHAR8  Arg1[256];
  CHAR8  Arg2[256];

  //
  // Parse command
  //
  INT32  Args = sscanf (Command, "%s %s %s", Cmd, Arg1, Arg2);
  if (Args < 1) {
    //
    // Repeat last command
    //
    if (Context->LastCommand[0] != '\0') {
      return ProcessCommand (Context, Context->LastCommand);
    }
    return TRUE;
  }

  //
  // Save command
  //
  strncpy (Context->LastCommand, Command, sizeof (Context->LastCommand) - 1);

  //
  // Process command
  //
  if (strcmp (Cmd, "help") == 0 || strcmp (Cmd, "h") == 0 || strcmp (Cmd, "?") == 0) {
    printf ("Commands:\n");
    printf ("  break <addr>     - Set breakpoint\n");
    printf ("  delete <addr>    - Delete breakpoint\n");
    printf ("  watch <addr>     - Set watchpoint\n");
    printf ("  continue (c)     - Continue execution\n");
    printf ("  step (s)         - Single step\n");
    printf ("  next (n)         - Step over\n");
    printf ("  disasm <count>   - Disassemble instructions\n");
    printf ("  registers (r)    - Print registers\n");
    printf ("  examine <addr>   - Examine memory\n");
    printf ("  quit (q)         - Quit debugger\n");
    printf ("  help (h)         - Show this help\n");
  } else if (strcmp (Cmd, "break") == 0 || strcmp (Cmd, "b") == 0) {
    if (Args >= 2) {
      UINT64  Addr = strtoull (Arg1, NULL, 0);
      MmixDebugAddBreakpoint (Context, Addr, MmixBreakpointSoftware);
    } else {
      printf ("Usage: break <address>\n");
    }
  } else if (strcmp (Cmd, "delete") == 0 || strcmp (Cmd, "d") == 0) {
    if (Args >= 2) {
      UINT64  Addr = strtoull (Arg1, NULL, 0);
      MmixDebugRemoveBreakpoint (Context, Addr);
    } else {
      printf ("Usage: delete <address>\n");
    }
  } else if (strcmp (Cmd, "watch") == 0 || strcmp (Cmd, "w") == 0) {
    if (Args >= 2) {
      UINT64  Addr = strtoull (Arg1, NULL, 0);
      MmixDebugAddWatchpoint (Context, Addr, 8, MmixWatchpointAccess);
    } else {
      printf ("Usage: watch <address>\n");
    }
  } else if (strcmp (Cmd, "continue") == 0 || strcmp (Cmd, "c") == 0) {
    return FALSE;
  } else if (strcmp (Cmd, "step") == 0 || strcmp (Cmd, "s") == 0) {
    Context->SingleStep = TRUE;
    return FALSE;
  } else if (strcmp (Cmd, "next") == 0 || strcmp (Cmd, "n") == 0) {
    Context->SingleStep = TRUE;
    return FALSE;
  } else if (strcmp (Cmd, "disasm") == 0 || strcmp (Cmd, "dis") == 0) {
    UINT32  Count = (Args >= 2) ? atoi (Arg1) : 10;
    MmixDebugDisassemble (Context, Count);
  } else if (strcmp (Cmd, "registers") == 0 || strcmp (Cmd, "r") == 0 || strcmp (Cmd, "reg") == 0) {
    MmixDebugPrintRegisters (Context);
  } else if (strcmp (Cmd, "examine") == 0 || strcmp (Cmd, "x") == 0) {
    if (Args >= 2) {
      UINT64  Addr = strtoull (Arg1, NULL, 0);
      UINT64  Size = (Args >= 3) ? strtoull (Arg2, NULL, 0) : 64;
      MmixDebugPrintMemory (Context, Addr, Size);
    } else {
      printf ("Usage: examine <address> [size]\n");
    }
  } else if (strcmp (Cmd, "quit") == 0 || strcmp (Cmd, "q") == 0) {
    exit (0);
  } else {
    printf ("Unknown command: %s (type 'help' for commands)\n", Cmd);
  }

  return TRUE;
}

/**
  Run debugger interactive session.

  @param[in,out]  Context       Debug context.

  @retval MMIX_SUCCESS          Session completed.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixDebugRun (
  IN OUT MMIX_DEBUG_CONTEXT  *Context
  )
{
  CHAR8  CommandLine[256];

  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  printf ("MMIX Debugger\n");
  printf ("Type 'help' for commands\n\n");

  //
  // Show current location
  //
  printf ("Stopped at 0x%016llx\n", (unsigned long long)Context->Emulator->CpuState->Pc);
  MmixDebugDisassemble (Context, 5);

  //
  // Interactive loop
  //
  while (1) {
    printf ("(mmix-db) ");
    fflush (stdout);

    if (fgets (CommandLine, sizeof (CommandLine), stdin) == NULL) {
      break;
    }

    //
    // Remove newline
    //
    size_t  Len = strlen (CommandLine);
    if (Len > 0 && CommandLine[Len - 1] == '\n') {
      CommandLine[Len - 1] = '\0';
    }

    //
    // Process command
    //
    if (!ProcessCommand (Context, CommandLine)) {
      break;
    }
  }

  return MMIX_SUCCESS;
}
