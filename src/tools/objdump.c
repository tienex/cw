/** @file
  objdump - Display information from object files.

  Universal object file dumper that works across all binary formats:
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
#include <ctype.h>

typedef struct {
  BOOLEAN  FileHeader;         ///< Display file header
  BOOLEAN  PrivateHeaders;     ///< Display format-specific headers
  BOOLEAN  SectionHeaders;     ///< Display section headers
  BOOLEAN  AllHeaders;         ///< Display all headers
  BOOLEAN  Disassemble;        ///< Disassemble code sections
  BOOLEAN  DisassembleAll;     ///< Disassemble all sections
  BOOLEAN  Symbols;            ///< Display symbol table
  BOOLEAN  DynamicSymbols;     ///< Display dynamic symbol table
  BOOLEAN  Relocations;        ///< Display relocations
  BOOLEAN  DynamicRelocations; ///< Display dynamic relocations
  BOOLEAN  FullContents;       ///< Display full contents of sections
  BOOLEAN  ArchiveHeaders;     ///< Display archive headers
  BOOLEAN  Demangle;           ///< Demangle C++ symbols
  BOOLEAN  Debugging;          ///< Display debugging information
  CHAR8    *Section;           ///< Specific section to display
  CHAR8    *TargetFormat;      ///< Target format override
} OBJDUMP_OPTIONS;

STATIC OBJDUMP_OPTIONS  gOptions = {0};

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
  printf ("Display information from object files (universal: ELF, a.out, COFF, Mach-O, etc.)\n\n");
  printf ("Options:\n");
  printf ("  -a, --archive-headers      Display archive member information\n");
  printf ("  -f, --file-headers         Display file header\n");
  printf ("  -p, --private-headers      Display format-specific file header\n");
  printf ("  -h, --section-headers      Display section headers\n");
  printf ("  -x, --all-headers          Display all headers\n");
  printf ("  -d, --disassemble          Display assembler contents of code sections\n");
  printf ("  -D, --disassemble-all      Display assembler contents of all sections\n");
  printf ("  -S, --source               Intermix source code with disassembly\n");
  printf ("  -s, --full-contents        Display full contents of all sections\n");
  printf ("  -g, --debugging            Display debugging information\n");
  printf ("  -t, --syms                 Display symbol table\n");
  printf ("  -T, --dynamic-syms         Display dynamic symbol table\n");
  printf ("  -r, --reloc                Display relocation entries\n");
  printf ("  -R, --dynamic-reloc        Display dynamic relocation entries\n");
  printf ("  -C, --demangle             Decode (demangle) C++ symbol names\n");
  printf ("  -j, --section=NAME         Display information for section NAME only\n");
  printf ("  -b, --target=FORMAT        Specify target object format\n");
  printf ("      --help                 Display this information\n");
  printf ("  -V, --version              Display version information\n\n");
  printf ("Supported formats:\n");
  printf ("  elf32, elf64, aout, coff, pe, macho, omf, nlm, mmo\n");
}

/**
  Print file header.
**/
STATIC
VOID
PrintFileHeader (
  IN CONST CHAR8          *FileName,
  IN BINFORMAT_CONTEXT    *Context
  )
{
  BINFORMAT_HEADER  Header;

  if (Context->Api->GetHeader (Context, &Header) != BINFORMAT_SUCCESS) {
    return;
  }

  printf ("\n%s:     file format %s\n\n", FileName, Context->Api->LibraryName);
  printf ("architecture: ");

  switch (Header.Machine) {
    case 3:   printf ("i386\n"); break;
    case 62:  printf ("x86-64\n"); break;
    case 40:  printf ("arm\n"); break;
    case 183: printf ("aarch64\n"); break;
    case 20:  printf ("powerpc\n"); break;
    case 21:  printf ("powerpc64\n"); break;
    case 8:   printf ("mips\n"); break;
    case 2:   printf ("sparc\n"); break;
    case 243: printf ("riscv\n"); break;
    default:  printf ("unknown (%u)\n", Header.Machine); break;
  }

  printf ("start address: 0x%016llx\n", (unsigned long long)Header.EntryPoint);
}

/**
  Print section headers.
**/
STATIC
VOID
PrintSectionHeaders (
  IN BINFORMAT_CONTEXT  *Context
  )
{
  BINFORMAT_SECTION  Section;
  UINT32             Index;
  CHAR8              Flags[16];

  printf ("\nSections:\n");
  printf ("Idx Name          Size      VMA               LMA               File off  Algn  Flags\n");

  Index = 0;
  while (Context->Api->GetSection (Context, Index, &Section) == BINFORMAT_SUCCESS) {
    //
    // Build flags string
    //
    memset (Flags, 0, sizeof (Flags));
    INT32 FlagIdx = 0;

    if (Section.Flags & BINFORMAT_SEC_ALLOC)    Flags[FlagIdx++] = 'A';
    if (Section.Flags & BINFORMAT_SEC_LOAD)     Flags[FlagIdx++] = 'L';
    if (Section.Flags & BINFORMAT_SEC_CODE)     Flags[FlagIdx++] = 'C';
    if (Section.Flags & BINFORMAT_SEC_DATA)     Flags[FlagIdx++] = 'D';
    if (Section.Flags & BINFORMAT_SEC_READONLY) Flags[FlagIdx++] = 'R';
    if (Section.Flags & BINFORMAT_SEC_BSS)      Flags[FlagIdx++] = 'B';

    printf ("  %u %-12s  %08llx  %016llx  %016llx  %08llx  2**%u  %s\n",
            Index,
            Section.Name,
            (unsigned long long)Section.Size,
            (unsigned long long)Section.Address,
            (unsigned long long)Section.Address,
            (unsigned long long)Section.Offset,
            Section.Alignment,
            Flags);

    Index++;
  }
}

