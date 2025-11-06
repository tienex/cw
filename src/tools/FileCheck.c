/**
  @file FileCheck.c

  MMIX FileCheck - Pattern matching tool for test verification.
  Similar to LLVM's FileCheck utility.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

#define MAX_LINE_LENGTH 4096
#define MAX_CHECKS 1024

typedef struct {
  char *Pattern;
  bool IsLabel;
  bool IsRegex;
  int LineNumber;
} CHECK_PATTERN;

typedef struct {
  CHECK_PATTERN Checks[MAX_CHECKS];
  int CheckCount;
  char *InputFile;
  bool Verbose;
  bool StrictWhitespace;
} FILECHECK_CONTEXT;

/**
  Parse CHECK comments from input file.

  @param[in,out]  Context       FileCheck context.
  @param[in]      CheckFile     File containing CHECK directives.

  @return  Number of checks parsed.

**/
int
ParseCheckFile (
  FILECHECK_CONTEXT *Context,
  const char        *CheckFile
  )
{
  FILE *File;
  char Line[MAX_LINE_LENGTH];
  int LineNum = 0;

  File = fopen (CheckFile, "r");
  if (File == NULL) {
    fprintf (stderr, "Error: Cannot open check file: %s\n", CheckFile);
    return -1;
  }

  Context->CheckCount = 0;

  while (fgets (Line, sizeof (Line), File) != NULL) {
    LineNum++;
    char *CheckPos = strstr (Line, "// CHECK");

    if (CheckPos != NULL) {
      // Found a CHECK directive
      char *ColonPos = strchr (CheckPos, ':');
      if (ColonPos == NULL) continue;

      // Check if it's CHECK-LABEL
      bool IsLabel = (strstr (CheckPos, "CHECK-LABEL") != NULL);

      // Extract pattern after colon
      char *Pattern = ColonPos + 1;
      while (*Pattern == ' ' || *Pattern == '\t') Pattern++;

      // Remove trailing newline
      size_t Len = strlen (Pattern);
      while (Len > 0 && (Pattern[Len-1] == '\n' || Pattern[Len-1] == '\r')) {
        Pattern[--Len] = '\0';
      }

      if (Len > 0) {
        CHECK_PATTERN *Check = &Context->Checks[Context->CheckCount];
        Check->Pattern = strdup (Pattern);
        Check->IsLabel = IsLabel;
        Check->IsRegex = (strstr (Pattern, "{{.*}}") != NULL);
        Check->LineNumber = LineNum;
        Context->CheckCount++;

        if (Context->CheckCount >= MAX_CHECKS) {
          fprintf (stderr, "Warning: Maximum number of checks reached\n");
          break;
        }
      }
    }
  }

  fclose (File);
  return Context->CheckCount;
}

/**
  Check if line matches pattern.

  @param[in]      Line          Line to check.
  @param[in]      Pattern       Pattern to match.
  @param[in]      IsRegex       Whether pattern uses regex syntax.

  @return  TRUE if matches, FALSE otherwise.

**/
bool
MatchPattern (
  const char  *Line,
  const char  *Pattern,
  bool        IsRegex
  )
{
  if (IsRegex) {
    // Convert {{.*}} style patterns to regex
    char RegexPattern[MAX_LINE_LENGTH];
    char *Dst = RegexPattern;
    const char *Src = Pattern;

    while (*Src) {
      if (Src[0] == '{' && Src[1] == '{') {
        // Start of regex pattern
        Src += 2;
        while (*Src && !(Src[0] == '}' && Src[1] == '}')) {
          *Dst++ = *Src++;
        }
        if (*Src) Src += 2; // Skip }}
      } else {
        // Escape special regex chars
        if (strchr (".*+?[](){}|^$\\", *Src)) {
          *Dst++ = '\\';
        }
        *Dst++ = *Src++;
      }
    }
    *Dst = '\0';

    regex_t Regex;
    int Result = regcomp (&Regex, RegexPattern, REG_EXTENDED | REG_NOSUB);
    if (Result != 0) {
      fprintf (stderr, "Error: Invalid regex pattern: %s\n", RegexPattern);
      return false;
    }

    Result = regexec (&Regex, Line, 0, NULL, 0);
    regfree (&Regex);
    return (Result == 0);
  } else {
    // Simple substring match
    return (strstr (Line, Pattern) != NULL);
  }
}

