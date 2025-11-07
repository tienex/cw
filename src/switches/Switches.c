/** @file
  Cross-platform Command-Line Option Parser Implementation.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "switches/Switches.h"

///
/// Maximum number of options
///
#define MAX_OPTIONS  128

///
/// Maximum number of positional arguments
///
#define MAX_ARGS  256

///
/// ANSI color codes
///
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_CYAN    "\033[36m"

///
/// Parser context structure
///
struct _SWITCH_CONTEXT {
  CONST CHAR8          *ProgramName;
  CONST CHAR8          *Description;
  CONST SWITCH_OPTION  *Options;
  SWITCH_STYLE         Style;
  SWITCH_RESULT        Results[MAX_OPTIONS];
  UINT32               ResultCount;
  CHAR8                *Arguments[MAX_ARGS];
  UINT32               ArgumentCount;
};

/**
  Detect if output supports color.

  @retval TRUE   TTY supports color.
  @retval FALSE  No color support.
**/
STATIC
BOOLEAN
SupportsColor (
  VOID
  )
{
  CHAR8  *Term;

  if (!isatty (STDOUT_FILENO)) {
    return FALSE;
  }

  Term = getenv ("TERM");
  if (Term == NULL) {
    return FALSE;
  }

  if (strstr (Term, "color") != NULL ||
      strcmp (Term, "xterm") == 0 ||
      strcmp (Term, "xterm-256color") == 0 ||
      strcmp (Term, "screen") == 0) {
    return TRUE;
  }

  return FALSE;
}

/**
  Get color string.

  @param[in]  Color        Color code.
  @param[in]  ColorOutput  Color output mode.

  @return Color string or empty string.
**/
STATIC
CONST CHAR8 *
GetColor (
  IN  CONST CHAR8   *Color,
  IN  COLOR_OUTPUT  ColorOutput
  )
{
  if (ColorOutput == ColorOutputNever) {
    return "";
  }

  if (ColorOutput == ColorOutputAuto && !SupportsColor ()) {
    return "";
  }

  return Color;
}

/**
  Determine option style for platform.

  @param[in]  Style  Requested style.

  @return Actual style to use.
**/
STATIC
SWITCH_STYLE
DetermineStyle (
  IN  SWITCH_STYLE  Style
  )
{
  if (Style != SwitchStyleAuto) {
    return Style;
  }

#if defined(__VMS)
  return SwitchStyleVMS;
#elif defined(_WIN32) || defined(__MSDOS__)
  return SwitchStyleDOS;
#else
  return SwitchStyleUnix;
#endif
}

/**
  Find option by long name.

  @param[in]  Options   Option array.
  @param[in]  LongName  Long name to find.

  @return Option descriptor or NULL.
**/
STATIC
CONST SWITCH_OPTION *
FindOptionByLongName (
  IN  CONST SWITCH_OPTION  *Options,
  IN  CONST CHAR8          *LongName
  )
{
  UINT32  i;

  for (i = 0; Options[i].LongName != NULL || Options[i].ShortName != 0; i++) {
    if (Options[i].LongName != NULL && strcmp (Options[i].LongName, LongName) == 0) {
      return &Options[i];
    }
  }

  return NULL;
}

/**
  Find option by short name.

  @param[in]  Options    Option array.
  @param[in]  ShortName  Short name to find.

  @return Option descriptor or NULL.
**/
STATIC
CONST SWITCH_OPTION *
FindOptionByShortName (
  IN  CONST SWITCH_OPTION  *Options,
  IN  CHAR8                ShortName
  )
{
  UINT32  i;

  for (i = 0; Options[i].LongName != NULL || Options[i].ShortName != 0; i++) {
    if (Options[i].ShortName == ShortName) {
      return &Options[i];
    }
  }

  return NULL;
}

