/** @file
  strings - Extract printable strings from binary files.

  Universal string extractor that works on any binary file.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include "binformat/BinFormat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

typedef struct {
  INT32    MinLength;        ///< Minimum string length (default 4)
  BOOLEAN  PrintOffset;      ///< Print offset of string
  BOOLEAN  PrintFileName;    ///< Print file name before each string
  INT32    Radix;            ///< Offset radix: 8, 10, or 16
} STRINGS_OPTIONS;

STATIC STRINGS_OPTIONS  gOptions = {
  .MinLength      = 4,
  .PrintOffset    = FALSE,
  .PrintFileName  = FALSE,
  .Radix          = 10
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
  printf ("Extract printable strings from binary files\n\n");
  printf ("Options:\n");
  printf ("  -n, --bytes=NUMBER     Minimum string length (default 4)\n");
  printf ("  -t, --radix={o,d,x}    Print offset in octal, decimal, or hex\n");
  printf ("  -f, --print-file-name  Print file name before each string\n");
  printf ("  -h, --help             Display this information\n");
  printf ("  -V, --version          Display version information\n");
}

/**
  Check if character is printable.
**/
STATIC
BOOLEAN
IsPrintable (
  IN CHAR8  c
  )
{
  return (c >= ' ' && c <= '~') || c == '\t';
}

/**
  Extract strings from data buffer.
**/
STATIC
VOID
ExtractStrings (
  IN CONST CHAR8  *FileName,
  IN CONST UINT8  *Data,
  IN UINT64       Size
  )
{
  UINT64  i;
  UINT64  StringStart;
  INT32   StringLength;
  CHAR8   Buffer[4096];

  StringStart  = 0;
  StringLength = 0;

  for (i = 0; i < Size; i++) {
    if (IsPrintable (Data[i])) {
      if (StringLength == 0) {
        StringStart = i;
      }
      if (StringLength < (INT32)sizeof (Buffer) - 1) {
        Buffer[StringLength] = Data[i];
      }
      StringLength++;
    } else {
      //
      // End of string
      //
      if (StringLength >= gOptions.MinLength) {
        //
        // Null-terminate
        //
        if (StringLength >= (INT32)sizeof (Buffer)) {
          StringLength = (INT32)sizeof (Buffer) - 1;
        }
        Buffer[StringLength] = '\0';

        //
        // Print file name if requested
        //
        if (gOptions.PrintFileName) {
          printf ("%s: ", FileName);
        }

        //
        // Print offset if requested
        //
        if (gOptions.PrintOffset) {
          if (gOptions.Radix == 8) {
            printf ("%7llo ", (unsigned long long)StringStart);
          } else if (gOptions.Radix == 16) {
            printf ("%7llx ", (unsigned long long)StringStart);
          } else {
            printf ("%7llu ", (unsigned long long)StringStart);
          }
        }

        printf ("%s\n", Buffer);
      }

      StringStart  = 0;
      StringLength = 0;
    }
  }

  //
  // Handle string at end of file
  //
  if (StringLength >= gOptions.MinLength) {
    if (StringLength >= (INT32)sizeof (Buffer)) {
      StringLength = (INT32)sizeof (Buffer) - 1;
    }
    Buffer[StringLength] = '\0';

    if (gOptions.PrintFileName) {
      printf ("%s: ", FileName);
    }

    if (gOptions.PrintOffset) {
      if (gOptions.Radix == 8) {
        printf ("%7llo ", (unsigned long long)StringStart);
      } else if (gOptions.Radix == 16) {
        printf ("%7llx ", (unsigned long long)StringStart);
      } else {
        printf ("%7llu ", (unsigned long long)StringStart);
      }
    }

    printf ("%s\n", Buffer);
  }
}

/**
  Process a file and extract strings.
**/
STATIC
INT32
ProcessFile (
  IN CONST CHAR8  *FileName
  )
{
  BINFORMAT_STREAM  Stream;
  BINFORMAT_STATUS  Status;
  UINT8             *Data;

  //
  // Initialize stream from file (load entire file)
  //
  Status = BinFormatStreamInitFile (&Stream, FileName, BINFORMAT_STREAM_READ);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "strings: %s: Cannot read file\n", FileName);
    return 1;
  }

  //
  // Get direct pointer to data
  //
  Data = (UINT8 *)BinFormatStreamGetDataPointer (&Stream);
  if (Data == NULL) {
    fprintf (stderr, "strings: %s: Cannot access file data\n", FileName);
    BinFormatStreamClose (&Stream);
    return 1;
  }

  //
  // Extract strings
  //
  ExtractStrings (FileName, Data, Stream.Size);

  //
  // Close stream
  //
  BinFormatStreamClose (&Stream);

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
    {"bytes",           required_argument, 0, 'n'},
    {"radix",           required_argument, 0, 't'},
    {"print-file-name", no_argument,       0, 'f'},
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  while ((c = getopt_long (argc, argv, "n:t:fhV", long_options, &option_index)) != -1) {
    switch (c) {
      case 'n':
        gOptions.MinLength = atoi (optarg);
        if (gOptions.MinLength < 1) {
          gOptions.MinLength = 1;
        }
        break;
      case 't':
        gOptions.PrintOffset = TRUE;
        if (strcmp (optarg, "o") == 0) {
          gOptions.Radix = 8;
        } else if (strcmp (optarg, "d") == 0) {
          gOptions.Radix = 10;
        } else if (strcmp (optarg, "x") == 0) {
          gOptions.Radix = 16;
        }
        break;
      case 'f':
        gOptions.PrintFileName = TRUE;
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("strings (MMIX toolchain) 1.0\n");
        return 0;
      default:
        PrintUsage (argv[0]);
        return 1;
    }
  }

  if (optind >= argc) {
    fprintf (stderr, "strings: No input files specified\n");
    PrintUsage (argv[0]);
    return 1;
  }

  //
  // Process each file
  //
  for (INT32 i = optind; i < argc; i++) {
    result |= ProcessFile (argv[i]);
  }

  return result;
}
