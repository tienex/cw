/** @file
  MMIX Assembler command-line tool.

  This file implements the command-line interface for the MMIX assembler.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../../include/MmixAsm.h"

/**
  Main assembler entry point.

  @param[in]      argc          Argument count.
  @param[in]      argv          Argument vector.

  @return  Exit status.

**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  MMIX_ASM_CONTEXT  *Context;
  MMIX_STATUS       Status;
  CHAR8             *InputFile;
  CHAR8             *OutputFile;
  INT32             Opt;

  InputFile = NULL;
  OutputFile = (CHAR8 *)"a.out";

  //
  // Parse options
  //
  while ((Opt = getopt (argc, argv, "o:h")) != -1) {
    switch (Opt) {
      case 'o':
        OutputFile = optarg;
        break;
      case 'h':
        printf ("Usage: mmix-as [-o output] input.asm\n");
        printf ("Options:\n");
        printf ("  -o <file>  Output file (default: a.out)\n");
        printf ("  -h         Show this help\n");
        return 0;
      default:
        fprintf (stderr, "Usage: mmix-as [-o output] input.asm\n");
        return 1;
    }
  }

  //
  // Get input file
  //
  if (optind >= argc) {
    fprintf (stderr, "Error: No input file specified\n");
    fprintf (stderr, "Usage: mmix-as [-o output] input.asm\n");
    return 1;
  }

  InputFile = argv[optind];

  //
  // Create assembler context
  //
  Status = MmixAsmCreate (&Context);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Cannot create assembler context\n");
    return 1;
  }

  //
  // Assemble file
  //
  printf ("Assembling: %s\n", InputFile);
  Status = MmixAsmAssembleFile (Context, InputFile);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Assembly failed\n");
    MmixAsmDestroy (Context);
    return 1;
  }

  //
  // Resolve symbols
  //
  Status = MmixAsmResolve (Context);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Symbol resolution failed\n");
    MmixAsmDestroy (Context);
    return 1;
  }

  //
  // Write output
  //
  Status = MmixAsmWriteObject (Context, OutputFile);
  if (Status != MMIX_SUCCESS) {
    fprintf (stderr, "Error: Cannot write output file\n");
    MmixAsmDestroy (Context);
    return 1;
  }

  printf ("Output: %s\n", OutputFile);
  printf ("Assembly successful (%u symbols, %u bytes)\n",
         Context->SymbolCount,
         (UINT32)(Context->SectionCount > 0 ? Context->Sections[0].Size : 0));

  MmixAsmDestroy (Context);
  return 0;
}