/**
  Validate option value.

  @param[in]  Option  Option descriptor.
  @param[in]  Value   Value to validate.

  @retval TRUE   Valid.
  @retval FALSE  Invalid.
**/
STATIC
BOOLEAN
ValidateValue (
  IN  CONST SWITCH_OPTION  *Option,
  IN  CONST CHAR8          *Value
  )
{
  CHAR8  *EndPtr;
  UINT32 i;

  if (Value == NULL) {
    return Option->ArgType != SwitchArgRequired;
  }

  switch (Option->ValueType) {
    case SwitchValueString:
      return TRUE;

    case SwitchValueInt:
    case SwitchValueUInt:
      strtoll (Value, &EndPtr, 0);
      return *EndPtr == '\0';

    case SwitchValueFloat:
      strtod (Value, &EndPtr);
      return *EndPtr == '\0';

    case SwitchValueBool:
      return (strcmp (Value, "true") == 0 || strcmp (Value, "false") == 0 ||
              strcmp (Value, "yes") == 0 || strcmp (Value, "no") == 0 ||
              strcmp (Value, "1") == 0 || strcmp (Value, "0") == 0);

    case SwitchValueEnum:
      if (Option->EnumValues == NULL) {
        return TRUE;
      }
      for (i = 0; Option->EnumValues[i] != NULL; i++) {
        if (strcmp (Value, Option->EnumValues[i]) == 0) {
          return TRUE;
        }
      }
      return FALSE;

    default:
      return TRUE;
  }
}

/**
  Store option result.

  @param[in]  Context  Parser context.
  @param[in]  Option   Option descriptor.
  @param[in]  Value    Option value.
**/
STATIC
VOID
StoreResult (
  IN  SWITCH_CONTEXT       *Context,
  IN  CONST SWITCH_OPTION  *Option,
  IN  CONST CHAR8          *Value
  )
{
  UINT32  i;

  //
  // Check if option already exists
  //
  for (i = 0; i < Context->ResultCount; i++) {
    if (Context->Results[i].Option == Option) {
      Context->Results[i].Count++;
      if (Value != NULL) {
        Context->Results[i].Value = Value;
      }
      return;
    }
  }

  //
  // Add new result
  //
  if (Context->ResultCount < MAX_OPTIONS) {
    Context->Results[Context->ResultCount].Option = Option;
    Context->Results[Context->ResultCount].Value = Value;
    Context->Results[Context->ResultCount].Present = TRUE;
    Context->Results[Context->ResultCount].Count = 1;
    Context->ResultCount++;
  }
}

/**
  Parse Unix-style argument.

  @param[in]  Context  Parser context.
  @param[in]  Arg      Argument string.
  @param[in]  argv     Remaining arguments.
  @param[in]  Index    Current index.
  @param[in]  argc     Total argument count.

  @return Number of arguments consumed.
**/
STATIC
INT32
ParseUnixArg (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *Arg,
  IN  CHAR8           **argv,
  IN  INT32           Index,
  IN  INT32           argc
  )
{
  CONST SWITCH_OPTION  *Option;
  CONST CHAR8          *Value;
  CHAR8                *Equals;

  if (strncmp (Arg, "--", 2) == 0) {
    //
    // Long option: --option or --option=value
    //
    Arg += 2;
    Equals = strchr (Arg, '=');
    if (Equals != NULL) {
      *Equals = '\0';
      Value = Equals + 1;
    } else {
      Value = NULL;
    }

    Option = FindOptionByLongName (Context->Options, Arg);
    if (Option == NULL) {
      fprintf (stderr, "%s: unknown option --%s\n", Context->ProgramName, Arg);
      return -1;
    }

    if (Value == NULL && Option->ArgType == SwitchArgRequired) {
      if (Index + 1 < argc) {
        Value = argv[Index + 1];
        if (!ValidateValue (Option, Value)) {
          fprintf (stderr, "%s: invalid value for --%s: %s\n", Context->ProgramName, Arg, Value);
          return -1;
        }
        StoreResult (Context, Option, Value);
        return 2;
      } else {
        fprintf (stderr, "%s: option --%s requires an argument\n", Context->ProgramName, Arg);
        return -1;
      }
    }

    if (!ValidateValue (Option, Value)) {
      fprintf (stderr, "%s: invalid value for --%s: %s\n", Context->ProgramName, Arg, Value ? Value : "(none)");
      return -1;
    }

    StoreResult (Context, Option, Value);
    return 1;
  } else if (Arg[0] == '-' && Arg[1] != '\0') {
    //
    // Short option: -x or -xyz (combined)
    //
    Arg++;
    while (*Arg != '\0') {
      Option = FindOptionByShortName (Context->Options, *Arg);
      if (Option == NULL) {
        fprintf (stderr, "%s: unknown option -%c\n", Context->ProgramName, *Arg);
        return -1;
      }

      if (Option->ArgType == SwitchArgRequired) {
        if (*(Arg + 1) != '\0') {
          //
          // Argument attached: -xVALUE
          //
          Value = Arg + 1;
        } else if (Index + 1 < argc) {
          //
          // Argument in next argv
          //
          Value = argv[Index + 1];
          if (!ValidateValue (Option, Value)) {
            fprintf (stderr, "%s: invalid value for -%c: %s\n", Context->ProgramName, *Arg, Value);
            return -1;
          }
          StoreResult (Context, Option, Value);
          return 2;
        } else {
          fprintf (stderr, "%s: option -%c requires an argument\n", Context->ProgramName, *Arg);
          return -1;
        }

        if (!ValidateValue (Option, Value)) {
          fprintf (stderr, "%s: invalid value for -%c: %s\n", Context->ProgramName, *Arg, Value);
          return -1;
        }
        StoreResult (Context, Option, Value);
        return 1;
      }

      StoreResult (Context, Option, NULL);
      Arg++;
    }
    return 1;
  }

  return 0;
}