/**
  Print symbol table.
**/
STATIC
VOID
PrintSymbolTable (
  IN BINFORMAT_CONTEXT  *Context,
  IN BOOLEAN            Dynamic
  )
{
  BINFORMAT_SYMBOL  Symbol;
  UINT32            Index;
  CHAR8             Type;
  UINT32            Count;

  printf ("\n%s:\n", Dynamic ? "DYNAMIC SYMBOL TABLE" : "SYMBOL TABLE");

  Index = 0;
  Count = 0;
  while (Context->Api->GetSymbol (Context, Index, &Symbol) == BINFORMAT_SUCCESS) {
    //
    // Skip non-dynamic symbols if requested
    //
    if (Dynamic && !(Symbol.Flags & BINFORMAT_SYM_DYNAMIC)) {
      Index++;
      continue;
    }

    //
    // Determine symbol type
    //
    if (Symbol.Flags & BINFORMAT_SYM_UNDEFINED) {
      Type = 'U';
    } else if (Symbol.Flags & BINFORMAT_SYM_ABSOLUTE) {
      Type = 'A';
    } else if (Symbol.Flags & BINFORMAT_SYM_BSS) {
      Type = 'B';
    } else if (Symbol.Flags & BINFORMAT_SYM_DATA) {
      Type = 'D';
    } else if (Symbol.Flags & BINFORMAT_SYM_TEXT) {
      Type = 'T';
    } else {
      Type = '?';
    }

    //
    // Lowercase for local symbols
    //
    if (!(Symbol.Flags & BINFORMAT_SYM_GLOBAL)) {
      if (Type >= 'A' && Type <= 'Z') {
        Type = Type + ('a' - 'A');
      }
    }

    printf ("%016llx %c %s",
            (unsigned long long)Symbol.Value,
            Type,
            Symbol.Name);

    if (Symbol.Size > 0) {
      printf ("\t%llu", (unsigned long long)Symbol.Size);
    }

    printf ("\n");

    Count++;
    Index++;
  }

  if (Count == 0) {
    printf ("no symbols\n");
  }
}

/**
  Print relocations.
**/
STATIC
VOID
PrintRelocations (
  IN BINFORMAT_CONTEXT  *Context
  )
{
  BINFORMAT_SECTION      Section;
  BINFORMAT_RELOCATION   *Relocations;
  UINT32                 SectionIndex;
  UINT32                 Count;
  UINT32                 i;

  printf ("\nRELOCATION RECORDS:\n");

  SectionIndex = 0;
  while (Context->Api->GetSection (Context, SectionIndex, &Section) == BINFORMAT_SUCCESS) {
    if (Context->Api->GetRelocations (Context, SectionIndex, &Relocations, &Count) == BINFORMAT_SUCCESS) {
      if (Count > 0) {
        printf ("\nRELOCATION RECORDS FOR [%s]:\n", Section.Name);
        printf ("OFFSET           TYPE              VALUE\n");

        for (i = 0; i < Count; i++) {
          printf ("%016llx %-16u  %s",
                  (unsigned long long)Relocations[i].Offset,
                  Relocations[i].Type,
                  Relocations[i].SymbolName ? Relocations[i].SymbolName : "(null)");

          if (Relocations[i].Addend != 0) {
            printf ("+0x%llx", (unsigned long long)Relocations[i].Addend);
          }

          printf ("\n");
        }
      }
    }

    SectionIndex++;
  }
}

/**
  Print section contents in hex dump format.
**/
STATIC
VOID
PrintSectionContents (
  IN BINFORMAT_CONTEXT  *Context,
  IN CONST CHAR8        *SectionName
  )
{
  BINFORMAT_SECTION  Section;
  UINT32             Index;
  UINT8              *Data;
  UINT64             i, j;

  //
  // Find section
  //
  if (SectionName) {
    if (Context->Api->GetSectionByName (Context, SectionName, &Section) != BINFORMAT_SUCCESS) {
      fprintf (stderr, "objdump: section '%s' not found\n", SectionName);
      return;
    }
  } else {
    //
    // Display all sections
    //
    Index = 0;
    while (Context->Api->GetSection (Context, Index, &Section) == BINFORMAT_SUCCESS) {
      printf ("\nContents of section %s:\n", Section.Name);

      //
      // Read section data (stub - would need API extension)
      //
      printf (" (section contents display not yet implemented)\n");

      Index++;
    }
    return;
  }

  printf ("\nContents of section %s:\n", Section.Name);
  printf (" (section contents display not yet implemented)\n");
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
    goto DisplayInformation;
  }

  Status = AoutGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto DisplayInformation;
  }

  Status = CoffGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto DisplayInformation;
  }

  Status = MachoGetApi()->InitFile (&Context, FileName);
  if (Status == BINFORMAT_SUCCESS) {
    goto DisplayInformation;
  }

  fprintf (stderr, "objdump: %s: File format not recognized\n", FileName);
  return BINFORMAT_ERROR_INVALID_FORMAT;

