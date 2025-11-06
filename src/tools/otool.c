/** @file
  otool - macOS object file examining tool.

  This is a wrapper around objdump that provides macOS-style options.
  For full otool compatibility, use the actual macOS otool.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/**
  Main entry point.
**/
int
main (
  int   argc,
  char  **argv
  )
{
  char   **newargv;
  int    i, j;
  int    newargc;

  //
  // Allocate new argument array
  //
  newargv = malloc (sizeof (char *) * (argc + 10));
  if (newargv == NULL) {
    fprintf (stderr, "otool: out of memory\n");
    return 1;
  }

  newargv[0] = "objdump";
  newargc    = 1;

  //
  // Convert otool options to objdump options
  //
  for (i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-h") == 0) {
      //
      // Print otool-style help
      //
      printf ("Usage: otool [options] file...\n");
      printf ("macOS object file examining tool (implemented via objdump)\n\n");
      printf ("Common options:\n");
      printf ("  -f            Display universal headers\n");
      printf ("  -h            Display Mach-O headers\n");
      printf ("  -l            Display load commands\n");
      printf ("  -L            Display shared libraries\n");
      printf ("  -D            Display shared library id\n");
      printf ("  -t            Display text section\n");
      printf ("  -d            Display data section\n");
      printf ("  -o            Display Objective-C segment\n");
      printf ("  -r            Display relocation entries\n");
      printf ("  -S            Display contents of __.SYMDEF\n");
      printf ("  -v            Verbose output\n");
      printf ("  -V            Display version\n");
      printf ("\nThis is a wrapper around objdump. For full otool compatibility,\n");
      printf ("use the actual macOS otool.\n");
      free (newargv);
      return 0;
    } else if (strcmp (argv[i], "-f") == 0) {
      newargv[newargc++] = "-f";  // File headers
    } else if (strcmp (argv[i], "-h") == 0) {
      newargv[newargc++] = "-h";  // Section headers
    } else if (strcmp (argv[i], "-l") == 0 || strcmp (argv[i], "-L") == 0) {
      newargv[newargc++] = "-p";  // Private headers (load commands)
    } else if (strcmp (argv[i], "-t") == 0 || strcmp (argv[i], "-d") == 0) {
      newargv[newargc++] = "-s";  // Full contents
    } else if (strcmp (argv[i], "-r") == 0) {
      newargv[newargc++] = "-r";  // Relocations
    } else if (strcmp (argv[i], "-V") == 0) {
      printf ("otool (MMIX toolchain) version 1.0\n");
      printf ("macOS-compatible object file examining tool\n");
      printf ("Implemented as wrapper around objdump\n");
      free (newargv);
      return 0;
    } else {
      //
      // Pass through other arguments
      //
      newargv[newargc++] = argv[i];
    }
  }

  newargv[newargc] = NULL;

  //
  // Execute objdump
  //
  execvp ("objdump", newargv);

  //
  // If we get here, exec failed
  //
  fprintf (stderr, "otool: failed to execute objdump\n");
  free (newargv);
  return 1;
}