/**
  Parse VMS-style argument.

  @param[in]  Context  Parser context.
  @param[in]  Arg      Argument string.
  @param[in]  argv     Remaining arguments.
  @param[in]  Index    Current index.
  @param[in]  argc     Total argument count.

  @return Number of arguments consumed.
**/
STATIC
INT32
ParseVMSArg (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *Arg,
  IN  CHAR8           **argv,
  IN  INT32           Index,
  IN  INT32           argc
  )
{
  CONST SWITCH_OPTION  *Option;
  CONST CHAR8          *Value;
  CHAR8                *Separator;
  CHAR8                NameBuf[256];

  if (Arg[0] != '/') {
    return 0;
  }

  //
  // VMS style: /OPTION, /OPTION=value, /OPTION:value
  //
  Arg++;
  Separator = strchr (Arg, '=');
  if (Separator == NULL) {
    Separator = strchr (Arg, ':');
  }

  if (Separator != NULL) {
    //
    // Has value: /OPTION=value or /OPTION:value
    //
    size_t NameLen = Separator - Arg;
    if (NameLen >= sizeof (NameBuf)) {
      NameLen = sizeof (NameBuf) - 1;
    }
    strncpy (NameBuf, Arg, NameLen);
    NameBuf[NameLen] = '\0';
    Value = Separator + 1;
  } else {
    //
    // No value: /OPTION
    //
    strncpy (NameBuf, Arg, sizeof (NameBuf) - 1);
    NameBuf[sizeof (NameBuf) - 1] = '\0';
    Value = NULL;
  }

  //
  // Convert to uppercase for VMS convention
  //
  for (CHAR8 *p = NameBuf; *p != '\0'; p++) {
    *p = toupper (*p);
  }

  Option = FindOptionByLongName (Context->Options, NameBuf);
  if (Option == NULL) {
    fprintf (stderr, "%s: unknown option /%s\n", Context->ProgramName, NameBuf);
    return -1;
  }

  if (Value == NULL && Option->ArgType == SwitchArgRequired) {
    fprintf (stderr, "%s: option /%s requires an argument\n", Context->ProgramName, NameBuf);
    return -1;
  }

  if (!ValidateValue (Option, Value)) {
    fprintf (stderr, "%s: invalid value for /%s: %s\n", Context->ProgramName, NameBuf, Value ? Value : "(none)");
    return -1;
  }

  StoreResult (Context, Option, Value);
  return 1;
}