/**
  Run FileCheck verification.

  @param[in,out]  Context       FileCheck context.

  @return  0 on success, non-zero on failure.

**/
int
RunFileCheck (
  FILECHECK_CONTEXT *Context
  )
{
  FILE *Input;
  char Line[MAX_LINE_LENGTH];
  int CurrentCheck = 0;
  int LineNum = 0;
  int LastMatchLine = 0;

  if (Context->InputFile == NULL) {
    Input = stdin;
  } else {
    Input = fopen (Context->InputFile, "r");
    if (Input == NULL) {
      fprintf (stderr, "Error: Cannot open input file: %s\n", Context->InputFile);
      return 1;
    }
  }

  while (fgets (Line, sizeof (Line), Input) != NULL && CurrentCheck < Context->CheckCount) {
    LineNum++;

    // Remove trailing newline
    size_t Len = strlen (Line);
    while (Len > 0 && (Line[Len-1] == '\n' || Line[Len-1] == '\r')) {
      Line[--Len] = '\0';
    }

    CHECK_PATTERN *Check = &Context->Checks[CurrentCheck];

    if (MatchPattern (Line, Check->Pattern, Check->IsRegex)) {
      if (Context->Verbose) {
        printf ("CHECK: %s (line %d) matched at input line %d: %s\n",
                Check->Pattern, Check->LineNumber, LineNum, Line);
      }
      LastMatchLine = LineNum;
      CurrentCheck++;

      // If this was a label check, continue immediately to next check
      if (Check->IsLabel) {
        continue;
      }
    }
  }

  if (Context->InputFile != NULL) {
    fclose (Input);
  }

  // Verify all checks passed
  if (CurrentCheck < Context->CheckCount) {
    fprintf (stderr, "Error: Not all checks passed\n");
    fprintf (stderr, "Failed at CHECK on line %d: %s\n",
             Context->Checks[CurrentCheck].LineNumber,
             Context->Checks[CurrentCheck].Pattern);
    return 1;
  }

  if (Context->Verbose) {
    printf ("All %d checks passed\n", Context->CheckCount);
  }

  return 0;
}

/**
  Print usage information.

  @param[in]      ProgramName   Name of the program.

**/
void
PrintUsage (
  const char  *ProgramName
  )
{
  printf ("Usage: %s [options] <check-file>\n\n", ProgramName);
  printf ("Options:\n");
  printf ("  -input <file>      Input file to check (default: stdin)\n");
  printf ("  -v, --verbose      Enable verbose output\n");
  printf ("  -h, --help         Print this help message\n");
  printf ("\n");
  printf ("FileCheck reads CHECK directives from <check-file> and verifies\n");
  printf ("that they appear in the input in order.\n");
  printf ("\n");
  printf ("CHECK directives:\n");
  printf ("  // CHECK: pattern           - Match pattern anywhere\n");
  printf ("  // CHECK-LABEL: pattern     - Match label (resets search position)\n");
  printf ("  // CHECK: text {{.*}} more  - Regex match using {{...}} syntax\n");
}

/**
  Main entry point.

  @param[in]      argc          Argument count.
  @param[in]      argv          Argument array.

  @return  0 on success, non-zero on error.

**/
int
main (
  int   argc,
  char  **argv
  )
{
  FILECHECK_CONTEXT Context = {0};
  const char *CheckFile = NULL;

  // Parse command line arguments
  for (int i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
      PrintUsage (argv[0]);
      return 0;
    } else if (strcmp (argv[i], "-v") == 0 || strcmp (argv[i], "--verbose") == 0) {
      Context.Verbose = true;
    } else if (strcmp (argv[i], "-input") == 0) {
      if (i + 1 < argc) {
        Context.InputFile = argv[++i];
      } else {
        fprintf (stderr, "Error: -input requires an argument\n");
        return 1;
      }
    } else if (argv[i][0] == '-') {
      fprintf (stderr, "Error: Unknown option: %s\n", argv[i]);
      PrintUsage (argv[0]);
      return 1;
    } else {
      CheckFile = argv[i];
    }
  }

  if (CheckFile == NULL) {
    fprintf (stderr, "Error: No check file specified\n");
    PrintUsage (argv[0]);
    return 1;
  }

  // Parse CHECK directives from check file
  if (ParseCheckFile (&Context, CheckFile) < 0) {
    return 1;
  }

  if (Context.CheckCount == 0) {
    fprintf (stderr, "Warning: No CHECK directives found in %s\n", CheckFile);
    return 0;
  }

  // Run verification
  int Result = RunFileCheck (&Context);

  // Cleanup
  for (int i = 0; i < Context.CheckCount; i++) {
    free (Context.Checks[i].Pattern);
  }

  return Result;
}
