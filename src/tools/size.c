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
  IN CONST CHAR8               *FileName,
  IN CONST BINFORMAT_API       *Api,
  IN BINFORMAT_CONTEXT         *Context,
  IN BINFORMAT_HEADER_INFO     *HeaderInfo
  )
{
  BINFORMAT_SECTION  Section;
  BINFORMAT_STATUS   Status;
  UINT64             TextSize = 0;
  UINT64             DataSize = 0;
  UINT64             BssSize  = 0;
  UINT64             TotalSize;
  UINT32             i;

  //
  // Iterate through sections and categorize them
  //
  for (i = 0; i < HeaderInfo->SectionCount; i++) {
    Status = Api->GetSection (Context, i, &Section);
    if (BINFORMAT_IS_ERROR (Status)) {
      continue;
    }

    //
    // Skip sections that don't occupy memory
    //
    if (!(Section.Flags & BINFORMAT_SECTION_FLAG_ALLOC)) {
      continue;
    }

    //
    // Categorize based on section flags
    //
    if (Section.Flags & BINFORMAT_SECTION_FLAG_EXEC) {
      TextSize += Section.Size;
    } else if (Section.Type == BinSectionTypeNoBits) {
      BssSize += Section.Size;
    } else if (Section.Flags & BINFORMAT_SECTION_FLAG_WRITE) {
      DataSize += Section.Size;
    } else {
      //
      // Read-only data counts as text
      //
      TextSize += Section.Size;
    }
  }

  TotalSize = TextSize + DataSize + BssSize;

  //
  // Print in Berkeley format
  //
  printf ("   ");
  PrintSize (TextSize);
  printf ("\t   ");
  PrintSize (DataSize);
  printf ("\t    ");
  PrintSize (BssSize);
  printf ("\t    ");
  PrintSize (TotalSize);
  printf ("\t%llu\t%s\n", (unsigned long long)TotalSize, FileName);
}

/**
  Process file in System V format.
**/
STATIC
VOID
ProcessFileSysV (
  IN CONST CHAR8               *FileName,
  IN CONST BINFORMAT_API       *Api,
  IN BINFORMAT_CONTEXT         *Context,
  IN BINFORMAT_HEADER_INFO     *HeaderInfo
  )
{
  BINFORMAT_SECTION  Section;
  BINFORMAT_STATUS   Status;
  UINT64             TotalSize = 0;
  UINT32             i;

  printf ("%s  :\n", FileName);
  printf ("section              size      addr\n");

  //
  // Iterate through all sections
  //
  for (i = 0; i < HeaderInfo->SectionCount; i++) {
    Status = Api->GetSection (Context, i, &Section);
    if (BINFORMAT_IS_ERROR (Status)) {
      continue;
    }

    //
    // Only print sections that occupy space
    //
    if (Section.Size == 0) {
      continue;
    }

    printf ("%-20s ", Section.Name);
    PrintSize (Section.Size);
    printf ("      ");
    PrintSize (Section.VirtualAddress);
    printf ("\n");

    if (Section.Flags & BINFORMAT_SECTION_FLAG_ALLOC) {
      TotalSize += Section.Size;
    }
  }

  printf ("Total                ");
  PrintSize (TotalSize);
  printf ("\n\n");
}

/**
  Process a binary file and display its sizes.
**/
STATIC
INT32
ProcessFile (
  IN CONST CHAR8  *FileName
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_CONTEXT    *Context;
  BINFORMAT_STATUS     Status;
  BINFORMAT_HEADER_INFO HeaderInfo;

  //
  // Auto-detect format and initialize
  //
  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "size: %s: File format not recognized\n", FileName);
    return 1;
  }

  //
  // Get header information
  //
  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "size: %s: Failed to get header\n", FileName);
    Api->Close (Context);
    return 1;
  }

  //
  // Process based on format
  //
  if (gOptions.SysV) {
    ProcessFileSysV (FileName, Api, Context, &HeaderInfo);
  } else {
    ProcessFileBerkeley (FileName, Api, Context, &HeaderInfo);
  }

  Api->Close (Context);
  return 0;
}

/**
  Main entry point.
**/
INT32
main (
  INT32   argc,
  CHAR8   **argv
  )
{
  INT32  c;
  INT32  option_index;
  INT32  result = 0;

  static struct option long_options[] = {
    {"format",  required_argument, 0, 'F'},
    {"radix",   required_argument, 0, 'R'},
    {"totals",  no_argument,       0, 't'},
    {"help",    no_argument,       0, 'h'},
    {"version", no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long (argc, argv, "ABodxthV", long_options, &option_index)) != -1) {
    switch (c) {
      case 'A':
        gOptions.SysV = TRUE;
        gOptions.Berkeley = FALSE;
        break;
      case 'B':
        gOptions.Berkeley = TRUE;
        gOptions.SysV = FALSE;
        break;
      case 'F':
        if (strcmp (optarg, "sysv") == 0 || strcmp (optarg, "sysv") == 0) {
          gOptions.SysV = TRUE;
          gOptions.Berkeley = FALSE;
        } else if (strcmp (optarg, "berkeley") == 0 || strcmp (optarg, "bsd") == 0) {
          gOptions.Berkeley = TRUE;
          gOptions.SysV = FALSE;
        }
        break;
      case 'o':
      case 'R':
        if (c == 'o' || (optarg && strcmp (optarg, "8") == 0)) {
          gOptions.Format = 8;
          gOptions.Octal = TRUE;
        } else if (optarg && strcmp (optarg, "10") == 0) {
          gOptions.Format = 10;
          gOptions.Decimal = TRUE;
        } else if (optarg && strcmp (optarg, "16") == 0) {
          gOptions.Format = 16;
          gOptions.Hex = TRUE;
        }
        break;
      case 'd':
        gOptions.Format = 10;
        gOptions.Decimal = TRUE;
        break;
      case 'x':
        gOptions.Format = 16;
        gOptions.Hex = TRUE;
        break;
      case 't':
        // Totals are always printed in Berkeley format
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("size (MMIX toolchain) 1.0\n");
        return 0;
      default:
        PrintUsage (argv[0]);
        return 1;
    }
  }

  if (optind >= argc) {
    fprintf (stderr, "size: No input files specified\n");
    PrintUsage (argv[0]);
    return 1;
  }

  //
  // Print Berkeley header
  //
  if (gOptions.Berkeley) {
    printf ("   text\t   data\t    bss\t    dec\t    hex\tfilename\n");
  }

  //
  // Process each file
  //
  for (INT32 i = optind; i < argc; i++) {
    result |= ProcessFile (argv[i]);
  }

  return result;
}
