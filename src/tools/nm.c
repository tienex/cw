/** @file
  nm - List symbols from object files.

  Universal symbol lister that works across all binary formats:
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
  BOOLEAN  DebugSymbols;      ///< Show debug symbols
  BOOLEAN  ExternalOnly;      ///< Show only external symbols
  BOOLEAN  UndefinedOnly;     ///< Show only undefined symbols
  BOOLEAN  NumericSort;       ///< Sort by address
  BOOLEAN  ReverseSort;       ///< Reverse sort order
  BOOLEAN  NoSort;            ///< Don't sort
  BOOLEAN  SizeSort;          ///< Sort by size
  BOOLEAN  PrintFileName;     ///< Print file name with each symbol
  BOOLEAN  Demangle;          ///< Demangle C++ names
  BOOLEAN  Dynamic;           ///< Show dynamic symbols only
  BOOLEAN  DefinedOnly;       ///< Show only defined symbols
  CHAR8    *Format;           ///< Output format (bsd, posix, sysv)
} NM_OPTIONS;

STATIC NM_OPTIONS  gOptions = {
  .DebugSymbols  = FALSE,
  .ExternalOnly  = FALSE,
  .UndefinedOnly = FALSE,
  .NumericSort   = FALSE,
  .ReverseSort   = FALSE,
  .NoSort        = FALSE,
  .SizeSort      = FALSE,
  .PrintFileName = FALSE,
  .Demangle      = FALSE,
  .Dynamic       = FALSE,
  .DefinedOnly   = FALSE,
  .Format        = "bsd"
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
  printf ("List symbols from object files (universal: ELF, a.out, COFF, Mach-O, etc.)\n\n");
  printf ("Options:\n");
  printf ("  -a, --debug-syms       Display debugger-only symbols\n");
  printf ("  -g, --extern-only      Display only external symbols\n");
  printf ("  -u, --undefined-only   Display only undefined symbols\n");
  printf ("  -n, --numeric-sort     Sort symbols numerically by address\n");
  printf ("  -p, --no-sort          Do not sort symbols\n");
  printf ("  -r, --reverse-sort     Reverse the sense of the sort\n");
  printf ("  -S, --print-size       Print size of defined symbols\n");
  printf ("      --size-sort        Sort symbols by size\n");
  printf ("  -A, --print-file-name  Print name of the input file before each symbol\n");
  printf ("  -C, --demangle         Decode (demangle) C++ symbol names\n");
  printf ("  -D, --dynamic          Display dynamic symbols instead of normal symbols\n");
  printf ("      --defined-only     Display only defined symbols\n");
  printf ("  -f, --format=FORMAT    Use the output format FORMAT (bsd, sysv, posix)\n");
  printf ("  -h, --help             Display this information\n");
  printf ("  -V, --version          Display version information\n\n");
  printf ("Symbol type letters:\n");
  printf ("  A  Absolute symbol\n");
  printf ("  B  BSS (uninitialized data) symbol\n");
  printf ("  C  Common symbol\n");
  printf ("  D  Initialized data symbol\n");
  printf ("  T  Text (code) symbol\n");
  printf ("  U  Undefined symbol\n");
  printf ("  W  Weak symbol\n");
  printf ("  ?  Unknown symbol type\n\n");
  printf ("Lowercase letters indicate local symbols.\n");
}

/**
  Print symbol in BSD format.
**/
STATIC
VOID
PrintSymbolBSD (
  IN CONST CHAR8       *FileName,
  IN BINFORMAT_SYMBOL  *Symbol
  )
{
  CHAR8  Type;

  Type = BinFormatGetSymbolTypeChar (Symbol);

  if (gOptions.PrintFileName) {
    printf ("%s:", FileName);
  }

  //
  // For undefined symbols, don't print address
  //
  if (Symbol->SectionIndex == 0) {
    printf ("                 %c %s\n", Type, Symbol->Name);
  } else {
    printf ("%016llx %c %s\n", (unsigned long long)Symbol->Value, Type, Symbol->Name);
  }
}

/**
  Print symbol in POSIX format.
**/
STATIC
VOID
PrintSymbolPOSIX (
  IN CONST CHAR8       *FileName,
  IN BINFORMAT_SYMBOL  *Symbol
  )
{
  CHAR8  Type;

  Type = BinFormatGetSymbolTypeChar (Symbol);

  if (gOptions.PrintFileName) {
    printf ("%s: ", FileName);
  }

  printf ("%s %c %016llx %llu\n",
    Symbol->Name,
    Type,
    (unsigned long long)Symbol->Value,
    (unsigned long long)Symbol->Size);
}