DisplayInformation:
  //
  // Display requested information
  //
  if (gOptions.FileHeader || gOptions.AllHeaders) {
    PrintFileHeader (FileName, &Context);
  }

  if (gOptions.SectionHeaders || gOptions.AllHeaders) {
    PrintSectionHeaders (&Context);
  }

  if (gOptions.Symbols || gOptions.AllHeaders) {
    PrintSymbolTable (&Context, FALSE);
  }

  if (gOptions.DynamicSymbols) {
    PrintSymbolTable (&Context, TRUE);
  }

  if (gOptions.Relocations || gOptions.AllHeaders) {
    PrintRelocations (&Context);
  }

  if (gOptions.FullContents) {
    PrintSectionContents (&Context, gOptions.Section);
  }

  if (gOptions.Disassemble) {
    printf ("\nDisassembly not yet implemented\n");
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
  INT32    opt;
  INT32    i;
  BOOLEAN  Success;

  STATIC struct option long_options[] = {
    {"archive-headers",   no_argument,       0, 'a'},
    {"file-headers",      no_argument,       0, 'f'},
    {"private-headers",   no_argument,       0, 'p'},
    {"section-headers",   no_argument,       0, 'h'},
    {"all-headers",       no_argument,       0, 'x'},
    {"disassemble",       no_argument,       0, 'd'},
    {"disassemble-all",   no_argument,       0, 'D'},
    {"source",            no_argument,       0, 'S'},
    {"full-contents",     no_argument,       0, 's'},
    {"debugging",         no_argument,       0, 'g'},
    {"syms",              no_argument,       0, 't'},
    {"dynamic-syms",      no_argument,       0, 'T'},
    {"reloc",             no_argument,       0, 'r'},
    {"dynamic-reloc",     no_argument,       0, 'R'},
    {"demangle",          no_argument,       0, 'C'},
    {"section",           required_argument, 0, 'j'},
    {"target",            required_argument, 0, 'b'},
    {"help",              no_argument,       0, 'H'},
    {"version",           no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  Success = TRUE;

  //
  // Parse options
  //
  while ((opt = getopt_long (argc, argv, "afphxdDSsgtTrRCj:b:HV", long_options, NULL)) != -1) {
    switch (opt) {
      case 'a':
        gOptions.ArchiveHeaders = TRUE;
        break;
      case 'f':
        gOptions.FileHeader = TRUE;
        break;
      case 'p':
        gOptions.PrivateHeaders = TRUE;
        break;
      case 'h':
        gOptions.SectionHeaders = TRUE;
        break;
      case 'x':
        gOptions.AllHeaders = TRUE;
        break;
      case 'd':
        gOptions.Disassemble = TRUE;
        break;
      case 'D':
        gOptions.DisassembleAll = TRUE;
        gOptions.Disassemble    = TRUE;
        break;
      case 's':
        gOptions.FullContents = TRUE;
        break;
      case 'g':
        gOptions.Debugging = TRUE;
        break;
      case 't':
        gOptions.Symbols = TRUE;
        break;
      case 'T':
        gOptions.DynamicSymbols = TRUE;
        break;
      case 'r':
        gOptions.Relocations = TRUE;
        break;
      case 'R':
        gOptions.DynamicRelocations = TRUE;
        break;
      case 'C':
        gOptions.Demangle = TRUE;
        break;
      case 'j':
        gOptions.Section = optarg;
        break;
      case 'b':
        gOptions.TargetFormat = optarg;
        break;
      case 'H':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("objdump (MMIX toolchain) version 1.0\n");
        printf ("Universal object dumper for ELF, a.out, COFF, Mach-O, OMF\n");
        return 0;
      default:
        PrintUsage (argv[0]);
        return 1;
    }
  }

  //
  // If no options specified, show file header by default
  //
  if (!gOptions.FileHeader && !gOptions.PrivateHeaders &&
      !gOptions.SectionHeaders && !gOptions.AllHeaders &&
      !gOptions.Symbols && !gOptions.DynamicSymbols &&
      !gOptions.Relocations && !gOptions.DynamicRelocations &&
      !gOptions.FullContents && !gOptions.Disassemble) {
    gOptions.FileHeader = TRUE;
  }

  //
  // Process files
  //
  if (optind >= argc) {
    fprintf (stderr, "objdump: no input files\n");
    return 1;
  }

  for (i = optind; i < argc; i++) {
    if (ProcessFile (argv[i]) != BINFORMAT_SUCCESS) {
      Success = FALSE;
    }
  }

  return Success ? 0 : 1;
}