/**
  Parse DOS/Windows-style argument.

  @param[in]  Context  Parser context.
  @param[in]  Arg      Argument string.
  @param[in]  argv     Remaining arguments.
  @param[in]  Index    Current index.
  @param[in]  argc     Total argument count.

  @return Number of arguments consumed.
**/
STATIC
INT32
ParseDOSArg (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *Arg,
  IN  CHAR8           **argv,
  IN  INT32           Index,
  IN  INT32           argc
  )
{
  CONST SWITCH_OPTION  *Option;
  CONST CHAR8          *Value;
  CHAR8                *Separator;

  if (Arg[0] != '/') {
    return 0;
  }

  //
  // DOS/Windows style: /X, /OPTION, /OPTION:value
  //
  Arg++;
  Separator = strchr (Arg, ':');

  if (Separator != NULL) {
    //
    // Has value: /OPTION:value
    //
    *Separator = '\0';
    Value = Separator + 1;
  } else {
    Value = NULL;
  }

  //
  // Try to find option by long name first
  //
  Option = FindOptionByLongName (Context->Options, Arg);
  if (Option == NULL && strlen (Arg) == 1) {
    //
    // Try short name: /X
    //
    Option = FindOptionByShortName (Context->Options, Arg[0]);
  }

  if (Option == NULL) {
    fprintf (stderr, "%s: unknown option /%s\n", Context->ProgramName, Arg);
    return -1;
  }

  if (Value == NULL && Option->ArgType == SwitchArgRequired) {
    if (Index + 1 < argc) {
      Value = argv[Index + 1];
      if (!ValidateValue (Option, Value)) {
        fprintf (stderr, "%s: invalid value for /%s: %s\n", Context->ProgramName, Arg, Value);
        return -1;
      }
      StoreResult (Context, Option, Value);
      return 2;
    } else {
      fprintf (stderr, "%s: option /%s requires an argument\n", Context->ProgramName, Arg);
      return -1;
    }
  }

  if (!ValidateValue (Option, Value)) {
    fprintf (stderr, "%s: invalid value for /%s: %s\n", Context->ProgramName, Arg, Value ? Value : "(none)");
    return -1;
  }

  StoreResult (Context, Option, Value);
  return 1;
}

/**
  Create option parser context.

  @param[in]  ProgramName   Program name for help text.
  @param[in]  Description   Program description.
  @param[in]  Options       Array of option descriptors (NULL-terminated).
  @param[in]  Style         Option style (SwitchStyleAuto for auto-detect).

  @return Parser context, or NULL on error.
**/
SWITCH_CONTEXT *
SwitchCreate (
  IN  CONST CHAR8          *ProgramName,
  IN  CONST CHAR8          *Description,
  IN  CONST SWITCH_OPTION  *Options,
  IN  SWITCH_STYLE         Style
  )
{
  SWITCH_CONTEXT  *Context;

  Context = calloc (1, sizeof (SWITCH_CONTEXT));
  if (Context == NULL) {
    return NULL;
  }

  Context->ProgramName = ProgramName;
  Context->Description = Description;
  Context->Options = Options;
  Context->Style = DetermineStyle (Style);
  Context->ResultCount = 0;
  Context->ArgumentCount = 0;

  return Context;
}

/**
  Parse command-line arguments.

  @param[in]  Context   Parser context.
  @param[in]  argc      Argument count.
  @param[in]  argv      Argument array.

  @retval TRUE   Parsing succeeded.
  @retval FALSE  Parsing failed (error printed).
**/
BOOLEAN
SwitchParse (
  IN  SWITCH_CONTEXT  *Context,
  IN  INT32           argc,
  IN  CHAR8           **argv
  )
{
  INT32  i;
  INT32  Consumed;

  if (Context == NULL || argv == NULL) {
    return FALSE;
  }

  for (i = 1; i < argc; i++) {
    Consumed = 0;

    //
    // Try to parse based on style
    //
    switch (Context->Style) {
      case SwitchStyleUnix:
      case SwitchStyleGNU:
        if (argv[i][0] == '-') {
          Consumed = ParseUnixArg (Context, argv[i], argv, i, argc);
        }
        break;

      case SwitchStyleVMS:
        if (argv[i][0] == '/') {
          Consumed = ParseVMSArg (Context, argv[i], argv, i, argc);
        }
        break;

      case SwitchStyleDOS:
        if (argv[i][0] == '/') {
          Consumed = ParseDOSArg (Context, argv[i], argv, i, argc);
        }
        break;

      default:
        break;
    }

    if (Consumed < 0) {
      return FALSE;
    } else if (Consumed > 0) {
      i += Consumed - 1;
    } else {
      //
      // Positional argument
      //
      if (Context->ArgumentCount < MAX_ARGS) {
        Context->Arguments[Context->ArgumentCount++] = argv[i];
      }
    }
  }

  return TRUE;
}

