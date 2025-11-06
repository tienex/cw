/** @file
  dumpbin - Windows COFF/PE dumping utility.

  This is a wrapper around objdump that provides Windows-style options.
  For full dumpbin compatibility, use the actual Windows dumpbin.

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
  int    i;
  int    newargc;

  //
  // Allocate new argument array
  //
  newargv = malloc (sizeof (char *) * (argc + 10));
  if (newargv == NULL) {
    fprintf (stderr, "dumpbin: out of memory\n");
    return 1;
  }

  newargv[0] = "objdump";
  newargc    = 1;

  //
  // Convert dumpbin options to objdump options
  //
  for (i = 1; i < argc; i++) {
    if (strcmp (argv[i], "/?") == 0 || strcmp (argv[i], "/HELP") == 0) {
      //
      // Print dumpbin-style help
      //
      printf ("Usage: dumpbin [options] file...\n");
      printf ("Windows COFF/PE dumping utility (implemented via objdump)\n\n");
      printf ("Common options:\n");
      printf ("  /ALL              Display all information\n");
      printf ("  /ARCHIVEMEMBERS   Display archive members\n");
      printf ("  /DEPENDENTS       Display dependent DLLs\n");
      printf ("  /DIRECTIVES       Display compiler directives\n");
      printf ("  /EXPORTS          Display exported symbols\n");
      printf ("  /HEADERS          Display file headers\n");
      printf ("  /IMPORTS          Display imported symbols\n");
      printf ("  /LINENUMBERS      Display line numbers\n");
      printf ("  /RELOCATIONS      Display relocations\n");
      printf ("  /SECTION:name     Display section\n");
      printf ("  /SYMBOLS          Display symbol table\n");
      printf ("  /SUMMARY          Display summary only\n");
      printf ("\nThis is a wrapper around objdump. For full dumpbin compatibility,\n");
      printf ("use the actual Windows dumpbin.\n");
      free (newargv);
      return 0;
    } else if (strcmp (argv[i], "/ALL") == 0) {
      newargv[newargc++] = "-x";  // All headers
      newargv[newargc++] = "-t";  // Symbols
      newargv[newargc++] = "-r";  // Relocations
    } else if (strcmp (argv[i], "/HEADERS") == 0) {
      newargv[newargc++] = "-f";  // File headers
      newargv[newargc++] = "-h";  // Section headers
    } else if (strcmp (argv[i], "/SYMBOLS") == 0) {
      newargv[newargc++] = "-t";  // Symbol table
    } else if (strcmp (argv[i], "/RELOCATIONS") == 0) {
      newargv[newargc++] = "-r";  // Relocations
    } else if (strncmp (argv[i], "/SECTION:", 9) == 0) {
      newargv[newargc++] = "-j";
      newargv[newargc++] = argv[i] + 9;
      newargv[newargc++] = "-s";  // Full contents
    } else {
      //
      // Pass through other arguments (removing / prefix if present)
      //
      if (argv[i][0] == '/') {
        //
        // Skip Windows-style options we don't understand
        //
        fprintf (stderr, "dumpbin: warning: ignoring option %s\n", argv[i]);
      } else {
        newargv[newargc++] = argv[i];
      }
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
  fprintf (stderr, "dumpbin: failed to execute objdump\n");
  free (newargv);
  return 1;
}
