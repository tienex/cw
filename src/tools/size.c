/** @file
  size - List section sizes of object files.

  Universal section size lister that works across all binary formats:
  ELF, a.out, COFF/PE, Mach-O, OMF, and other formats.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include "binformat/BinFormat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

typedef struct {
  BOOLEAN  Berkeley;     ///< Use Berkeley format (default)
  BOOLEAN  SysV;         ///< Use System V format
  BOOLEAN  Octal;        ///< Print sizes in octal
  BOOLEAN  Decimal;      ///< Print sizes in decimal
  BOOLEAN  Hex;          ///< Print sizes in hexadecimal
  BOOLEAN  Radix;        ///< Print radix with numbers
  INT32    Format;       ///< Format type: 10=decimal, 8=octal, 16=hex
} SIZE_OPTIONS;

STATIC SIZE_OPTIONS  gOptions = {
  .Berkeley = TRUE,
  .SysV     = FALSE,
  .Octal    = FALSE,
  .Decimal  = FALSE,
  .Hex      = FALSE,
  .Radix    = FALSE,
  .Format   = 10
};

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
  printf ("List section sizes of object files (universal: ELF, a.out, COFF, Mach-O, etc.)\n\n");
  printf ("Options:\n");
  printf ("  -A, --format=sysv      Use System V size output format\n");
  printf ("  -B, --format=berkeley  Use Berkeley size output format (default)\n");
  printf ("  -o, --radix=8          Print sizes in octal\n");
  printf ("  -d, --radix=10         Print sizes in decimal (default)\n");
  printf ("  -x, --radix=16         Print sizes in hexadecimal\n");
  printf ("      --radix=NUMBER     Set radix for sizes\n");
  printf ("  -t, --totals           Print totals (Berkeley format only)\n");
  printf ("  -h, --help             Display this information\n");
  printf ("  -V, --version          Display version information\n\n");
  printf ("Size output:\n");
  printf ("  Berkeley format: text + data + bss = total (decimal) filename\n");
  printf ("  System V format: detailed section listing with sizes\n");
}

/**
  Print number in specified radix.
**/
STATIC
VOID
PrintSize (
  IN UINT64  Size
  )
{
  if (gOptions.Format == 8) {
    printf ("%llo", (unsigned long long)Size);
    if (gOptions.Radix) {
      printf ("(8)");
    }
  } else if (gOptions.Format == 16) {
    printf ("0x%llx", (unsigned long long)Size);
    if (gOptions.Radix) {
      printf ("(16)");
    }
  } else {
    printf ("%llu", (unsigned long long)Size);
    if (gOptions.Radix) {
      printf ("(10)");
    }
  }
}

/**
  Process file in Berkeley format.
**/
STATIC
VOID
ProcessFileBerkeley (
  IN CONST CHAR8          *FileName,
  IN BINFORMAT_CONTEXT    *Context
  )
{
  BINFORMAT_SECTION  Section;
  UINT32             Index;
  UINT64             TextSize;
  UINT64             DataSize;
  UINT64             BssSize;
  UINT64             Total;

  TextSize = 0;
  DataSize = 0;
  BssSize  = 0;

  //
  // Sum up section sizes
  //
  Index = 0;
  while (Context->Api->GetSection (Context, Index, &Section) == BINFORMAT_SUCCESS) {
    if (Section.Flags & BINFORMAT_SEC_CODE) {
      TextSize += Section.Size;
    } else if (Section.Flags & BINFORMAT_SEC_DATA) {
      if (Section.Flags & BINFORMAT_SEC_BSS) {
        BssSize += Section.Size;
      } else {
        DataSize += Section.Size;
      }
    } else if (Section.Flags & BINFORMAT_SEC_BSS) {
      BssSize += Section.Size;
    }
    Index++;
  }

  Total = TextSize + DataSize + BssSize;

  //
  // Print Berkeley format
  //
  printf ("   ");
  PrintSize (TextSize);
  printf ("\t   ");
  PrintSize (DataSize);
  printf ("\t   ");
  PrintSize (BssSize);
  printf ("\t   ");
  PrintSize (Total);
  printf ("\t   ");
  if (gOptions.Format == 16) {
    printf ("0x%llx", (unsigned long long)Total);
  } else if (gOptions.Format == 8) {
    printf ("%llo", (unsigned long long)Total);
  } else {
    printf ("%llu", (unsigned long long)Total);
  }
  printf ("\t%s\n", FileName);
}

