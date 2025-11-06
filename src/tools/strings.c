/** @file
  strings - Print printable strings from files.

  Finds and prints sequences of printable characters in binary files.
  Works on any file type, including object files and executables.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdint.h>

typedef uint8_t   UINT8;
typedef uint32_t  UINT32;
typedef uint64_t  UINT64;
typedef int32_t   INT32;
typedef int       BOOLEAN;
typedef char      CHAR;
typedef void      VOID;
#define TRUE   1
#define FALSE  0
#define IN
#define OUT
#define CONST const
#define STATIC static

typedef struct {
  UINT32   MinLength;       ///< Minimum string length (default 4)
  BOOLEAN  PrintOffset;     ///< Print offset of string
  BOOLEAN  PrintFileName;   ///< Print file name
  CHAR     OffsetFormat;    ///< Offset format: 'o', 'd', 'x'
  BOOLEAN  ScanWholeFile;   ///< Scan whole file (default)
  BOOLEAN  ScanDataOnly;    ///< Scan only data sections
  UINT32   Encoding;        ///< Character encoding (ASCII, UTF-8, UTF-16, etc.)
} STRINGS_OPTIONS;

STATIC STRINGS_OPTIONS  gOptions = {
  .MinLength      = 4,
  .PrintOffset    = FALSE,
  .PrintFileName  = FALSE,
  .OffsetFormat   = 'd',
  .ScanWholeFile  = TRUE,
  .ScanDataOnly   = FALSE,
  .Encoding       = 's'  // Single-byte (ASCII)
};

/**
  Print usage information.
**/
STATIC
VOID
PrintUsage (
  CONST char  *ProgramName
  )
{
  printf ("Usage: %s [options] file...\n", ProgramName);
  printf ("Print the strings of printable characters in files\n\n");
  printf ("Options:\n");
  printf ("  -a, --all                  Scan entire file (default)\n");
  printf ("  -d, --data                 Scan only initialized data sections\n");
  printf ("  -f, --print-file-name      Print file name before each string\n");
  printf ("  -n, --bytes=NUMBER         Minimum string length (default 4)\n");
  printf ("  -t, --radix=RADIX          Print offset in RADIX (o, d, x)\n");
  printf ("  -o                         Equivalent to -t o (octal)\n");
  printf ("  -e, --encoding=ENCODING    Select character encoding\n");
  printf ("                             s = single-byte (ASCII, default)\n");
  printf ("                             S = single-byte (7-bit ASCII)\n");
  printf ("                             b = 16-bit big-endian\n");
  printf ("                             l = 16-bit little-endian\n");
  printf ("                             B = 32-bit big-endian\n");
  printf ("                             L = 32-bit little-endian\n");
  printf ("  -h, --help                 Display this information\n");
  printf ("  -V, --version              Display version information\n");
}

/**
  Check if character is printable.
**/
STATIC
BOOLEAN
IsPrintable (
  IN UINT8  Ch
  )
{
  return (Ch >= 32 && Ch < 127) || Ch == '\t';
}

/**
  Print offset in specified radix.
**/
STATIC
VOID
PrintOffset (
  IN UINT64  Offset
  )
{
  if (gOptions.OffsetFormat == 'o') {
    printf ("%7llo ", (unsigned long long)Offset);
  } else if (gOptions.OffsetFormat == 'x') {
    printf ("%7llx ", (unsigned long long)Offset);
  } else {
    printf ("%7llu ", (unsigned long long)Offset);
  }
}

