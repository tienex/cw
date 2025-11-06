/** @file
  lipo - Create and operate on universal (fat) binaries.

  Universal binary tool for creating and manipulating multi-architecture binaries.
  Works with Mach-O fat binaries and FatELF.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include "binformat/BinFormat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef enum {
  LIPO_ACTION_NONE,
  LIPO_ACTION_INFO,
  LIPO_ACTION_DETAILED_INFO,
  LIPO_ACTION_CREATE,
  LIPO_ACTION_THIN,
  LIPO_ACTION_EXTRACT,
  LIPO_ACTION_REMOVE,
  LIPO_ACTION_REPLACE,
  LIPO_ACTION_VERIFY_ARCH,
  LIPO_ACTION_ARCHS
} LIPO_ACTION;

typedef struct {
  LIPO_ACTION  Action;
  CHAR8        *Output;
  CHAR8        *ThinArch;
  CHAR8        *ExtractArch;
  CHAR8        *RemoveArch;
  CHAR8        *ReplaceArch;
  CHAR8        *ReplaceFile;
  CHAR8        *VerifyArch;
} LIPO_OPTIONS;

STATIC LIPO_OPTIONS  gOptions = {0};

/**
  Print usage information.
**/
STATIC
VOID
PrintUsage (
  CONST CHAR8  *ProgramName
  )
{
  printf ("Usage: %s [options] input_file(s)\n", ProgramName);
  printf ("Create and operate on universal (fat) binaries\n\n");
  printf ("Options:\n");
  printf ("  -create                     Create universal binary from input files\n");
  printf ("  -info                       Display brief architecture information\n");
  printf ("  -detailed_info              Display detailed architecture information\n");
  printf ("  -archs                      Display architecture names only\n");
  printf ("  -thin <arch>                Extract single architecture\n");
  printf ("  -extract <arch>             Extract single architecture (same as -thin)\n");
  printf ("  -remove <arch>              Remove architecture\n");
  printf ("  -replace <arch> <file>      Replace architecture with file\n");
  printf ("  -verify_arch <arch>...      Verify architectures present\n");
  printf ("  -output <file>              Output file name (required for creation)\n");
  printf ("  -segalign <arch> <value>    Set segment alignment\n");
  printf ("  -h, --help                  Display this information\n");
  printf ("  -V, --version               Display version information\n\n");
  printf ("Architectures:\n");
  printf ("  i386, x86_64, arm, arm64, arm64e, arm64_32, ppc, ppc64\n\n");
  printf ("Examples:\n");
  printf ("  %s -create -arch i386 file32 -arch x86_64 file64 -output universal\n", ProgramName);
  printf ("  %s -info universal\n", ProgramName);
  printf ("  %s -thin x86_64 universal -output file64\n", ProgramName);
}

/**
  Display architecture information.
**/
STATIC
VOID
DisplayInfo (
  IN CONST CHAR8  *FileName,
  IN BOOLEAN      Detailed
  )
{
  FILE   *File;
  UINT32 Magic;

  File = fopen (FileName, "rb");
  if (File == NULL) {
    fprintf (stderr, "lipo: can't open input file %s\n", FileName);
    return;
  }

  fread (&Magic, sizeof (Magic), 1, File);
  fclose (File);

  if (Magic == MACHO_FAT_MAGIC || Magic == MACHO_FAT_MAGIC_SWAP ||
      Magic == MACHO_FAT_MAGIC_64 || Magic == MACHO_FAT_MAGIC_64_SWAP) {
    if (Detailed) {
      printf ("Fat header for: %s\n", FileName);
      printf ("Fat header magic: 0x%08x\n", Magic);
      printf ("Number of architectures: (parsing not yet implemented)\n");
      printf ("\nArchitecture details:\n");
      printf ("  (would list each architecture with offset, size, align)\n");
    } else {
      printf ("Architectures in the fat file: %s are: (parsing not yet implemented)\n", FileName);
      printf ("(would list: i386 x86_64 arm64 etc.)\n");
    }
  } else if (Magic == ELF_MAGIC_0 || Magic == FATELF_MAGIC) {
    printf ("FatELF support not yet implemented\n");
  } else {
    printf ("Non-fat file: %s is architecture: (detection not yet implemented)\n", FileName);
  }

  printf ("\nNote: This is a stub implementation. Full fat binary support requires:\n");
  printf ("  - Parsing fat_header and fat_arch structures\n");
  printf ("  - Extracting/combining individual architectures\n");
  printf ("  - Managing alignment and offsets\n");
  printf ("\nFor production use, use the actual macOS 'lipo' tool.\n");
}

