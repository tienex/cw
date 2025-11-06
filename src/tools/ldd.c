/** @file
  ldd - List dynamic dependencies.

  Universal dynamic dependency lister that works across formats:
  ELF (Linux/BSD), Mach-O (macOS), PE (Windows).

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include "binformat/BinFormat.h"
#include "binformat/LibElf.h"
#include "binformat/LibCoff.h"
#include "binformat/LibMacho.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct {
  BOOLEAN  Verbose;        ///< Verbose output
  BOOLEAN  DataRelocations;///< Print data relocations
  BOOLEAN  FunctionRelocations; ///< Print function relocations
  BOOLEAN  Unused;         ///< Print unused direct dependencies
  BOOLEAN  Versions;       ///< Print symbol versions
} LDD_OPTIONS;

STATIC LDD_OPTIONS  gOptions = {0};

/**
  Print usage information.
**/
STATIC
VOID
PrintUsage (
  CONST CHAR8  *ProgramName
  )
{
  printf ("Usage: %s [options] file...\n", ProgramName);
  printf ("List dynamic shared objects required by program (universal: ELF, PE, Mach-O)\n\n");
  printf ("Options:\n");
  printf ("  -v, --verbose              Verbose mode\n");
  printf ("  -u, --unused               Print unused direct dependencies\n");
  printf ("  -d, --data-relocs          Process data relocations\n");
  printf ("  -r, --function-relocs      Process function relocations\n");
  printf ("      --version              Print version information\n");
  printf ("  -h, --help                 Display this information\n\n");
  printf ("Notes:\n");
  printf ("  - On ELF: reads DT_NEEDED entries from .dynamic section\n");
  printf ("  - On Mach-O: reads LC_LOAD_DYLIB load commands\n");
  printf ("  - On PE: reads import directory table\n");
  printf ("  - Does not resolve actual library paths (use actual ldd/dyld for that)\n");
}

/**
  Process ELF file dynamic section.
**/
STATIC
VOID
ProcessElfDynamic (
  IN CONST CHAR8          *FileName,
  IN BINFORMAT_CONTEXT    *Context
  )
{
  printf ("\t(Dynamic section parsing not yet fully implemented)\n");
  printf ("\tlibrary dependencies would appear here\n");
}

/**
  Process Mach-O load commands.
**/
STATIC
VOID
ProcessMachoLoadCommands (
  IN CONST CHAR8          *FileName,
  IN BINFORMAT_CONTEXT    *Context
  )
{
  printf ("\t(Load command parsing not yet fully implemented)\n");
  printf ("\tdylib dependencies would appear here\n");
}

/**
  Process PE import directory.
**/
STATIC
VOID
ProcessPeImports (
  IN CONST CHAR8          *FileName,
  IN BINFORMAT_CONTEXT    *Context
  )
{
  printf ("\t(Import directory parsing not yet fully implemented)\n");
  printf ("\tDLL dependencies would appear here\n");
}

/**
  Process a single file.
**/
STATIC
BINFORMAT_STATUS
ProcessFile (
  IN CONST CHAR8  *FileName
  )
{
  BINFORMAT_CONTEXT  Context;
  BINFORMAT_STATUS   Status;
  BOOLEAN            IsElf;
  BOOLEAN            IsMacho;
  BOOLEAN            IsPe;

  IsElf   = FALSE;
  IsMacho = FALSE;
  IsPe    = FALSE;

  //
  // Try ELF
  //
  Status = ElfGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    IsElf = TRUE;
    goto ProcessDependencies;
  }

  //
  // Try Mach-O
  //
  Status = MachoGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    IsMacho = TRUE;
    goto ProcessDependencies;
  }

  //
  // Try PE/COFF
  //
  Status = CoffGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    IsPe = TRUE;
    goto ProcessDependencies;
  }

  fprintf (stderr, "ldd: %s: not a dynamic executable\n", FileName);
  return BINFORMAT_ERROR_INVALID_FORMAT;

ProcessDependencies:
  if (gOptions.Verbose) {
    printf ("%s:\n", FileName);
    printf ("\tFormat: %s\n", Context.Api->LibraryName);
  }

  //
  // Process based on format
  //
  if (IsElf) {
    ProcessElfDynamic (FileName, &Context);
  } else if (IsMacho) {
    ProcessMachoLoadCommands (FileName, &Context);
  } else if (IsPe) {
    ProcessPeImports (FileName, &Context);
  }

  //
  // For now, show a message that this is a stub implementation
  //
  printf ("\nNote: This is a stub implementation. Full dynamic dependency resolution\n");
  printf ("requires reading and parsing dynamic sections/load commands/import tables.\n");
  printf ("The binary format libraries provide the foundation, but the dynamic linker\n");
  printf ("logic needs to be implemented.\n\n");
  printf ("For production use:\n");
  printf ("  - ELF: Use system 'ldd' or 'readelf -d'\n");
  printf ("  - Mach-O: Use 'otool -L' or 'dyld_info'\n");
  printf ("  - PE: Use 'dumpbin /dependents' or 'objdump -p'\n");

  Context.Api->Close (&Context);
  return BINFORMAT_SUCCESS;
}

/**
  Main entry point.
**/
INT32
main (
  IN INT32   argc,
  IN CHAR8   **argv
  )
{
  INT32    opt;
  INT32    i;
  BOOLEAN  Success;

  STATIC struct option long_options[] = {
    {"verbose",        no_argument, 0, 'v'},
    {"unused",         no_argument, 0, 'u'},
    {"data-relocs",    no_argument, 0, 'd'},
    {"function-relocs",no_argument, 0, 'r'},
    {"version",        no_argument, 0, 'V'},
    {"help",           no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  Success = TRUE;

  //
  // Parse options
  //
  while ((opt = getopt_long (argc, argv, "vudrVh", long_options, NULL)) != -1) {
    switch (opt) {
      case 'v':
        gOptions.Verbose = TRUE;
        break;
      case 'u':
        gOptions.Unused = TRUE;
        break;
      case 'd':
        gOptions.DataRelocations = TRUE;
        break;
      case 'r':
        gOptions.FunctionRelocations = TRUE;
        break;
      case 'V':
        printf ("ldd (MMIX toolchain) version 1.0\n");
        printf ("Universal dynamic dependency lister for ELF, PE, Mach-O\n");
        return 0;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      default:
        PrintUsage (argv[0]);
        return 1;
    }
  }

  //
  // Process files
  //
  if (optind >= argc) {
    fprintf (stderr, "ldd: missing file arguments\n");
    PrintUsage (argv[0]);
    return 1;
  }

  for (i = optind; i < argc; i++) {
    if (ProcessFile (argv[i]) != BINFORMAT_SUCCESS) {
      Success = FALSE;
    }

    if (i < argc - 1) {
      printf ("\n");
    }
  }

  return Success ? 0 : 1;
}
