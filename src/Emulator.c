/** @file
  MMIX Emulator main implementation.

  This file implements the top-level emulator context management and
  main execution loop.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/MmixEmulator.h"

/**
  Create and initialize a new emulator instance.

  Allocates and initializes all emulator components including CPU,
  memory, and devices according to the configuration.

  @param[out]  Context           Pointer to receive emulator context.
  @param[in]   Config            Pointer to configuration structure.

  @retval MMIX_SUCCESS           Emulator created successfully.
  @retval MMIX_ERROR_OUT_OF_MEMORY  Failed to allocate memory.
  @retval MMIX_ERROR_INVALID_PARAMETER  Invalid configuration.

**/
MMIX_STATUS
MmixEmulatorCreate (
  OUT MMIX_EMULATOR_CONTEXT  **Context,
  IN  MMIX_EMULATOR_CONFIG   *Config
  )
{
  MMIX_EMULATOR_CONTEXT  *Emulator;
  MMIX_STATUS            Status;

  if (Context == NULL || Config == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate emulator context
  //
  Emulator = (MMIX_EMULATOR_CONTEXT *)malloc (sizeof (MMIX_EMULATOR_CONTEXT));
  if (Emulator == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  memset (Emulator, 0, sizeof (MMIX_EMULATOR_CONTEXT));

  //
  // Copy configuration
  //
  memcpy (&Emulator->Config, Config, sizeof (MMIX_EMULATOR_CONFIG));

  //
  // Initialize memory subsystem
  //
  Status = MmixMemoryInitialize (
             &Emulator->MemoryState,
             Config->MemorySize
             );
  if (MMIX_IS_ERROR (Status)) {
    free (Emulator);
    return Status;
  }

  //
  // Initialize CPU
  //
  Status = MmixCpuInitialize (
             &Emulator->CpuState,
             Config->InitialPc,
             Emulator->MemoryState,
             Config->VectorLength
             );
  if (MMIX_IS_ERROR (Status)) {
    MmixMemoryDestroy (Emulator->MemoryState);
    free (Emulator);
    return Status;
  }

  //
  // Link CPU and memory
  //
  Emulator->MemoryState->CpuState = Emulator->CpuState;
  Emulator->CpuState->EmulatorContext = Emulator;

  //
  // Initialize device array (empty for now)
  //
  Emulator->Devices = NULL;
  Emulator->DeviceCount = 0;

  //
  // Set initial state
  //
  Emulator->Running = FALSE;
  Emulator->ExitRequested = FALSE;
  Emulator->ExitCode = 0;

  *Context = Emulator;
  return MMIX_SUCCESS;
}

/**
  Run the emulator.

  Begins execution at the configured initial PC. Continues until
  an error occurs, the emulator is halted, or the exit is requested.

  @param[in,out]  Context       Emulator context.

  @retval MMIX_SUCCESS          Emulator exited normally.
  @retval Others                Error occurred during execution.

**/
MMIX_STATUS
MmixEmulatorRun (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context
  )
{
  MMIX_STATUS  Status;

  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Context->Running = TRUE;

  //
  // Main execution loop
  //
  while (Context->Running && !Context->ExitRequested) {
    //
    // Execute one instruction
    //
    Status = MmixCpuExecuteInstruction (Context->CpuState);
    if (MMIX_IS_ERROR (Status)) {
      Context->Running = FALSE;
      return Status;
    }

    //
    // Check if CPU is halted
    //
    if (Context->CpuState->ExecutionMode == MmixExecutionModeHalted) {
      Context->Running = FALSE;
      break;
    }
  }

  return MMIX_SUCCESS;
}

/**
  Single-step the emulator.

  Executes exactly one instruction and returns.

  @param[in,out]  Context       Emulator context.

  @retval MMIX_SUCCESS          Instruction executed successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixEmulatorStep (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  return MmixCpuExecuteInstruction (Context->CpuState);
}

/**
  Reset the emulator.

  Resets CPU and device state to initial values. Memory contents
  are preserved.

  @param[in,out]  Context       Emulator context.

  @retval MMIX_SUCCESS          Emulator reset successfully.

**/
MMIX_STATUS
MmixEmulatorReset (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context
  )
{
  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  Context->ExitRequested = FALSE;
  Context->ExitCode = 0;

  return MmixCpuReset (Context->CpuState);
}

/**
  Load a binary image into memory.

  Loads a binary file into emulator memory at the specified address.
  Useful for loading bootloaders or kernels.

  @param[in,out]  Context       Emulator context.
  @param[in]      ImagePath     Path to binary image file.
  @param[in]      LoadAddress   Physical address to load at.

  @retval MMIX_SUCCESS          Image loaded successfully.
  @retval MMIX_ERROR_NOT_FOUND  Image file not found.
  @retval Others                Error occurred during load.

**/
MMIX_STATUS
MmixEmulatorLoadImage (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *ImagePath,
  IN     UINT64                 LoadAddress
  )
{
  FILE    *ImageFile;
  UINT64  FileSize;
  UINT64  BytesRead;

  if (Context == NULL || ImagePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Open image file
  //
  ImageFile = fopen (ImagePath, "rb");
  if (ImageFile == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Get file size
  //
  fseek (ImageFile, 0, SEEK_END);
  FileSize = ftell (ImageFile);
  fseek (ImageFile, 0, SEEK_SET);

  //
  // Check if image fits in memory
  //
  if (LoadAddress + FileSize > Context->MemoryState->PhysicalMemorySize) {
    fclose (ImageFile);
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  //
  // Read image into memory
  //
  BytesRead = fread (
                &Context->MemoryState->PhysicalMemory[LoadAddress],
                1,
                FileSize,
                ImageFile
                );

  fclose (ImageFile);

  if (BytesRead != FileSize) {
    return MMIX_ERROR_DEVICE_ERROR;
  }

  return MMIX_SUCCESS;
}

/**
  Request emulator exit.

  Signals the emulator to exit at the next safe point with the
  specified exit code.

  @param[in,out]  Context       Emulator context.
  @param[in]      ExitCode      Exit code value.

**/
VOID
MmixEmulatorRequestExit (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     UINT64                 ExitCode
  )
{
  if (Context != NULL) {
    Context->ExitRequested = TRUE;
    Context->ExitCode = ExitCode;
  }
}

/**
  Destroy an emulator instance.

  Frees all resources associated with the emulator including CPU,
  memory, and devices.

  @param[in]  Context           Emulator context to destroy.

**/
VOID
MmixEmulatorDestroy (
  IN MMIX_EMULATOR_CONTEXT  *Context
  )
{
  UINT32  i;

  if (Context == NULL) {
    return;
  }

  //
  // Destroy devices
  //
  if (Context->Devices != NULL) {
    for (i = 0; i < Context->DeviceCount; i++) {
      MmixDeviceDestroy (Context->Devices[i]);
    }
    free (Context->Devices);
  }

  //
  // Destroy CPU
  //
  if (Context->CpuState != NULL) {
    MmixCpuDestroy (Context->CpuState);
  }

  //
  // Destroy memory
  //
  if (Context->MemoryState != NULL) {
    MmixMemoryDestroy (Context->MemoryState);
  }

  //
  // Free context
  //
  free (Context);
}

/**
  Get default emulator configuration.

  Returns a configuration structure with reasonable default values.

  @param[out]  Config           Pointer to receive default configuration.

**/
VOID
MmixEmulatorGetDefaultConfig (
  OUT MMIX_EMULATOR_CONFIG  *Config
  )
{
  if (Config == NULL) {
    return;
  }

  memset (Config, 0, sizeof (MMIX_EMULATOR_CONFIG));

  Config->MemorySize = 256 * 1024 * 1024;  // 256 MB
  Config->InitialPc = 0x0;
  Config->VectorLength = 128;  // 128 bytes (1024 bits)
  Config->EnableHypervisor = FALSE;
  Config->EnableVectorExtensions = TRUE;
  Config->EnableMatrixExtensions = TRUE;
  Config->EnableCompressedInstructions = TRUE;
  Config->EnableFramebuffer = FALSE;
  Config->FramebufferWidth = 1024;
  Config->FramebufferHeight = 768;
  Config->EnableNetwork = FALSE;
  Config->EnableSerial = FALSE;
  Config->SerialPortCount = 0;
}

/**
  Raise an interrupt.

  Raises an interrupt on the emulated CPU. The interrupt will be
  delivered at the next opportunity based on interrupt masking.

  @param[in,out]  Context       Emulator context.
  @param[in]      InterruptNum  Interrupt number (0-63).

  @retval MMIX_SUCCESS          Interrupt raised successfully.

**/
MMIX_STATUS
MmixEmulatorRaiseInterrupt (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     UINT8                  InterruptNum
  )
{
  if (Context == NULL || InterruptNum >= 64) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Set interrupt pending bit
  //
  Context->CpuState->InterruptsPending |= (1ULL << InterruptNum);

  return MMIX_SUCCESS;
}

/**
  Dump emulator state.

  Dumps CPU registers, memory, and device state to a file or stdout
  for debugging purposes.

  @param[in]  Context           Emulator context.
  @param[in]  OutputPath        Path to output file (NULL for stdout).

  @retval MMIX_SUCCESS          State dumped successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixEmulatorDumpState (
  IN MMIX_EMULATOR_CONTEXT  *Context,
  IN CONST CHAR8            *OutputPath OPTIONAL
  )
{
  FILE    *Output;
  UINT32  i;
  UINT64  Value;

  if (Context == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Open output file or use stdout
  //
  if (OutputPath != NULL) {
    Output = fopen (OutputPath, "w");
    if (Output == NULL) {
      return MMIX_ERROR_DEVICE_ERROR;
    }
  } else {
    Output = stdout;
  }

  //
  // Dump CPU state
  //
  fprintf (Output, "MMIX Emulator State Dump\n");
  fprintf (Output, "========================\n\n");
  fprintf (Output, "PC: 0x%016llX\n", (unsigned long long)Context->CpuState->Pc);
  fprintf (Output, "Privilege: %d\n", Context->CpuState->PrivilegeLevel);
  fprintf (Output, "Execution Mode: %d\n", Context->CpuState->ExecutionMode);
  fprintf (Output, "Instruction Count: %llu\n", (unsigned long long)Context->CpuState->InstructionCount);
  fprintf (Output, "Cycle Count: %llu\n\n", (unsigned long long)Context->CpuState->CycleCount);

  //
  // Dump general registers (first 32 for brevity)
  //
  fprintf (Output, "General Registers:\n");
  for (i = 0; i < 32; i++) {
    MmixCpuReadRegister (Context->CpuState, i, &Value);
    fprintf (Output, "  $%-3d = 0x%016llX", i, (unsigned long long)Value);
    if ((i % 2) == 1) {
      fprintf (Output, "\n");
    } else {
      fprintf (Output, "    ");
    }
  }
  fprintf (Output, "\n");

  //
  // Dump special registers
  //
  fprintf (Output, "Special Registers:\n");
  fprintf (Output, "  rA  = 0x%016llX    rG  = 0x%016llX\n",
           (unsigned long long)Context->CpuState->SpecialRegisters[MMIX_rA],
           (unsigned long long)Context->CpuState->SpecialRegisters[MMIX_rG]);
  fprintf (Output, "  rL  = 0x%016llX    rT  = 0x%016llX\n",
           (unsigned long long)Context->CpuState->SpecialRegisters[MMIX_rL],
           (unsigned long long)Context->CpuState->SpecialRegisters[MMIX_rT]);

  //
  // Dump TLB statistics
  //
  fprintf (Output, "\nTLB Statistics:\n");
  fprintf (Output, "  Hits: %llu\n", (unsigned long long)Context->MemoryState->TlbHits);
  fprintf (Output, "  Misses: %llu\n", (unsigned long long)Context->MemoryState->TlbMisses);

  //
  // Close file if we opened one
  //
  if (OutputPath != NULL) {
    fclose (Output);
  }

  return MMIX_SUCCESS;
}
