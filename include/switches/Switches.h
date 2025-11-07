/** @file
  Cross-platform Command-Line Option Parser Library.

  This library provides unified command-line parsing across different
  operating systems and conventions:
  - Unix/Linux: -x, --option=value
  - VMS: /OPTION=value, /OPTION:value
  - DOS/Windows: /X, /OPTION:value

  Features:
  - Automatic platform detection
  - Integrated help generation
  - Color output support
  - Short and long option names
  - Required/optional arguments
  - Type validation
  - Default values

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifndef __SWITCHES_H__
#define __SWITCHES_H__

#include "../MmixTypes.h"

///
/// Option style detection
///
typedef enum {
  SwitchStyleAuto = 0,    ///< Auto-detect based on platform
  SwitchStyleUnix,        ///< Unix: -x, --option
  SwitchStyleVMS,         ///< VMS: /OPTION
  SwitchStyleDOS,         ///< DOS/Windows: /X
  SwitchStyleGNU          ///< GNU: --option only (no single dash)
} SWITCH_STYLE;

///
/// Option argument type
///
typedef enum {
  SwitchArgNone = 0,      ///< No argument
  SwitchArgRequired,      ///< Required argument
  SwitchArgOptional       ///< Optional argument
} SWITCH_ARG_TYPE;

///
/// Option value type for validation
///
typedef enum {
  SwitchValueString = 0,  ///< String value (no validation)
  SwitchValueInt,         ///< Integer value
  SwitchValueUInt,        ///< Unsigned integer
  SwitchValueFloat,       ///< Floating point
  SwitchValueBool,        ///< Boolean (true/false, yes/no, 1/0)
  SwitchValueEnum         ///< Enumerated value (list of valid strings)
} SWITCH_VALUE_TYPE;

///
/// Option flags
///
#define SWITCH_FLAG_NONE        0x00
#define SWITCH_FLAG_HIDDEN      0x01  ///< Don't show in help
#define SWITCH_FLAG_DEPRECATED  0x02  ///< Show deprecation warning
#define SWITCH_FLAG_REQUIRED    0x04  ///< Option must be specified
#define SWITCH_FLAG_MULTIPLE    0x08  ///< Can be specified multiple times

///
/// Color output options
///
typedef enum {
  ColorOutputAuto = 0,    ///< Auto-detect TTY
  ColorOutputNever,       ///< Never use colors
  ColorOutputAlways       ///< Always use colors
} COLOR_OUTPUT;

///
/// Option descriptor
///
typedef struct {
  CHAR8              ShortName;        ///< Short option (e.g., 'v' for -v)
  CONST CHAR8        *LongName;        ///< Long option (e.g., "verbose")
  SWITCH_ARG_TYPE    ArgType;          ///< Argument requirement
  SWITCH_VALUE_TYPE  ValueType;        ///< Value type for validation
  CONST CHAR8        *DefaultValue;    ///< Default value (NULL if none)
  CONST CHAR8        *Description;     ///< Help description
  CONST CHAR8        *ArgName;         ///< Argument name in help (e.g., "FILE")
  CONST CHAR8        **EnumValues;     ///< Valid values for enum type (NULL-terminated)
  UINT32             Flags;            ///< Option flags
  VOID               *UserData;        ///< User-defined data
} SWITCH_OPTION;

///
/// Parsed option result
///
typedef struct {
  CONST SWITCH_OPTION  *Option;        ///< Matching option descriptor
  CONST CHAR8          *Value;         ///< Parsed value (NULL if no arg)
  BOOLEAN              Present;        ///< TRUE if option was specified
  UINT32               Count;          ///< Number of times specified
} SWITCH_RESULT;

///
/// Parser context (opaque)
///
typedef struct _SWITCH_CONTEXT SWITCH_CONTEXT;

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Print help message.

  @param[in]  Context      Parser context.
  @param[in]  ColorOutput  Color output mode.
**/
VOID
SwitchPrintHelp (
  IN  SWITCH_CONTEXT  *Context,
  IN  COLOR_OUTPUT    ColorOutput
  );

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
  );

/**
  Free parser context.

  @param[in]  Context  Parser context to free.
**/
VOID
SwitchFree (
  IN  SWITCH_CONTEXT  *Context
  );

///
/// Helper macros for defining options
///

#define SWITCH_OPTION_BOOL(short, long, desc) \
  { short, long, SwitchArgNone, SwitchValueBool, NULL, desc, NULL, NULL, SWITCH_FLAG_NONE, NULL }

#define SWITCH_OPTION_STRING(short, long, argname, desc) \
  { short, long, SwitchArgRequired, SwitchValueString, NULL, desc, argname, NULL, SWITCH_FLAG_NONE, NULL }

#define SWITCH_OPTION_INT(short, long, argname, desc) \
  { short, long, SwitchArgRequired, SwitchValueInt, NULL, desc, argname, NULL, SWITCH_FLAG_NONE, NULL }

#define SWITCH_OPTION_OPTIONAL(short, long, argname, desc, defval) \
  { short, long, SwitchArgOptional, SwitchValueString, defval, desc, argname, NULL, SWITCH_FLAG_NONE, NULL }

#define SWITCH_OPTION_END() \
  { 0, NULL, SwitchArgNone, SwitchValueString, NULL, NULL, NULL, NULL, SWITCH_FLAG_NONE, NULL }

#endif // __SWITCHES_H__