/**
  Get option result by long name.

  @param[in]  Context   Parser context.
  @param[in]  LongName  Long option name.

  @return Option result, or NULL if not found.
**/
CONST SWITCH_RESULT *
SwitchGetOption (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *LongName
  )
{
  UINT32  i;

  if (Context == NULL || LongName == NULL) {
    return NULL;
  }

  for (i = 0; i < Context->ResultCount; i++) {
    if (Context->Results[i].Option->LongName != NULL &&
        strcmp (Context->Results[i].Option->LongName, LongName) == 0) {
      return &Context->Results[i];
    }
  }

  return NULL;
}

/**
  Get option result by short name.

  @param[in]  Context    Parser context.
  @param[in]  ShortName  Short option character.

  @return Option result, or NULL if not found.
**/
CONST SWITCH_RESULT *
SwitchGetOptionByChar (
  IN  SWITCH_CONTEXT  *Context,
  IN  CHAR8           ShortName
  )
{
  UINT32  i;

  if (Context == NULL) {
    return NULL;
  }

  for (i = 0; i < Context->ResultCount; i++) {
    if (Context->Results[i].Option->ShortName == ShortName) {
      return &Context->Results[i];
    }
  }

  return NULL;
}

/**
  Get non-option arguments (positional arguments).

  @param[in]   Context   Parser context.
  @param[out]  Count     Pointer to receive argument count.

  @return Array of positional arguments.
**/
CHAR8 **
SwitchGetArguments (
  IN  SWITCH_CONTEXT  *Context,
  OUT UINT32          *Count
  )
{
  if (Context == NULL || Count == NULL) {
    return NULL;
  }

  *Count = Context->ArgumentCount;
  return Context->Arguments;
}

/**
  Check if option is present.

  @param[in]  Context   Parser context.
  @param[in]  LongName  Long option name.

  @retval TRUE   Option was specified.
  @retval FALSE  Option was not specified.
**/
BOOLEAN
SwitchIsPresent (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *LongName
  )
{
  CONST SWITCH_RESULT  *Result;

  Result = SwitchGetOption (Context, LongName);
  return (Result != NULL && Result->Present);
}

/**
  Get option value as string.

  @param[in]  Context      Parser context.
  @param[in]  LongName     Long option name.
  @param[in]  DefaultVal   Default value if not specified.

  @return Option value or default.
**/
CONST CHAR8 *
SwitchGetString (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *LongName,
  IN  CONST CHAR8     *DefaultVal
  )
{
  CONST SWITCH_RESULT  *Result;

  Result = SwitchGetOption (Context, LongName);
  if (Result != NULL && Result->Value != NULL) {
    return Result->Value;
  }

  return DefaultVal;
}

/**
  Get option value as integer.

  @param[in]  Context      Parser context.
  @param[in]  LongName     Long option name.
  @param[in]  DefaultVal   Default value if not specified.

  @return Option value or default.
**/
INT64
SwitchGetInt (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *LongName,
  IN  INT64           DefaultVal
  )
{
  CONST SWITCH_RESULT  *Result;

  Result = SwitchGetOption (Context, LongName);
  if (Result != NULL && Result->Value != NULL) {
    return strtoll (Result->Value, NULL, 0);
  }

  return DefaultVal;
}

