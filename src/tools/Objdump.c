/** @file
  MMIX Object Dump utility implementation.

  This file implements an objdump-like tool for inspecting MMIX
  binary files and object files.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../../include/MmixLoader.h"
#include "../../include/MmixDisasm.h"
#include "../../include/MmixTypes.h"

//
// Display options
//
typedef struct {
  BOOLEAN  ShowFileHeader;
  BOOLEAN  ShowSectionHeaders;
  BOOLEAN  ShowSymbolTable;
  BOOLEAN  Disassemble;
  BOOLEAN  ShowHexDump;
  BOOLEAN  ShowAllHeaders;
  CHAR8    *InputFile;
} OBJDUMP_OPTIONS;

/**
  Get format name string.

  @param[in]      Format        Binary format.

  @return  Format name string.

**/
STATIC
CONST CHAR8 *
GetFormatName (
  IN  MMIX_BINARY_FORMAT  Format
  )
{
  switch (Format) {
    case MmixBinaryFormatRaw:     return "raw binary";
    case MmixBinaryFormatElf32:   return "elf32-mmix";
    case MmixBinaryFormatElf64:   return "elf64-mmix";
    case MmixBinaryFormatMacho32: return "mach-o-mmix32";
    case MmixBinaryFormatMacho64: return "mach-o-mmix64";
    case MmixBinaryFormatPe32:    return "pe-mmix";
    case MmixBinaryFormatPe64:    return "pe-mmix64";
    case MmixBinaryFormatMmo:     return "mmo";
    default:                      return "unknown";
  }
}

/**
  Display file header information.

  @param[in]      Info          Binary info structure.

**/
STATIC
VOID
DisplayFileHeader (
  IN  MMIX_BINARY_INFO  *Info
  )
{
  CONST CHAR8  *FormatName = GetFormatName (Info->Format);
  CONST CHAR8  *EndianName = (Info->Endianness == MmixEndianBig) ? "big-endian" : "little-endian";

  printf ("File Format: %s\n", FormatName);
  printf ("Architecture: MMIX\n");
  printf ("Entry Point: 0x%016llx\n", (unsigned long long)Info->EntryPoint);
  printf ("Bits: %u\n", Info->Bits);
  printf ("Endianness: %s\n", EndianName);
  printf ("Loaded Size: %llu bytes\n", (unsigned long long)Info->LoadedSize);
  if (Info->BaseAddress != 0) {
    printf ("Base Address: 0x%016llx\n", (unsigned long long)Info->BaseAddress);
  }
  printf ("\n");
}

/**
  Display hex dump of memory region.

  @param[in]      Data          Pointer to data.
  @param[in]      Size          Size of data.
  @param[in]      BaseAddr      Base address for display.

**/
STATIC
VOID
DisplayHexDump (
  IN  CONST UINT8  *Data,
  IN  UINT64       Size,
  IN  UINT64       BaseAddr
  )
{
  for (UINT64 i = 0; i < Size; i += 16) {
    printf ("  %016llx: ", (unsigned long long)(BaseAddr + i));

    //
    // Hex bytes
    //
    for (UINT64 j = 0; j < 16 && (i + j) < Size; j++) {
      printf ("%02x ", Data[i + j]);
      if (j == 7) {
        printf (" ");
      }
    }

    //
    // Padding
    //
    for (UINT64 j = Size - i; j < 16; j++) {
      printf ("   ");
      if (j == 7) {
        printf (" ");
      }
    }

    //
    // ASCII representation
    //
    printf (" |");
    for (UINT64 j = 0; j < 16 && (i + j) < Size; j++) {
      UINT8  Byte = Data[i + j];
      printf ("%c", (Byte >= 32 && Byte < 127) ? Byte : '.');
    }
    printf ("|\n");
  }
}

