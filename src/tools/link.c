/** @file
  link - Windows linker tool.

  This is a wrapper around ld that provides Windows-style options.
  For full link.exe compatibility, use the actual Microsoft link tool.

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
    fprintf (stderr, "link: out of memory\n");
    return 1;
  }

  newargv[0] = "ld";
  newargc    = 1;

  //
  // Parse link.exe options and convert to ld options
  //
  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '/' || argv[i][0] == '-') {
      char *opt = argv[i] + 1;

      if (strcmp (opt, "?") == 0 || strcmp (opt, "HELP") == 0) {
        //
        // Print link-style help
        //
        printf ("Usage: link [options] file...\n");
        printf ("Windows linker (implemented via ld)\n\n");
        printf ("Common options:\n");
        printf ("  /OUT:filename     Output file name\n");
        printf ("  /DLL              Build a DLL\n");
        printf ("  /SUBSYSTEM:type   Set subsystem (CONSOLE, WINDOWS, etc.)\n");
        printf ("  /ENTRY:symbol     Set entry point\n");
        printf ("  /LIBPATH:dir      Add library search path\n");
        printf ("  /MACHINE:target   Specify target machine\n");
        printf ("  /DEBUG            Generate debug information\n");
        printf ("  /MAP[:filename]   Generate map file\n");
        printf ("  /NOLOGO           Suppress copyright message\n");
        printf ("  /VERBOSE          Verbose output\n");
        printf ("  /?                Display this help\n");
        printf ("\nThis is a wrapper around ld. For full link.exe compatibility,\n");
        printf ("use the actual Microsoft link tool.\n\n");
        printf ("Equivalent ld commands:\n");
        printf ("  Link executable:  link /OUT:foo.exe a.obj b.obj  =>  ld -o foo.exe a.obj b.obj\n");
        printf ("  Build DLL:        link /DLL /OUT:foo.dll a.obj   =>  ld -shared -o foo.dll a.obj\n");
        printf ("  Set entry:        link /ENTRY:main a.obj         =>  ld -e main a.obj\n");
        printf ("  Add lib path:     link /LIBPATH:C:\\libs a.obj    =>  ld -L C:/libs a.obj\n");
        free (newargv);
        return 0;
      } else if (strncmp (opt, "OUT:", 4) == 0) {
        newargv[newargc++] = "-o";
        newargv[newargc++] = opt + 4;
      } else if (strcmp (opt, "DLL") == 0) {
        newargv[newargc++] = "-shared";
      } else if (strncmp (opt, "SUBSYSTEM:", 10) == 0) {
        // ld doesn't have direct subsystem equivalent, ignore
      } else if (strncmp (opt, "ENTRY:", 6) == 0) {
        newargv[newargc++] = "-e";
        newargv[newargc++] = opt + 6;
      } else if (strncmp (opt, "LIBPATH:", 8) == 0) {
        newargv[newargc++] = "-L";
        newargv[newargc++] = opt + 8;
      } else if (strncmp (opt, "MACHINE:", 8) == 0) {
        // Machine type mapping would go here
      } else if (strcmp (opt, "DEBUG") == 0) {
        newargv[newargc++] = "-g";
      } else if (strncmp (opt, "MAP", 3) == 0) {
        if (opt[3] == ':') {
          newargv[newargc++] = "-Map";
          newargv[newargc++] = opt + 4;
        } else {
          newargv[newargc++] = "-Map";
          newargv[newargc++] = "output.map";
        }
      } else if (strcmp (opt, "VERBOSE") == 0) {
        newargv[newargc++] = "--verbose";
      } else if (strcmp (opt, "NOLOGO") == 0) {
        // Ignore
      } else {
        //
        // Unknown option, try to pass through
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

  newargv[newargc] = NULL;

  //
  // Execute ld
  //
  execvp ("ld", newargv);

  //
  // If we get here, exec failed
  //
  fprintf (stderr, "link: failed to execute ld\n");
  free (newargv);
  return 1;
}