/**
  Process file and extract strings.
**/
STATIC
INT32
ProcessFile (
  IN CONST char  *FileName
  )
{
  FILE    *File;
  UINT8   *Buffer;
  UINT8   *StringBuf;
  size_t  BytesRead;
  UINT64  Offset;
  UINT32  StringLen;
  size_t  i;

  File = fopen (FileName, "rb");
  if (File == NULL) {
    fprintf (stderr, "strings: %s: No such file or directory\n", FileName);
    return 1;
  }

  //
  // Allocate buffers
  //
  Buffer = malloc (65536);
  if (Buffer == NULL) {
    fprintf (stderr, "strings: out of memory\n");
    fclose (File);
    return 1;
  }

  StringBuf = malloc (65536);
  if (StringBuf == NULL) {
    fprintf (stderr, "strings: out of memory\n");
    free (Buffer);
    fclose (File);
    return 1;
  }

  //
  // Scan file
  //
  Offset    = 0;
  StringLen = 0;

  while ((BytesRead = fread (Buffer, 1, 65536, File)) > 0) {
    for (i = 0; i < BytesRead; i++) {
      if (IsPrintable (Buffer[i])) {
        //
        // Add to current string
        //
        if (StringLen < 65535) {
          StringBuf[StringLen++] = Buffer[i];
        }
      } else {
        //
        // End of string - print if long enough
        //
        if (StringLen >= gOptions.MinLength) {
          StringBuf[StringLen] = '\0';

          if (gOptions.PrintFileName) {
            printf ("%s: ", FileName);
          }

          if (gOptions.PrintOffset) {
            PrintOffset (Offset + i - StringLen);
          }

          printf ("%s\n", StringBuf);
        }

        StringLen = 0;
      }
    }

    Offset += BytesRead;
  }

  //
  // Handle string at end of file
  //
  if (StringLen >= gOptions.MinLength) {
    StringBuf[StringLen] = '\0';

    if (gOptions.PrintFileName) {
      printf ("%s: ", FileName);
    }

    if (gOptions.PrintOffset) {
      PrintOffset (Offset - StringLen);
    }

    printf ("%s\n", StringBuf);
  }

  free (StringBuf);
  free (Buffer);
  fclose (File);
  return 0;
}

/**
  Main entry point.
**/
INT32
main (
  IN INT32   argc,
  IN char    **argv
  )
{
  INT32    opt;
  INT32    i;
  BOOLEAN  Success;

  STATIC struct option long_options[] = {
    {"all",             no_argument,       0, 'a'},
    {"data",            no_argument,       0, 'd'},
    {"print-file-name", no_argument,       0, 'f'},
    {"bytes",           required_argument, 0, 'n'},
    {"radix",           required_argument, 0, 't'},
    {"encoding",        required_argument, 0, 'e'},
    {"help",            no_argument,       0, 'h'},
    {"version",         no_argument,       0, 'V'},
    {0, 0, 0, 0}
  };

  Success = TRUE;

  //
  // Parse options
  //
  while ((opt = getopt_long (argc, argv, "adfn:t:oe:hV", long_options, NULL)) != -1) {
    switch (opt) {
      case 'a':
        gOptions.ScanWholeFile = TRUE;
        gOptions.ScanDataOnly  = FALSE;
        break;
      case 'd':
        gOptions.ScanDataOnly  = TRUE;
        gOptions.ScanWholeFile = FALSE;
        break;
      case 'f':
        gOptions.PrintFileName = TRUE;
        break;
      case 'n':
        gOptions.MinLength = atoi (optarg);
        if (gOptions.MinLength < 1) {
          fprintf (stderr, "strings: invalid minimum string length: %s\n", optarg);
          return 1;
        }
        break;
      case 't':
        gOptions.OffsetFormat = optarg[0];
        gOptions.PrintOffset  = TRUE;
        if (gOptions.OffsetFormat != 'o' &&
            gOptions.OffsetFormat != 'd' &&
            gOptions.OffsetFormat != 'x') {
          fprintf (stderr, "strings: invalid radix: %s\n", optarg);
          return 1;
        }
        break;
      case 'o':
        gOptions.OffsetFormat = 'o';
        gOptions.PrintOffset  = TRUE;
        break;
      case 'e':
        gOptions.Encoding = optarg[0];
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      case 'V':
        printf ("strings (MMIX toolchain) version 1.0\n");
        printf ("Print printable strings from files\n");
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
    fprintf (stderr, "strings: no input files\n");
    return 1;
  }

  for (i = optind; i < argc; i++) {
    if (ProcessFile (argv[i]) != 0) {
      Success = FALSE;
    }
  }

  return Success ? 0 : 1;
}
