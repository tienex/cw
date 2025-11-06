/** @file
  MMIX Emulator main entry point.

  Simple command-line interface for the MMIX emulator.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/MmixEmulator.h"

/**
  Print usage information.

  @param[in]  ProgramName       Name of the program.

**/
VOID
PrintUsage (
  IN CONST CHAR8  *ProgramName
  )
{
  printf ("MMIX Emulator\n");
  printf ("Usage: %s [options] <image>\n\n", ProgramName);
  printf ("Options:\n");
  printf ("  -m <size>     Set memory size in MB (default: 256)\n");
  printf ("  -pc <addr>    Set initial PC (default: 0x0)\n");
  printf ("  -vl <bytes>   Set vector length in bytes (default: 128)\n");
  printf ("  -step         Single-step mode\n");
  printf ("  -dump         Dump state after execution\n");
  printf ("  -h, --help    Show this help message\n");
  printf ("\n");
}

/**
  Main entry point.

  @param[in]  Argc              Argument count.
  @param[in]  Argv              Argument vector.

  @retval 0                     Success.
  @retval Non-zero              Error.

**/
INT32
main (
  IN INT32  Argc,
  IN CHAR8  **Argv
  )
{
  MMIX_EMULATOR_CONFIG    Config;
  MMIX_EMULATOR_CONTEXT   *Emulator;
  MMIX_STATUS             Status;
  CONST CHAR8             *ImagePath;
  UINT64                  MemorySizeMb;
  BOOLEAN                 StepMode;
  BOOLEAN                 DumpState;
  INT32                   i;

  //
  // Get default configuration
  //
  MmixEmulatorGetDefaultConfig (&Config);

  //
  // Parse command-line arguments
  //
  ImagePath = NULL;
  MemorySizeMb = 256;
  StepMode = FALSE;
  DumpState = FALSE;

  for (i = 1; i < Argc; i++) {
    if (strcmp (Argv[i], "-h") == 0 || strcmp (Argv[i], "--help") == 0) {
      PrintUsage (Argv[0]);
      return 0;
    } else if (strcmp (Argv[i], "-m") == 0 && i + 1 < Argc) {
      MemorySizeMb = strtoull (Argv[++i], NULL, 0);
      Config.MemorySize = MemorySizeMb * 1024 * 1024;
    } else if (strcmp (Argv[i], "-pc") == 0 && i + 1 < Argc) {
      Config.InitialPc = strtoull (Argv[++i], NULL, 0);
    } else if (strcmp (Argv[i], "-vl") == 0 && i + 1 < Argc) {
      Config.VectorLength = strtoul (Argv[++i], NULL, 0);
    } else if (strcmp (Argv[i], "-step") == 0) {
      StepMode = TRUE;
    } else if (strcmp (Argv[i], "-dump") == 0) {
      DumpState = TRUE;
    } else if (Argv[i][0] != '-') {
      ImagePath = Argv[i];
    } else {
      fprintf (stderr, "Unknown option: %s\n", Argv[i]);
      PrintUsage (Argv[0]);
      return 1;
    }
  }

  //
  // Check if image path was provided
  //
  if (ImagePath == NULL) {
    fprintf (stderr, "Error: No image file specified\n\n");
    PrintUsage (Argv[0]);
    return 1;
  }

  //
  // Create emulator
  //
  printf ("Creating MMIX emulator...\n");
  printf ("  Memory: %llu MB\n", (unsigned long long)(Config.MemorySize / (1024 * 1024)));
  printf ("  Initial PC: 0x%llX\n", (unsigned long long)Config.InitialPc);
  printf ("  Vector Length: %u bytes\n", Config.VectorLength);
  printf ("\n");

  Status = MmixEmulatorCreate (&Emulator, &Config);
  if (MMIX_IS_ERROR (Status)) {
    fprintf (stderr, "Error: Failed to create emulator (status = 0x%llX)\n",
             (unsigned long long)Status);
    return 1;
  }

  //
  // Load image
  //
  printf ("Loading image: %s\n", ImagePath);
  Status = MmixEmulatorLoadImage (Emulator, ImagePath, 0);
  if (MMIX_IS_ERROR (Status)) {
    fprintf (stderr, "Error: Failed to load image (status = 0x%llX)\n",
             (unsigned long long)Status);
    MmixEmulatorDestroy (Emulator);
    return 1;
  }

  //
  // Run emulator
  //
  printf ("Starting execution...\n\n");

  if (StepMode) {
    //
    // Single-step mode
    //
    CHAR8  Buffer[256];
    while (TRUE) {
      printf ("PC = 0x%016llX  > ", (unsigned long long)Emulator->CpuState->Pc);
      if (fgets (Buffer, sizeof (Buffer), stdin) == NULL) {
        break;
      }

      if (Buffer[0] == 'q' || Buffer[0] == 'Q') {
        break;
      }

      Status = MmixEmulatorStep (Emulator);
      if (MMIX_IS_ERROR (Status)) {
        fprintf (stderr, "Error during execution (status = 0x%llX)\n",
                 (unsigned long long)Status);
        break;
      }

      if (Emulator->CpuState->ExecutionMode == MmixExecutionModeHalted) {
        printf ("CPU halted\n");
        break;
      }
    }
  } else {
    //
    // Run to completion
    //
    Status = MmixEmulatorRun (Emulator);
    if (MMIX_IS_ERROR (Status)) {
      fprintf (stderr, "Error during execution (status = 0x%llX)\n",
               (unsigned long long)Status);
    } else {
      printf ("Execution completed\n");
    }
  }

  //
  // Dump state if requested
  //
  if (DumpState) {
    printf ("\n");
    MmixEmulatorDumpState (Emulator, NULL);
  }

  //
  // Print statistics
  //
  printf ("\nStatistics:\n");
  printf ("  Instructions executed: %llu\n",
          (unsigned long long)Emulator->CpuState->InstructionCount);
  printf ("  Cycles: %llu\n",
          (unsigned long long)Emulator->CpuState->CycleCount);
  printf ("  TLB hits: %llu\n",
          (unsigned long long)Emulator->MemoryState->TlbHits);
  printf ("  TLB misses: %llu\n",
          (unsigned long long)Emulator->MemoryState->TlbMisses);

  //
  // Clean up
  //
  MmixEmulatorDestroy (Emulator);

  return 0;
}