/**
  Get option value as boolean.

  @param[in]  Context      Parser context.
  @param[in]  LongName     Long option name.
  @param[in]  DefaultVal   Default value if not specified.

  @retval TRUE/FALSE based on option value.
**/
BOOLEAN
SwitchGetBool (
  IN  SWITCH_CONTEXT  *Context,
  IN  CONST CHAR8     *LongName,
  IN  BOOLEAN         DefaultVal
  )
{
  CONST SWITCH_RESULT  *Result;

  Result = SwitchGetOption (Context, LongName);
  if (Result != NULL) {
    if (Result->Value == NULL) {
      return TRUE;  // Boolean flag present
    }
    if (strcmp (Result->Value, "true") == 0 || strcmp (Result->Value, "yes") == 0 || strcmp (Result->Value, "1") == 0) {
      return TRUE;
    }
    if (strcmp (Result->Value, "false") == 0 || strcmp (Result->Value, "no") == 0 || strcmp (Result->Value, "0") == 0) {
      return FALSE;
    }
  }

  return DefaultVal;
}

/**
  Print help message.

  @param[in]  Context      Parser context.
  @param[in]  ColorOutput  Color output mode.
**/
VOID
SwitchPrintHelp (
  IN  SWITCH_CONTEXT  *Context,
  IN  COLOR_OUTPUT    ColorOutput
  )
{
  UINT32       i;
  CONST CHAR8  *Bold;
  CONST CHAR8  *Cyan;
  CONST CHAR8  *Reset;

  if (Context == NULL) {
    return;
  }

  Bold = GetColor (COLOR_BOLD, ColorOutput);
  Cyan = GetColor (COLOR_CYAN, ColorOutput);
  Reset = GetColor (COLOR_RESET, ColorOutput);

  printf ("%sUsage:%s %s [options]\n", Bold, Reset, Context->ProgramName);
  if (Context->Description != NULL) {
    printf ("\n%s\n", Context->Description);
  }

  printf ("\n%sOptions:%s\n", Bold, Reset);

  for (i = 0; Context->Options[i].LongName != NULL || Context->Options[i].ShortName != 0; i++) {
    if (Context->Options[i].Flags & SWITCH_FLAG_HIDDEN) {
      continue;
    }

    printf ("  ");

    if (Context->Options[i].ShortName != 0) {
      printf ("%s-%c%s", Cyan, Context->Options[i].ShortName, Reset);
      if (Context->Options[i].LongName != NULL) {
        printf (", ");
      }
    } else {
      printf ("    ");
    }

    if (Context->Options[i].LongName != NULL) {
      printf ("%s--%s%s", Cyan, Context->Options[i].LongName, Reset);
    }

    if (Context->Options[i].ArgType == SwitchArgRequired && Context->Options[i].ArgName != NULL) {
      printf (" <%s>", Context->Options[i].ArgName);
    } else if (Context->Options[i].ArgType == SwitchArgOptional && Context->Options[i].ArgName != NULL) {
      printf (" [%s]", Context->Options[i].ArgName);
    }

    if (Context->Options[i].Description != NULL) {
      printf ("\n      %s", Context->Options[i].Description);
    }

    if (Context->Options[i].DefaultValue != NULL) {
      printf (" (default: %s)", Context->Options[i].DefaultValue);
    }

    printf ("\n");
  }
}

/**
  Print version information.

  @param[in]  ProgramName  Program name.
  @param[in]  Version      Version string.
  @param[in]  Copyright    Copyright string.
**/
VOID
SwitchPrintVersion (
  IN  CONST CHAR8  *ProgramName,
  IN  CONST CHAR8  *Version,
  IN  CONST CHAR8  *Copyright
  )
{
  printf ("%s version %s\n", ProgramName, Version);
  if (Copyright != NULL) {
    printf ("%s\n", Copyright);
  }
}

/**
  Free parser context.

  @param[in]  Context  Parser context to free.
**/
VOID
SwitchFree (
  IN  SWITCH_CONTEXT  *Context
  )
{
  if (Context != NULL) {
    free (Context);
  }
}