/**
  Create universal binary.
**/
STATIC
INT32
CreateUniversal (
  IN CHAR8  **InputFiles,
  IN INT32  NumFiles,
  IN CHAR8  *OutputFile
  )
{
  INT32  i;

  if (OutputFile == NULL) {
    fprintf (stderr, "lipo: -output must be specified\n");
    return 1;
  }

  printf ("Creating universal binary: %s\n", OutputFile);
  printf ("From files:\n");

  for (i = 0; i < NumFiles; i++) {
    printf ("  %s\n", InputFiles[i]);
  }

  printf ("\nNote: Universal binary creation not yet implemented.\n");
  printf ("This requires:\n");
  printf ("  1. Reading each input architecture\n");
  printf ("  2. Creating fat_header with architecture count\n");
  printf ("  3. Creating fat_arch entry for each architecture\n");
  printf ("  4. Writing aligned architecture slices\n");
  printf ("\nFor production use, use the actual macOS 'lipo' tool.\n");

  return 1;
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
  INT32   i;
  CHAR8   **InputFiles;
  INT32   NumInputFiles;

  if (argc < 2) {
    PrintUsage (argv[0]);
    return 1;
  }

  InputFiles    = malloc (sizeof (CHAR8 *) * argc);
  NumInputFiles = 0;

  //
  // Parse arguments
  //
  for (i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-create") == 0) {
      gOptions.Action = LIPO_ACTION_CREATE;
    } else if (strcmp (argv[i], "-info") == 0) {
      gOptions.Action = LIPO_ACTION_INFO;
    } else if (strcmp (argv[i], "-detailed_info") == 0) {
      gOptions.Action = LIPO_ACTION_DETAILED_INFO;
    } else if (strcmp (argv[i], "-archs") == 0) {
      gOptions.Action = LIPO_ACTION_ARCHS;
    } else if (strcmp (argv[i], "-thin") == 0 || strcmp (argv[i], "-extract") == 0) {
      if (i + 1 >= argc) {
        fprintf (stderr, "lipo: -thin requires architecture argument\n");
        free (InputFiles);
        return 1;
      }
      gOptions.Action   = LIPO_ACTION_THIN;
      gOptions.ThinArch = argv[++i];
    } else if (strcmp (argv[i], "-remove") == 0) {
      if (i + 1 >= argc) {
        fprintf (stderr, "lipo: -remove requires architecture argument\n");
        free (InputFiles);
        return 1;
      }
      gOptions.Action     = LIPO_ACTION_REMOVE;
      gOptions.RemoveArch = argv[++i];
    } else if (strcmp (argv[i], "-output") == 0) {
      if (i + 1 >= argc) {
        fprintf (stderr, "lipo: -output requires file argument\n");
        free (InputFiles);
        return 1;
      }
      gOptions.Output = argv[++i];
    } else if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
      PrintUsage (argv[0]);
      free (InputFiles);
      return 0;
    } else if (strcmp (argv[i], "-V") == 0 || strcmp (argv[i], "--version") == 0) {
      printf ("lipo (MMIX toolchain) version 1.0\n");
      printf ("Universal binary tool for Mach-O and FatELF\n");
      free (InputFiles);
      return 0;
    } else {
      //
      // Input file
      //
      InputFiles[NumInputFiles++] = argv[i];
    }
  }

  //
  // Perform action
  //
  if (gOptions.Action == LIPO_ACTION_NONE) {
    if (NumInputFiles == 1) {
      //
      // Default to -info if just one file
      //
      gOptions.Action = LIPO_ACTION_INFO;
    } else {
      fprintf (stderr, "lipo: no action specified\n");
      free (InputFiles);
      return 1;
    }
  }

  switch (gOptions.Action) {
    case LIPO_ACTION_INFO:
    case LIPO_ACTION_DETAILED_INFO:
      if (NumInputFiles != 1) {
        fprintf (stderr, "lipo: -info requires exactly one input file\n");
        free (InputFiles);
        return 1;
      }
      DisplayInfo (InputFiles[0], gOptions.Action == LIPO_ACTION_DETAILED_INFO);
      break;

    case LIPO_ACTION_CREATE:
      if (CreateUniversal (InputFiles, NumInputFiles, gOptions.Output) != 0) {
        free (InputFiles);
        return 1;
      }
      break;

    case LIPO_ACTION_THIN:
      printf ("Thin operation not yet implemented\n");
      break;

    default:
      fprintf (stderr, "lipo: action not yet implemented\n");
      free (InputFiles);
      return 1;
  }

  free (InputFiles);
  return 0;
}