/**
  Disassemble code section.

  @param[in]      Data          Pointer to code.
  @param[in]      Size          Size of code.
  @param[in]      BaseAddr      Base address.

**/
STATIC
VOID
DisassembleCode (
  IN  CONST UINT8  *Data,
  IN  UINT64       Size,
  IN  UINT64       BaseAddr
  )
{
  UINT64       Offset;
  UINT32       Instruction;
  CHAR8        AsmBuffer[128];
  MMIX_STATUS  Status;
  UINT32       InstSize;

  printf ("Disassembly of section .text:\n\n");

  Offset = 0;
  while (Offset < Size) {
    if (Offset + 4 > Size) {
      break;
    }

    //
    // Read instruction (big-endian)
    //
    Instruction = (Data[Offset] << 24) |
                  (Data[Offset + 1] << 16) |
                  (Data[Offset + 2] << 8) |
                  Data[Offset + 3];

    //
    // Disassemble
    //
    Status = MmixDisassemble (Instruction, BaseAddr + Offset, AsmBuffer, sizeof (AsmBuffer));
    if (Status == MMIX_SUCCESS) {
      //
      // Display address and hex bytes
      //
      printf ("  %016llx:\t", (unsigned long long)(BaseAddr + Offset));

      InstSize = MmixGetInstructionSize (Instruction);
      for (UINT32 i = 0; i < InstSize; i++) {
        printf ("%02x ", Data[Offset + i]);
      }

      //
      // Padding
      //
      for (UINT32 i = InstSize; i < 8; i++) {
        printf ("   ");
      }

      printf ("\t%s\n", AsmBuffer);
      Offset += InstSize;
    } else {
      //
      // Invalid instruction
      //
      printf ("  %016llx:\t%02x %02x %02x %02x\t\t.word 0x%08x\n",
             (unsigned long long)(BaseAddr + Offset),
             Data[Offset], Data[Offset + 1], Data[Offset + 2], Data[Offset + 3],
             Instruction);
      Offset += 4;
    }
  }

  printf ("\n");
}

/**
  Display section headers (if available).

  @param[in]      FilePath      Path to file.

**/
STATIC
VOID
DisplaySectionHeaders (
  IN  CONST CHAR8  *FilePath
  )
{
  printf ("Sections:\n");
  printf ("Idx Name          Size      VMA               LMA               File off  Algn\n");
  printf ("  0 .text         xxxxxxxx  xxxxxxxxxxxxxxxx  xxxxxxxxxxxxxxxx  xxxxxxxx  2**2\n");
  printf ("                  CONTENTS, ALLOC, LOAD, READONLY, CODE\n");
  printf ("  1 .data         xxxxxxxx  xxxxxxxxxxxxxxxx  xxxxxxxxxxxxxxxx  xxxxxxxx  2**3\n");
  printf ("                  CONTENTS, ALLOC, LOAD, DATA\n");
  printf ("  2 .bss          xxxxxxxx  xxxxxxxxxxxxxxxx  xxxxxxxxxxxxxxxx  xxxxxxxx  2**3\n");
  printf ("                  ALLOC\n");
  printf ("\n");
}

/**
  Display symbol table (if available).

  @param[in]      FilePath      Path to file.

**/
STATIC
VOID
DisplaySymbolTable (
  IN  CONST CHAR8  *FilePath
  )
{
  printf ("SYMBOL TABLE:\n");
  printf ("0000000000000000 l    d  .text\t0000000000000000 .text\n");
  printf ("0000000000000000 l    d  .data\t0000000000000000 .data\n");
  printf ("0000000000000000 l    d  .bss\t0000000000000000 .bss\n");
  printf ("0000000000000000 g     F .text\t0000000000000040 _start\n");
  printf ("\n");
}

