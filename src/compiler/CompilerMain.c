/**
  @file CompilerMain.c

  MMIX C23 compiler driver.
  Orchestrates the complete compilation pipeline.

  Copyright (c) 2025, Tienex. All rights reserved.

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "../../include/compiler/MmixToken.h"
#include "../../include/compiler/MmixLexer.h"
#include "../../include/compiler/MmixParser.h"
#include "../../include/compiler/MmixSema.h"
#include "../../include/compiler/MmixIr.h"
#include "../../include/compiler/MmixCodeGen.h"

/**
  Compiler options
**/
typedef struct {
  CHAR8     *InputFile;
  CHAR8     *OutputFile;
  BOOLEAN   DumpTokens;
  BOOLEAN   DumpAst;
  BOOLEAN   DumpIr;
  BOOLEAN   EmitAssembly;
  BOOLEAN   Verbose;
  BOOLEAN   EnableGnu;
  BOOLEAN   EnableMsvc;
  BOOLEAN   EnableClang;
  BOOLEAN   EnableMetaware;
} COMPILER_OPTIONS;

/**
  Print usage information.

  @param[in]      Program       Program name.

**/
STATIC
VOID
PrintUsage (
  IN  CONST CHAR8  *Program
  )
{
  printf ("MMIX C23 Compiler\n");
  printf ("Usage: %s [options] <input-file>\n\n", Program);
  printf ("Options:\n");
  printf ("  -o <file>       Write output to <file> (default: a.s)\n");
  printf ("  -S              Emit assembly (default)\n");
  printf ("  -v              Verbose output\n");
  printf ("  --dump-tokens   Dump token stream\n");
  printf ("  --dump-ast      Dump abstract syntax tree\n");
  printf ("  --dump-ir       Dump intermediate representation\n");
  printf ("  --gnu           Enable GNU extensions\n");
  printf ("  --msvc          Enable MSVC extensions\n");
  printf ("  --clang         Enable Clang extensions\n");
  printf ("  --metaware      Enable MetaWare extensions\n");
  printf ("  -h, --help      Show this help message\n");
}

/**
  Read entire file into memory.

  @param[in]      Path          File path.
  @param[out]     Size          Pointer to receive file size.

  @return  Pointer to file contents, or NULL on error.

**/
STATIC
CHAR8 *
ReadFile (
  IN  CONST CHAR8  *Path,
  OUT UINT32       *Size
  )
{
  FILE   *File;
  CHAR8  *Buffer;
  UINT32 Length;

  File = fopen (Path, "rb");
  if (File == NULL) {
    fprintf (stderr, "Error: Cannot open file '%s'\n", Path);
    return NULL;
  }

  //
  // Get file size
  //
  fseek (File, 0, SEEK_END);
  Length = ftell (File);
  fseek (File, 0, SEEK_SET);

  //
  // Allocate buffer and read
  //
  Buffer = (CHAR8 *)malloc (Length + 1);
  if (Buffer == NULL) {
    fclose (File);
    return NULL;
  }

  if (fread (Buffer, 1, Length, File) != Length) {
    fprintf (stderr, "Error: Failed to read file '%s'\n", Path);
    free (Buffer);
    fclose (File);
    return NULL;
  }

  Buffer[Length] = '\0';
  fclose (File);

  *Size = Length;
  return Buffer;
}

/**
  Dump token stream to stdout.

  @param[in]      Lexer         Lexer state.

**/
STATIC
VOID
DumpTokens (
  IN  LEXER_STATE  *Lexer
  )
{
  TOKEN  *Tok;

  printf ("\n=== Token Stream ===\n");

  while ((Tok = LexerNextToken (Lexer)) != NULL) {
    BOOLEAN  IsEof = (Tok->Type == TOK_EOF);

    printf ("%s:%u:%u: %s (type=%d)",
            Tok->Location.FileName,
            Tok->Location.Line,
            Tok->Location.Column,
            TokenTypeName (Tok->Type),
            Tok->Type);

    if (Tok->Text != NULL && Tok->TextLength > 0) {
      printf (" '%.*s'", Tok->TextLength, Tok->Text);
    }

    printf ("\n");

    TokenDestroy (Tok);

    if (IsEof) {
      break;
    }
  }
}

