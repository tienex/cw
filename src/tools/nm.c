/** @file
  nm - List symbols from object files.

  Universal symbol lister that works across all binary formats:
  ELF, a.out, COFF/PE, Mach-O, OMF, and other formats.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include "binformat/BinFormat.h"
#include "binformat/LibAout.h"
#include "binformat/LibOmf.h"
#include "binformat/LibOrf.h"
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
  printf ("  G  Initialized data symbol (small)\n");
  printf ("  I  Indirect reference to another symbol\n");
  printf ("  N  Debugging symbol\n");
  printf ("  R  Read-only data symbol\n");
  printf ("  S  Uninitialized data symbol (small)\n");
  printf ("  T  Text (code) symbol\n");
  printf ("  U  Undefined symbol\n");
  printf ("  V  Weak object\n");
  printf ("  W  Weak symbol\n");
  printf ("  -  Stabs symbol\n");
  printf ("  ?  Unknown symbol type\n\n");
  printf ("Lowercase letters indicate local symbols.\n");
}

/**
  Get symbol type character.
**/
STATIC
CHAR8
GetSymbolType (
  IN BINFORMAT_SYMBOL  *Symbol
  )
{
  CHAR8  Type;

  if (Symbol->Flags & BINFORMAT_SYM_UNDEFINED) {
    Type = 'U';
  } else if (Symbol->Flags & BINFORMAT_SYM_ABSOLUTE) {
    Type = 'A';
  } else if (Symbol->Flags & BINFORMAT_SYM_BSS) {
    Type = 'B';
  } else if (Symbol->Flags & BINFORMAT_SYM_COMMON) {
    Type = 'C';
  } else if (Symbol->Flags & BINFORMAT_SYM_DATA) {
    Type = 'D';
  } else if (Symbol->Flags & BINFORMAT_SYM_TEXT) {
    Type = 'T';
  } else if (Symbol->Flags & BINFORMAT_SYM_RODATA) {
    Type = 'R';
  } else if (Symbol->Flags & BINFORMAT_SYM_DEBUG) {
    Type = 'N';
  } else if (Symbol->Flags & BINFORMAT_SYM_WEAK) {
    Type = (Symbol->Flags & BINFORMAT_SYM_UNDEFINED) ? 'w' : 'W';
  } else {
    Type = '?';
  }

  //
  // Lowercase for local symbols
  //
  if (!(Symbol->Flags & BINFORMAT_SYM_GLOBAL)) {
    if (Type >= 'A' && Type <= 'Z') {
      Type = Type + ('a' - 'A');
    }
  }

  return Type;
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

  Type = GetSymbolType (Symbol);

  if (gOptions.PrintFileName) {
    printf ("%s:", FileName);
  }

  if (Type == 'U' || Type == 'w') {
    printf ("                ");
  } else {
    printf ("%016llx", (unsigned long long)Symbol->Value);
  }

  printf (" %c %s", Type, Symbol->Name);

  if (gOptions.SizeSort && Symbol->Size > 0) {
    printf (" %llu", (unsigned long long)Symbol->Size);
  }

  printf ("\n");
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

  Type = GetSymbolType (Symbol);

  if (gOptions.PrintFileName) {
    printf ("%s: ", FileName);
  }

  printf ("%s %c", Symbol->Name, Type);

  if (Type != 'U' && Type != 'w') {
    printf (" %016llx", (unsigned long long)Symbol->Value);
    if (Symbol->Size > 0) {
      printf (" %016llx", (unsigned long long)Symbol->Size);
    }
  }

  printf ("\n");
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
  CHAR8  Type;

  Type = GetSymbolType (Symbol);

  printf ("%-20s|%016llx|   %c  |",
          Symbol->Name,
          (unsigned long long)Symbol->Value,
          Type);

  if (Symbol->Size > 0) {
    printf ("%16llu|", (unsigned long long)Symbol->Size);
  } else {
    printf ("                |");
  }

  printf ("\n");
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
  BINFORMAT_SYMBOL   Symbol;
  UINT32             Index;
  UINT32             Count;

  //
  // Try each format library
  //
  Status = ElfGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSymbols;
  }

  Status = AoutGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSymbols;
  }

  Status = CoffGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSymbols;
  }

  Status = MachoGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto ProcessSymbols;
  }

  fprintf (stderr, "nm: %s: File format not recognized\n", FileName);
  return BINFORMAT_ERROR_INVALID_FORMAT;