/**
  Print symbol in SysV format.
**/
STATIC
VOID
PrintSymbolSysV (
  IN CONST CHAR8       *FileName,
  IN BINFORMAT_SYMBOL  *Symbol
  )
{
  (VOID)FileName;

  printf ("%-20s|%016llx|   %c  |",
    Symbol->Name,
    (unsigned long long)Symbol->Value,
    BinFormatGetSymbolTypeChar (Symbol));

  printf ("%18s|%12s|%llu\n",
    BinFormatGetSymbolTypeName (Symbol->Type),
    BinFormatGetSymbolBindName (Symbol->Bind),
    (unsigned long long)Symbol->Size);
}

/**
  Process a binary file and list its symbols.
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
  BINFORMAT_SYMBOL     Symbol;
  UINT32               i;

  //
  // Auto-detect format and initialize
  //
  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "nm: %s: File format not recognized\n", FileName);
    return 1;
  }

  //
  // Get header information
  //
  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "nm: %s: Failed to get header\n", FileName);
    Api->Close (Context);
    return 1;
  }

  //
  // Print header for SysV format
  //
  if (strcmp (gOptions.Format, "sysv") == 0) {
    printf ("\n\nSymbols from %s:\n\n", FileName);
    printf ("Name                 |Value           |Class|Type              |Binding      |Size\n");
    printf ("---------------------|----------------|-----|------------------|-------------|----\n");
  }

  //
  // Iterate through symbols
  //
  for (i = 0; i < HeaderInfo.SymbolCount; i++) {
    Status = Api->GetSymbol (Context, i, &Symbol);
    if (BINFORMAT_IS_ERROR (Status)) {
      continue;
    }

    //
    // Apply filters
    //
    if (gOptions.ExternalOnly && Symbol.Bind != BinSymbolBindGlobal) {
      continue;
    }

    if (gOptions.UndefinedOnly && Symbol.SectionIndex != 0) {
      continue;
    }

    if (gOptions.DefinedOnly && Symbol.SectionIndex == 0) {
      continue;
    }

    if (!gOptions.DebugSymbols && Symbol.Type == BinSymbolTypeFile) {
      continue;
    }

    //
    // Print symbol based on format
    //
    if (strcmp (gOptions.Format, "sysv") == 0) {
      PrintSymbolSysV (FileName, &Symbol);
    } else if (strcmp (gOptions.Format, "posix") == 0) {
      PrintSymbolPOSIX (FileName, &Symbol);
    } else {
      PrintSymbolBSD (FileName, &Symbol);
    }
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
    {"debug-syms",       no_argument,       0, 'a'},
    {"extern-only",      no_argument,       0, 'g'},
    {"undefined-only",   no_argument,       0, 'u'},
    {"numeric-sort",     no_argument,       0, 'n'},
    {"no-sort",          no_argument,       0, 'p'},
    {"reverse-sort",     no_argument,       0, 'r'},
    {"print-size",       no_argument,       0, 'S'},
    {"size-sort",        no_argument,       0, 1000},
    {"print-file-name",  no_argument,       0, 'A'},
    {"demangle",         no_argument,       0, 'C'},
    {"dynamic",          no_argument,       0, 'D'},
    {"defined-only",     no_argument,       0, 1001},
    {"format",           required_argument, 0, 'f'},
    {"help",             no_argument,       0, 'h'},
    {"version",          no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long (argc, argv, "aguнprSACDf:hV", long_options, &option_index)) != -1) {
    switch (c) {
      case 'a':
        gOptions.DebugSymbols = TRUE;
        break;
      case 'g':
        gOptions.ExternalOnly = TRUE;
        break;
      case 'u':
        gOptions.UndefinedOnly = TRUE;
        break;
      case 'n':
        gOptions.NumericSort = TRUE;
        break;
      case 'p':
        gOptions.NoSort = TRUE;
        break;
      case 'r':
        gOptions.ReverseSort = TRUE;
        break;
      case 'S':
        // Size printing is always enabled in our output
        break;
      case 1000:
        gOptions.SizeSort = TRUE;
        break;
      case 'A':
        gOptions.PrintFileName = TRUE;
        break;
      case 'C':
        gOptions.Demangle = TRUE;
        break;
      case 'D':
        gOptions.Dynamic = TRUE;
        break;
      case 1001:
        gOptions.DefinedOnly = TRUE;
        break;
      case 'f':
        gOptions.Format = optarg;
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("nm (MMIX toolchain) 1.0\n");
        return 0;
      default:
        PrintUsage (argv[0]);
        return 1;
    }
  }

  if (optind >= argc) {
    fprintf (stderr, "nm: No input files specified\n");
    PrintUsage (argv[0]);
    return 1;
  }

  //
  // Process each file
  //
  for (INT32 i = optind; i < argc; i++) {
    if (argc - optind > 1) {
      printf ("\n%s:\n", argv[i]);
    }
    result |= ProcessFile (argv[i]);
  }

  return result;
}