/**
  Main entry point.

  @param[in]      argc          Argument count.
  @param[in]      argv          Argument vector.

  @return  Exit code.

**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  COMPILER_OPTIONS      Options;
  CHAR8                 *Source;
  UINT32                SourceSize;
  LEXER_STATE           *Lexer;
  PARSER_STATE          *Parser;
  AST_TRANSLATION_UNIT  *Ast;
  SEMA_CONTEXT          *Sema;
  IR_MODULE             *IrModule;
  CODEGEN_CONTEXT       *CodeGen;
  FILE                  *OutputFile;
  INT32                 ExitCode;

  //
  // Initialize options
  //
  memset (&Options, 0, sizeof (Options));
  Options.OutputFile = "a.s";
  Options.EmitAssembly = TRUE;
  Options.EnableGnu = TRUE;  // GNU extensions enabled by default

  //
  // Parse command-line arguments
  //
  static struct option LongOptions[] = {
    { "dump-tokens", no_argument, 0, 't' },
    { "dump-ast",    no_argument, 0, 'a' },
    { "dump-ir",     no_argument, 0, 'i' },
    { "gnu",         no_argument, 0, 'g' },
    { "msvc",        no_argument, 0, 'm' },
    { "clang",       no_argument, 0, 'c' },
    { "metaware",    no_argument, 0, 'w' },
    { "help",        no_argument, 0, 'h' },
    { 0, 0, 0, 0 }
  };

  INT32  c;
  INT32  OptionIndex;

  while ((c = getopt_long (argc, argv, "o:Svh", LongOptions, &OptionIndex)) != -1) {
    switch (c) {
      case 'o':
        Options.OutputFile = optarg;
        break;
      case 'S':
        Options.EmitAssembly = TRUE;
        break;
      case 'v':
        Options.Verbose = TRUE;
        break;
      case 't':
        Options.DumpTokens = TRUE;
        break;
      case 'a':
        Options.DumpAst = TRUE;
        break;
      case 'i':
        Options.DumpIr = TRUE;
        break;
      case 'g':
        Options.EnableGnu = TRUE;
        break;
      case 'm':
        Options.EnableMsvc = TRUE;
        break;
      case 'c':
        Options.EnableClang = TRUE;
        break;
      case 'w':
        Options.EnableMetaware = TRUE;
        break;
      case 'h':
        PrintUsage (argv[0]);
        return 0;
      default:
        PrintUsage (argv[0]);
        return 1;
    }
  }

  //
  // Check for input file
  //
  if (optind >= argc) {
    fprintf (stderr, "Error: No input file specified\n");
    PrintUsage (argv[0]);
    return 1;
  }

  Options.InputFile = argv[optind];

  if (Options.Verbose) {
    printf ("MMIX C23 Compiler\n");
    printf ("Input:  %s\n", Options.InputFile);
    printf ("Output: %s\n", Options.OutputFile);
    printf ("\n");
  }

  //
  // Read source file
  //
  Source = ReadFile (Options.InputFile, &SourceSize);
  if (Source == NULL) {
    return 1;
  }

  ExitCode = 0;

  //
  // Lexical analysis
  //
  if (Options.Verbose) {
    printf ("=== Lexical Analysis ===\n");
  }

  LEXER_OPTIONS  LexerOpts;
  memset (&LexerOpts, 0, sizeof (LexerOpts));
  LexerOpts.EnableGnuExtensions = Options.EnableGnu;
  LexerOpts.EnableMsvcExtensions = Options.EnableMsvc;
  LexerOpts.EnableClangExtensions = Options.EnableClang;
  LexerOpts.EnableMetawareExtensions = Options.EnableMetaware;
  LexerOpts.TabSize = 4;

  Lexer = LexerCreate (Source, SourceSize, Options.InputFile, &LexerOpts);
  if (Lexer == NULL) {
    fprintf (stderr, "Error: Failed to create lexer\n");
    free (Source);
    return 1;
  }

  if (Options.DumpTokens) {
    DumpTokens (Lexer);
    LexerDestroy (Lexer);
    free (Source);
    return 0;
  }

  //
  // Parsing
  //
  if (Options.Verbose) {
    printf ("=== Parsing ===\n");
  }

  Parser = ParserCreate (Lexer);
  if (Parser == NULL) {
    fprintf (stderr, "Error: Failed to create parser\n");
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  Ast = ParserParseTranslationUnit (Parser);

  if (Ast == NULL || Parser->ErrorCount > 0) {
    fprintf (stderr, "Error: Parsing failed with %u errors\n", Parser->ErrorCount);
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  if (Options.Verbose) {
    printf ("Parsed %u declarations\n", Ast->DeclarationCount);
  }

  if (Options.DumpAst) {
    printf ("\n=== Abstract Syntax Tree ===\n");
    printf ("Translation unit with %u declarations\n", Ast->DeclarationCount);
    // TODO: Implement AST dumping
  }

  //
  // Semantic analysis
  //
  if (Options.Verbose) {
    printf ("\n=== Semantic Analysis ===\n");
  }

  Sema = SemaCreate (Ast);

  if (Sema == NULL) {
    fprintf (stderr, "Error: Failed to create semantic analyzer\n");
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  if (SemaAnalyze (Sema) != MMIX_SUCCESS || Sema->ErrorCount > 0) {
    fprintf (stderr, "Error: Semantic analysis failed with %u errors\n", Sema->ErrorCount);
    SemaDestroy (Sema);
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  if (Options.Verbose) {
    printf ("Semantic analysis complete\n");
  }

  //
  // IR generation
  //
  if (Options.Verbose) {
    printf ("\n=== IR Generation ===\n");
  }

  IrModule = IrCreateModule (Ast);

  if (IrModule == NULL) {
    fprintf (stderr, "Error: Failed to create IR module\n");
    SemaDestroy (Sema);
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  if (IrGenerateModule (IrModule) != MMIX_SUCCESS) {
    fprintf (stderr, "Error: IR generation failed\n");
    IrDestroyModule (IrModule);
    SemaDestroy (Sema);
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  if (Options.Verbose) {
    printf ("Generated IR for %u functions\n", IrModule->FunctionCount);
  }

  if (Options.DumpIr) {
    IrPrintModule (IrModule, stdout);
  }

  //
  // Code generation
  //
  if (Options.Verbose) {
    printf ("\n=== Code Generation ===\n");
  }

  OutputFile = fopen (Options.OutputFile, "w");
  if (OutputFile == NULL) {
    fprintf (stderr, "Error: Cannot open output file '%s'\n", Options.OutputFile);
    IrDestroyModule (IrModule);
    SemaDestroy (Sema);
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  CodeGen = CodeGenCreate (IrModule, OutputFile);

  if (CodeGen == NULL) {
    fprintf (stderr, "Error: Failed to create code generator\n");
    fclose (OutputFile);
    IrDestroyModule (IrModule);
    SemaDestroy (Sema);
    ParserDestroy (Parser);
    LexerDestroy (Lexer);
    free (Source);
    return 1;
  }

  if (CodeGenModule (CodeGen) != MMIX_SUCCESS || CodeGen->ErrorCount > 0) {
    fprintf (stderr, "Error: Code generation failed with %u errors\n", CodeGen->ErrorCount);
    ExitCode = 1;
  } else {
    if (Options.Verbose) {
      printf ("Assembly written to %s\n", Options.OutputFile);
    }
  }

  //
  // Cleanup
  //
  if (Options.Verbose) {
    printf ("Cleaning up...\n");
  }

  CodeGenDestroy (CodeGen);

  fclose (OutputFile);

  // IrDestroyModule (IrModule);  // TODO: Fix double free

  // SemaDestroy (Sema);

  // ParserDestroy (Parser);

  LexerDestroy (Lexer);

  free (Source);

  // Note: Skipping cleanup to avoid double free
  (void)IrModule;
  (void)Sema;
  (void)Parser;

  if (Options.Verbose && ExitCode == 0) {
    printf ("\n=== Compilation Successful ===\n");
  }

  return ExitCode;
}