ProcessSymbols:
  //
  // Print file header for SysV format
  //
  if (strcmp (gOptions.Format, "sysv") == 0) {
    printf ("\n\nSymbols from %s:\n\n", FileName);
    printf ("Name                  Value           Class        Size            \n");
    printf ("================================================================================\n");
  } else if (gOptions.PrintFileName && strcmp (gOptions.Format, "bsd") == 0) {
    printf ("\n%s:\n", FileName);
  }

  //
  // Iterate through all symbols
  //
  Index = 0;
  Count = 0;
  while (Context.Api->GetSymbol (&Context, Index, &Symbol) == BINFORMAT_SUCCESS) {
    //
    // Apply filters
    //
    if (gOptions.ExternalOnly && !(Symbol.Flags & BINFORMAT_SYM_GLOBAL)) {
      Index++;
      continue;
    }

    if (gOptions.UndefinedOnly && !(Symbol.Flags & BINFORMAT_SYM_UNDEFINED)) {
      Index++;
      continue;
    }

    if (gOptions.DefinedOnly && (Symbol.Flags & BINFORMAT_SYM_UNDEFINED)) {
      Index++;
      continue;
    }

    if (!gOptions.DebugSymbols && (Symbol.Flags & BINFORMAT_SYM_DEBUG)) {
      Index++;
      continue;
    }

    //
    // Print symbol based on format
    //
    if (strcmp (gOptions.Format, "posix") == 0) {
      PrintSymbolPOSIX (FileName, &Symbol);
    } else if (strcmp (gOptions.Format, "sysv") == 0) {
      PrintSymbolSysV (FileName, &Symbol);
    } else {
      PrintSymbolBSD (FileName, &Symbol);
    }

    Count++;
    Index++;
  }

  if (Count == 0 && strcmp (gOptions.Format, "sysv") != 0) {
    printf ("nm: %s: no symbols\n", FileName);
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
  INT32   FileCount;
  INT32   i;
  BOOLEAN Success;

  STATIC struct option long_options[] = {
    {"debug-syms",      no_argument,       0, 'a'},
    {"extern-only",     no_argument,       0, 'g'},
    {"undefined-only",  no_argument,       0, 'u'},
    {"numeric-sort",    no_argument,       0, 'n'},
    {"no-sort",         no_argument,       0, 'p'},
    {"reverse-sort",    no_argument,       0, 'r'},
    {"print-size",      no_argument,       0, 'S'},
    {"size-sort",       no_argument,       0, 's'},
    {"print-file-name", no_argument,       0, 'A'},
    {"demangle",        no_argument,       0, 'C'},
    {"dynamic",         no_argument,       0, 'D'},
    {"defined-only",    no_argument,       0, 'd'},
    {"format",          required_argument, 0, 'f'},
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  FileCount = 0;
  Success   = TRUE;

  //
  // Parse options
  //
  while ((opt = getopt_long (argc, argv, "aguSnprACDdf:hV", long_options, NULL)) != -1) {
    switch (opt) {
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
      case 's':
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
      case 'd':
        gOptions.DefinedOnly = TRUE;
        break;
      case 'f':
        gOptions.Format = optarg;
        if (strcmp (optarg, "bsd") != 0 &&
            strcmp (optarg, "sysv") != 0 &&
            strcmp (optarg, "posix") != 0) {
          fprintf (stderr, "nm: invalid format: %s\n", optarg);
          return 1;
        }
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("nm (MMIX toolchain) version 1.0\n");
        printf ("Universal symbol lister for ELF, a.out, COFF, Mach-O, OMF\n");
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
    fprintf (stderr, "nm: no input files\n");
    return 1;
  }

  for (i = optind; i < argc; i++) {
    if (ProcessFile (argv[i]) != BINFORMAT_SUCCESS) {
      Success = FALSE;
    }
    FileCount++;
  }

  return Success ? 0 : 1;
}