/**
  Process file in System V format.
**/
STATIC
VOID
ProcessFileSysV (
  IN CONST CHAR8          *FileName,
  IN BINFORMAT_CONTEXT    *Context
  )
{
  BINFORMAT_SECTION  Section;
  UINT32             Index;
  UINT64             Total;

  printf ("%s  :\n", FileName);
  printf ("section              size      addr\n");

  Total = 0;
  Index = 0;
  while (Context->Api->GetSection (Context, Index, &Section) == BINFORMAT_SUCCESS) {
    printf ("%-20s ", Section.Name);
    PrintSize (Section.Size);
    printf ("      ");
    PrintSize (Section.Address);
    printf ("\n");

    if (!(Section.Flags & BINFORMAT_SEC_BSS)) {
      Total += Section.Size;
    }

    Index++;
  }

  printf ("%-20s ", "Total");
  PrintSize (Total);
  printf ("\n\n");
}

/**
  Process a single object file.
**/
STATIC
BINFORMAT_STATUS
ProcessFile (
  IN CONST CHAR8  *FileName
  )
{
  BINFORMAT_CONTEXT  Context;
  BINFORMAT_STATUS   Status;

  //
  // Try each format library
  //
  Status = ElfGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSections;
  }

  Status = AoutGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSections;
  }

  Status = CoffGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSections;
  }

  Status = MachoGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSections;
  }

  fprintf (stderr, "size: %s: File format not recognized\n", FileName);
  return BINFORMAT_ERROR_INVALID_FORMAT;

ProcessSections:
  if (gOptions.SysV) {
    ProcessFileSysV (FileName, &Context);
  } else {
    ProcessFileBerkeley (FileName, &Context);
  }

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
  INT32   opt;
  INT32   i;
  BOOLEAN Success;
  BOOLEAN PrintedHeader;

  STATIC struct option long_options[] = {
    {"format",   required_argument, 0, 'A'},
    {"radix",    required_argument, 0, 'r'},
    {"totals",   no_argument,       0, 't'},
    {"help",     no_argument,       0, 'h'},
    {"version",  no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  Success       = TRUE;
  PrintedHeader = FALSE;

  //
  // Parse options
  //
  while ((opt = getopt_long (argc, argv, "ABodxr:thV", long_options, NULL)) != -1) {
    switch (opt) {
      case 'A':
        gOptions.SysV     = TRUE;
        gOptions.Berkeley = FALSE;
        if (optarg && strcmp (optarg, "berkeley") == 0) {
          gOptions.Berkeley = TRUE;
          gOptions.SysV     = FALSE;
        }
        break;
      case 'B':
        gOptions.Berkeley = TRUE;
        gOptions.SysV     = FALSE;
        break;
      case 'o':
      case 'r':
        if (optarg) {
          gOptions.Format = atoi (optarg);
        } else {
          gOptions.Format = 8;
        }
        if (gOptions.Format != 8 && gOptions.Format != 10 && gOptions.Format != 16) {
          fprintf (stderr, "size: invalid radix: %d\n", gOptions.Format);
          return 1;
        }
        break;
      case 'd':
        gOptions.Format = 10;
        break;
      case 'x':
        gOptions.Format = 16;
        break;
      case 't':
        // Totals - currently ignored
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("size (MMIX toolchain) version 1.0\n");
        printf ("Universal size lister for ELF, a.out, COFF, Mach-O, OMF\n");
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
    fprintf (stderr, "size: no input files\n");
    return 1;
  }

  //
  // Print header for Berkeley format
  //
  if (gOptions.Berkeley && !PrintedHeader) {
    printf ("   text\t   data\t    bss\t    dec\t    hex\tfilename\n");
    PrintedHeader = TRUE;
  }

  for (i = optind; i < argc; i++) {
    if (ProcessFile (argv[i]) != BINFORMAT_SUCCESS) {
      Success = FALSE;
    }
  }

  return Success ? 0 : 1;
}
