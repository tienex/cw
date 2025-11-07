/** @file
  Test program for libswitches - cross-platform option parser.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#include <stdio.h>
#include "switches/Switches.h"

INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  SWITCH_CONTEXT       *Context;
  CONST CHAR8          *Input;
  CONST CHAR8          *Output;
  INT64                Count;
  BOOLEAN              Verbose;
  CHAR8                **Args;
  UINT32               ArgCount;
  UINT32               i;

  //
  // Define options
  //
  static CONST SWITCH_OPTION Options[] = {
    SWITCH_OPTION_BOOL ('v', "verbose", "Enable verbose output"),
    SWITCH_OPTION_STRING ('i', "input", "FILE", "Input file path"),
    SWITCH_OPTION_STRING ('o', "output", "FILE", "Output file path"),
    SWITCH_OPTION_INT ('c', "count", "NUM", "Number of iterations"),
    SWITCH_OPTION_BOOL ('h', "help", "Show this help message"),
    SWITCH_OPTION_END ()
  };

  //
  // Create parser context
  //
  Context = SwitchCreate (
              "switchtest",
              "Test program for cross-platform option parsing library",
              Options,
              SwitchStyleAuto
              );

  if (Context == NULL) {
    fprintf (stderr, "switchtest: failed to create parser context\n");
    return 1;
  }

  //
  // Parse arguments
  //
  if (!SwitchParse (Context, argc, argv)) {
    SwitchFree (Context);
    return 1;
  }

  //
  // Check for help
  //
  if (SwitchIsPresent (Context, "help")) {
    SwitchPrintHelp (Context, ColorOutputAuto);
    SwitchFree (Context);
    return 0;
  }

  //
  // Get option values
  //
  Verbose = SwitchGetBool (Context, "verbose", FALSE);
  Input = SwitchGetString (Context, "input", NULL);
  Output = SwitchGetString (Context, "output", NULL);
  Count = SwitchGetInt (Context, "count", 1);

  //
  // Get positional arguments
  //
  Args = SwitchGetArguments (Context, &ArgCount);

  //
  // Display parsed options
  //
  printf ("Parsed options:\n");
  printf ("  Verbose: %s\n", Verbose ? "yes" : "no");
  printf ("  Input: %s\n", Input ? Input : "(none)");
  printf ("  Output: %s\n", Output ? Output : "(none)");
  printf ("  Count: %lld\n", (long long)Count);

  if (ArgCount > 0) {
    printf ("  Positional arguments:\n");
    for (i = 0; i < ArgCount; i++) {
      printf ("    %u: %s\n", i + 1, Args[i]);
    }
  }

  //
  // Clean up
  //
  SwitchFree (Context);
  return 0;
}
