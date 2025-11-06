/** @file
  lib - Windows library manager tool.

  This is a wrapper around ar that provides Windows-style options.
  For full lib.exe compatibility, use the actual Microsoft lib tool.

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
  char   *outfile;

  //
  // Allocate new argument array
  //
  newargv = malloc (sizeof (char *) * (argc + 10));
  if (newargv == NULL) {
    fprintf (stderr, "lib: out of memory\n");
    return 1;
  }

  newargv[0] = "ar";
  newargc    = 1;
  outfile    = NULL;

  //
  // Parse lib.exe options and convert to ar options
  //
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '/' || argv[i][0] == '-') {
      char *opt = argv[i] + 1;

      if (strcmp (opt, "?") == 0 || strcmp (opt, "HELP") == 0) {
        //
        // Print lib-style help
        //
        printf ("Usage: lib [options] file...\n");
        printf ("Windows library manager (implemented via ar)\n\n");
        printf ("Common options:\n");
        printf ("  /OUT:library      Output library name\n");
        printf ("  /LIST             List contents of library\n");
        printf ("  /EXTRACT:member   Extract member from library\n");
        printf ("  /REMOVE:member    Remove member from library\n");
        printf ("  /NOLOGO           Suppress copyright message\n");
        printf ("  /VERBOSE          Verbose output\n");
        printf ("  /MACHINE:target   Specify target machine (ignored)\n");
        printf ("  /?                Display this help\n");
        printf ("\nThis is a wrapper around ar. For full lib.exe compatibility,\n");
        printf ("use the actual Microsoft lib tool.\n\n");
        printf ("Equivalent ar commands:\n");
        printf ("  Create library:   lib /OUT:foo.lib a.obj b.obj  =>  ar rcs foo.lib a.obj b.obj\n");
        printf ("  List contents:    lib /LIST foo.lib             =>  ar t foo.lib\n");
        printf ("  Extract member:   lib /EXTRACT:a.obj foo.lib    =>  ar x foo.lib a.obj\n");
        printf ("  Remove member:    lib /REMOVE:a.obj foo.lib     =>  ar d foo.lib a.obj\n");
        free (newargv);
        return 0;
      } else if (strncmp (opt, "OUT:", 4) == 0) {
        outfile = opt + 4;
      } else if (strcmp (opt, "LIST") == 0) {
        newargv[newargc++] = "t";  // List table of contents
      } else if (strncmp (opt, "EXTRACT:", 8) == 0) {
        newargv[newargc++] = "x";  // Extract
        newargv[newargc++] = opt + 8;
      } else if (strncmp (opt, "REMOVE:", 7) == 0) {
        newargv[newargc++] = "d";  // Delete
        newargv[newargc++] = opt + 7;
      } else if (strcmp (opt, "VERBOSE") == 0) {
        newargv[newargc++] = "v";  // Verbose
      } else if (strcmp (opt, "NOLOGO") == 0) {
        // Ignore
      } else if (strncmp (opt, "MACHINE:", 8) == 0) {
        // Ignore machine type
      } else {
        //
        // Unknown option, pass through
        //
        newargv[newargc++] = argv[i];
      }
    } else {
      //
      // Regular file argument
      //
      newargv[newargc++] = argv[i];
    }
  }

  //
  // If no operation specified, default to create/update
  //
  if (newargc == 1 || (newargc == 2 && newargv[1][0] != '-')) {
    newargv[newargc++] = "rcs";  // Replace, create, symbol table
  }

  //
  // Add output file if specified
  //
  if (outfile != NULL) {
    newargv[newargc++] = outfile;
  }

  newargv[newargc] = NULL;

  //
  // Execute ar
  //
  execvp ("ar", newargv);

  //
  // If we get here, exec failed
  //
  fprintf (stderr, "lib: failed to execute ar\n");
  free (newargv);
  return 1;
}