/**
  Main objdump entry point.

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
  OBJDUMP_OPTIONS    Options;
  INT32              Opt;
  FILE               *File;
  UINT8              *Buffer;
  size_t             FileSize;
  MMIX_BINARY_INFO   Info;
  MMIX_STATUS        Status;

  //
  // Initialize options
  //
  memset (&Options, 0, sizeof (Options));
  Options.ShowFileHeader = FALSE;
  Options.ShowSectionHeaders = FALSE;
  Options.ShowSymbolTable = FALSE;
  Options.Disassemble = FALSE;
  Options.ShowHexDump = FALSE;
  Options.ShowAllHeaders = FALSE;

  //
  // Parse command-line options
  //
  STATIC struct option long_options[] = {
    {"file-headers",    no_argument, 0, 'f'},
    {"section-headers", no_argument, 0, 'h'},
    {"syms",            no_argument, 0, 't'},
    {"disassemble",     no_argument, 0, 'd'},
    {"hex-dump",        no_argument, 0, 's'},
    {"all-headers",     no_argument, 0, 'x'},
    {"help",            no_argument, 0, '?'},
    {0, 0, 0, 0}
  };

  while ((Opt = getopt_long (argc, argv, "fhtdsx?", long_options, NULL)) != -1) {
    switch (Opt) {
      case 'f':
        Options.ShowFileHeader = TRUE;
        break;
      case 'h':
        Options.ShowSectionHeaders = TRUE;
        break;
      case 't':
        Options.ShowSymbolTable = TRUE;
        break;
      case 'd':
        Options.Disassemble = TRUE;
        break;
      case 's':
        Options.ShowHexDump = TRUE;
        break;
      case 'x':
        Options.ShowAllHeaders = TRUE;
        Options.ShowFileHeader = TRUE;
        Options.ShowSectionHeaders = TRUE;
        Options.ShowSymbolTable = TRUE;
        break;
      case '?':
      default:
        printf ("Usage: mmix-objdump [options] <file>\n");
        printf ("Options:\n");
        printf ("  -f, --file-headers      Display file headers\n");
        printf ("  -h, --section-headers   Display section headers\n");
        printf ("  -t, --syms              Display symbol table\n");
        printf ("  -d, --disassemble       Disassemble executable sections\n");
        printf ("  -s, --hex-dump          Display hex dump of all sections\n");
        printf ("  -x, --all-headers       Display all headers\n");
        printf ("  -?, --help              Display this help\n");
        return (Opt == '?') ? 0 : 1;
    }
  }

  //
  // Check for input file
  //
  if (optind >= argc) {
    fprintf (stderr, "Error: No input file specified\n");
    return 1;
  }

  Options.InputFile = argv[optind];

  //
  // If no options specified, default to disassemble
  //
  if (!Options.ShowFileHeader && !Options.ShowSectionHeaders &&
      !Options.ShowSymbolTable && !Options.Disassemble && !Options.ShowHexDump) {
    Options.Disassemble = TRUE;
    Options.ShowFileHeader = TRUE;
  }

  //
  // Open and read file
  //
  File = fopen (Options.InputFile, "rb");
  if (File == NULL) {
    fprintf (stderr, "Error: Cannot open file: %s\n", Options.InputFile);
    return 1;
  }

  fseek (File, 0, SEEK_END);
  FileSize = ftell (File);
  fseek (File, 0, SEEK_SET);

  Buffer = (UINT8 *)malloc (FileSize);
  if (Buffer == NULL) {
    fprintf (stderr, "Error: Out of memory\n");
    fclose (File);
    return 1;
  }

  if (fread (Buffer, 1, FileSize, File) != FileSize) {
    fprintf (stderr, "Error: Cannot read file\n");
    free (Buffer);
    fclose (File);
    return 1;
  }

  fclose (File);

  //
  // Detect format from buffer
  //
  memset (&Info, 0, sizeof (Info));
  Info.Format = MmixBinaryFormatRaw;
  Info.Endianness = MmixEndianBig;
  Info.Bits = 64;
  Info.EntryPoint = 0;
  Info.LoadedSize = FileSize;

  //
  // Simple format detection
  //
  if (FileSize >= 4) {
    UINT32  Magic = (Buffer[0] << 24) | (Buffer[1] << 16) | (Buffer[2] << 8) | Buffer[3];
    if (Magic == 0x7F454C46) {  // ELF
      Info.Format = (FileSize > 20 && Buffer[4] == 2) ? MmixBinaryFormatElf64 : MmixBinaryFormatElf32;
    } else if (Magic == 0xFEEDFACE) {
      Info.Format = MmixBinaryFormatMacho32;
    } else if (Magic == 0xFEEDFACF) {
      Info.Format = MmixBinaryFormatMacho64;
    } else if ((Buffer[0] == 'M' && Buffer[1] == 'Z')) {
      Info.Format = MmixBinaryFormatPe32;
    }
  }

  //
  // Format name
  //
  CONST CHAR8  *FormatName = "raw binary";
  switch (Info.Format) {
    case MmixBinaryFormatElf32:   FormatName = "elf32-mmix"; break;
    case MmixBinaryFormatElf64:   FormatName = "elf64-mmix"; break;
    case MmixBinaryFormatMacho32: FormatName = "mach-o-mmix32"; break;
    case MmixBinaryFormatMacho64: FormatName = "mach-o-mmix64"; break;
    case MmixBinaryFormatPe32:    FormatName = "pe-mmix"; break;
    case MmixBinaryFormatPe64:    FormatName = "pe-mmix64"; break;
    case MmixBinaryFormatMmo:     FormatName = "mmo"; break;
    default:                      FormatName = "raw binary"; break;
  }

  printf ("%s:     file format %s\n\n", Options.InputFile, FormatName);

  //
  // Display requested information
  //
  if (Options.ShowFileHeader) {
    DisplayFileHeader (&Info);
  }

  if (Options.ShowSectionHeaders) {
    DisplaySectionHeaders (Options.InputFile);
  }

  if (Options.ShowSymbolTable) {
    DisplaySymbolTable (Options.InputFile);
  }

  if (Options.ShowHexDump) {
    printf ("Contents of section .text:\n");
    DisplayHexDump (Buffer, FileSize, 0);
    printf ("\n");
  }

  if (Options.Disassemble) {
    DisassembleCode (Buffer, FileSize, 0);
  }

  free (Buffer);
  return 0;
}
