/** @file
  redo_prebinding - Redo prebinding for Mach-O files.

  Stub implementation showing that prebinding is deprecated in modern macOS.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include <stdio.h>
#include <stdlib.h>

/**
  Main entry point.
**/
int
main (
  int   argc,
  char  **argv
  )
{
  if (argc < 2) {
    printf ("Usage: redo_prebinding file...\n");
    printf ("Redo prebinding information for Mach-O binaries\n\n");
    printf ("Note: Prebinding is deprecated as of Mac OS X 10.3.4 (2004).\n");
    printf ("Modern macOS uses dyld shared cache instead.\n\n");
    printf ("This tool is provided for historical compatibility only.\n");
    printf ("For production use on older systems, use the actual macOS redo_prebinding.\n");
    return 1;
  }

  printf ("redo_prebinding: prebinding is deprecated\n");
  printf ("\nHistorical note:\n");
  printf ("  Prebinding was used to speed up application launch times by pre-linking\n");
  printf ("  libraries at installation time. It was replaced by the dyld shared cache\n");
  printf ("  which provides better performance and doesn't require per-app prebinding.\n");
  printf ("\n");
  printf ("  On modern macOS (10.4+), the dynamic linker handles optimization automatically.\n");

  return 0;
}
