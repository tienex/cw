/** @file
  lipo - Create and manipulate universal/fat binaries.

  This tool creates and manipulates universal (fat) binaries containing
  multiple architecture slices. Similar to Apple's lipo tool.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "binformat/BinFormat.h"

typedef enum {
  CMD_NONE = 0,
  CMD_INFO,
  CMD_DETAILED_INFO,
  CMD_CREATE,
  CMD_THIN,
  CMD_REMOVE,
  CMD_REPLACE,
  CMD_ARCHS,
  CMD_VERIFY_ARCH
} LIPO_COMMAND;

typedef struct {
  LIPO_COMMAND  Command;
  CHAR8         *OutputFile;
  CHAR8         *ThinArch;
  CHAR8         *RemoveArch;
  CHAR8         *ReplaceArch;
  CHAR8         *ReplaceFile;
  CHAR8         *VerifyArch;
  CHAR8         **InputFiles;
  UINT32        InputFileCount;
} LIPO_OPTIONS;

STATIC LIPO_OPTIONS gOptions = {0};

/**
  Print usage information.
**/
STATIC
VOID
PrintUsage (
  VOID
  )
{
  printf ("Usage: lipo [options] <input_file(s)>\n");
  printf ("\n");
  printf ("Commands:\n");
  printf ("  -info <file>              List architectures in file\n");
  printf ("  -detailed_info <file>     List architectures with details\n");
  printf ("  -archs <file>             List architecture names only\n");
  printf ("  -verify_arch <arch> <file> Verify file contains architecture\n");
  printf ("  -create <files...>        Create fat binary from thin binaries\n");
  printf ("  -thin <arch> <file>       Extract single architecture\n");
  printf ("  -remove <arch> <file>     Remove architecture from fat binary\n");
  printf ("  -replace <arch> <file> <fat> Replace architecture in fat binary\n");
  printf ("\n");
  printf ("Options:\n");
  printf ("  -output <file>            Output file path\n");
  printf ("  -h, --help                Show this help message\n");
  printf ("\n");
  printf ("Examples:\n");
  printf ("  lipo -info app.universal\n");
  printf ("  lipo -create app.x86_64 app.arm64 -output app.universal\n");
  printf ("  lipo -thin arm64 app.universal -output app.arm64\n");
  printf ("  lipo -remove x86_64 app.universal -output app.arm_only\n");
}

/**
  Parse machine name to BINFORMAT_MACHINE enum.

  @param[in]  Name  Architecture name (e.g., "x86_64", "arm64").

  @return BINFORMAT_MACHINE value.
**/
STATIC
BINFORMAT_MACHINE
ParseMachineName (
  IN  CONST CHAR8  *Name
  )
{
  if (strcmp (Name, "x86_64") == 0 || strcmp (Name, "x64") == 0) {
    return BinMachineX64;
  } else if (strcmp (Name, "i386") == 0 || strcmp (Name, "x86") == 0) {
    return BinMachineX86;
  } else if (strcmp (Name, "arm64") == 0 || strcmp (Name, "aarch64") == 0) {
    return BinMachineARM64;
  } else if (strcmp (Name, "arm") == 0 || strcmp (Name, "armv7") == 0) {
    return BinMachineARM;
  } else if (strcmp (Name, "ppc") == 0 || strcmp (Name, "powerpc") == 0) {
    return BinMachinePowerPC;
  } else if (strcmp (Name, "ppc64") == 0 || strcmp (Name, "powerpc64") == 0) {
    return BinMachinePowerPC64;
  } else if (strcmp (Name, "mips") == 0) {
    return BinMachineMIPS;
  } else if (strcmp (Name, "mips64") == 0) {
    return BinMachineMIPS64;
  } else if (strcmp (Name, "riscv32") == 0) {
    return BinMachineRISCV32;
  } else if (strcmp (Name, "riscv64") == 0) {
    return BinMachineRISCV64;
  }

  return BinMachineUnknown;
}

/**
  Display information about a binary file.

  @param[in]  FileName   Path to binary file.
  @param[in]  Detailed   TRUE for detailed output.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
DisplayInfo (
  IN  CONST CHAR8  *FileName,
  IN  BOOLEAN      Detailed
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_CONTEXT    *Context;
  BINFORMAT_STATUS     Status;
  BINFORMAT_HEADER_INFO HeaderInfo;
  UINT32               i;

  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "lipo: %s: File format not recognized\n", FileName);
    return 1;
  }

  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "lipo: %s: Failed to get header\n", FileName);
    Api->Close (Context);
    return 1;
  }

  if (HeaderInfo.ArchitectureCount > 1) {
    printf ("Architectures in the fat file: %s are:", FileName);
    for (i = 0; i < HeaderInfo.ArchitectureCount; i++) {
      printf (" %s", BinFormatGetMachineName (HeaderInfo.Architectures[i].Machine));
    }
    printf ("\n");

    if (Detailed) {
      for (i = 0; i < HeaderInfo.ArchitectureCount; i++) {
        printf ("\n");
        printf ("architecture %s\n", BinFormatGetMachineName (HeaderInfo.Architectures[i].Machine));
        printf ("    offset %llu\n", (unsigned long long)HeaderInfo.Architectures[i].Offset);
        printf ("    size %llu\n", (unsigned long long)HeaderInfo.Architectures[i].Size);
        printf ("    align 2^12 (4096)\n");
      }
    }
  } else {
    printf ("Non-fat file: %s is architecture: %s\n",
            FileName,
            BinFormatGetMachineName (HeaderInfo.Machine));
  }

  Api->Close (Context);
  return 0;
}

/**
  List architecture names only.

  @param[in]  FileName   Path to binary file.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
ListArchs (
  IN  CONST CHAR8  *FileName
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_CONTEXT    *Context;
  BINFORMAT_STATUS     Status;
  BINFORMAT_HEADER_INFO HeaderInfo;
  UINT32               i;

  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "lipo: %s: File format not recognized\n", FileName);
    return 1;
  }

  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "lipo: %s: Failed to get header\n", FileName);
    Api->Close (Context);
    return 1;
  }

  if (HeaderInfo.ArchitectureCount > 1) {
    for (i = 0; i < HeaderInfo.ArchitectureCount; i++) {
      printf ("%s", BinFormatGetMachineName (HeaderInfo.Architectures[i].Machine));
      if (i < HeaderInfo.ArchitectureCount - 1) {
        printf (" ");
      }
    }
    printf ("\n");
  } else {
    printf ("%s\n", BinFormatGetMachineName (HeaderInfo.Machine));
  }

  Api->Close (Context);
  return 0;
}

/**
  Verify that a file contains a specific architecture.

  @param[in]  FileName   Path to binary file.
  @param[in]  ArchName   Architecture name to verify.

  @return 0 if architecture exists, 1 if not.
**/
STATIC
INT32
VerifyArch (
  IN  CONST CHAR8  *FileName,
  IN  CONST CHAR8  *ArchName
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_CONTEXT    *Context;
  BINFORMAT_STATUS     Status;
  BINFORMAT_HEADER_INFO HeaderInfo;
  BINFORMAT_MACHINE    Machine;
  UINT32               i;
  BOOLEAN              Found = FALSE;

  Machine = ParseMachineName (ArchName);
  if (Machine == BinMachineUnknown) {
    fprintf (stderr, "lipo: unknown architecture: %s\n", ArchName);
    return 1;
  }

  Api = BinFormatDetectFile (FileName, &Context, TRUE);
  if (Api == NULL) {
    return 1;
  }

  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    Api->Close (Context);
    return 1;
  }

  if (HeaderInfo.ArchitectureCount > 1) {
    for (i = 0; i < HeaderInfo.ArchitectureCount; i++) {
      if (HeaderInfo.Architectures[i].Machine == Machine) {
        Found = TRUE;
        break;
      }
    }
  } else {
    Found = (HeaderInfo.Machine == Machine);
  }

  Api->Close (Context);
  return Found ? 0 : 1;
}

/**
  Create a fat binary from multiple thin binaries.
  Note: This is a stub - CreateFat API not yet implemented in libraries.

  @param[in]  InputFiles      Array of input file paths.
  @param[in]  FileCount       Number of input files.
  @param[in]  OutputFile      Output file path.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
CreateFat (
  IN  CHAR8       **InputFiles,
  IN  UINT32      FileCount,
  IN  CONST CHAR8 *OutputFile
  )
{
  if (FileCount == 0) {
    fprintf (stderr, "lipo: no input files specified for -create\n");
    return 1;
  }

  if (OutputFile == NULL) {
    fprintf (stderr, "lipo: no output file specified (use -output)\n");
    return 1;
  }

  fprintf (stderr, "lipo: -create not yet fully implemented\n");
  fprintf (stderr, "Creating fat binary from:\n");
  for (UINT32 i = 0; i < FileCount; i++) {
    fprintf (stderr, "  %s\n", InputFiles[i]);
  }
  fprintf (stderr, "Output would be: %s\n", OutputFile);
  fprintf (stderr, "\nNote: CreateFat callback needs implementation in library\n");

  return 1;
}

/**
  Extract a thin binary for a specific architecture.
  Note: This is a stub - ExtractThin API not yet implemented in libraries.

  @param[in]  InputFile   Path to fat binary.
  @param[in]  ArchName    Architecture name to extract.
  @param[in]  OutputFile  Output file path.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
ExtractThin (
  IN  CONST CHAR8  *InputFile,
  IN  CONST CHAR8  *ArchName,
  IN  CONST CHAR8  *OutputFile
  )
{
  if (OutputFile == NULL) {
    fprintf (stderr, "lipo: no output file specified (use -output)\n");
    return 1;
  }

  fprintf (stderr, "lipo: -thin not yet fully implemented\n");
  fprintf (stderr, "Would extract %s from %s to %s\n", ArchName, InputFile, OutputFile);
  fprintf (stderr, "\nNote: ExtractThin callback needs implementation in library\n");

  return 1;
}

/**
  Remove an architecture from a fat binary.
  Note: This is a stub - RemoveArchSlice API not yet implemented in libraries.

  @param[in]  InputFile   Path to fat binary.
  @param[in]  ArchName    Architecture name to remove.
  @param[in]  OutputFile  Output file path.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
RemoveArch (
  IN  CONST CHAR8  *InputFile,
  IN  CONST CHAR8  *ArchName,
  IN  CONST CHAR8  *OutputFile
  )
{
  if (OutputFile == NULL) {
    fprintf (stderr, "lipo: no output file specified (use -output)\n");
    return 1;
  }

  fprintf (stderr, "lipo: -remove not yet fully implemented\n");
  fprintf (stderr, "Would remove %s from %s and save to %s\n", ArchName, InputFile, OutputFile);
  fprintf (stderr, "\nNote: RemoveArchSlice callback needs implementation in library\n");

  return 1;
}

/**
  Replace an architecture in a fat binary.
  Note: This is a stub - ReplaceArchSlice API not yet implemented in libraries.

  @param[in]  FatFile      Path to fat binary.
  @param[in]  ArchName     Architecture name to replace.
  @param[in]  ReplaceFile  Path to replacement binary.
  @param[in]  OutputFile   Output file path.

  @return 0 on success, 1 on error.
**/
STATIC
INT32
ReplaceArch (
  IN  CONST CHAR8  *FatFile,
  IN  CONST CHAR8  *ArchName,
  IN  CONST CHAR8  *ReplaceFile,
  IN  CONST CHAR8  *OutputFile
  )
{
  if (OutputFile == NULL) {
    fprintf (stderr, "lipo: no output file specified (use -output)\n");
    return 1;
  }

  fprintf (stderr, "lipo: -replace not yet fully implemented\n");
  fprintf (stderr, "Would replace %s in %s with %s and save to %s\n",
           ArchName, FatFile, ReplaceFile, OutputFile);
  fprintf (stderr, "\nNote: ReplaceArchSlice callback needs implementation in library\n");

  return 1;
}

/**
  Main entry point.

  @param[in]  argc  Argument count.
  @param[in]  argv  Argument array.

  @return Exit code (0 for success).
**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  INT32   i;
  INT32   Result = 0;

  if (argc < 2) {
    PrintUsage ();
    return 1;
  }

  //
  // Parse command-line arguments
  //
  for (i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
      PrintUsage ();
      return 0;
    } else if (strcmp (argv[i], "-info") == 0) {
      gOptions.Command = CMD_INFO;
      if (i + 1 < argc) {
        gOptions.InputFiles = &argv[i + 1];
        gOptions.InputFileCount = 1;
        i++;
      }
    } else if (strcmp (argv[i], "-detailed_info") == 0) {
      gOptions.Command = CMD_DETAILED_INFO;
      if (i + 1 < argc) {
        gOptions.InputFiles = &argv[i + 1];
        gOptions.InputFileCount = 1;
        i++;
      }
    } else if (strcmp (argv[i], "-archs") == 0) {
      gOptions.Command = CMD_ARCHS;
      if (i + 1 < argc) {
        gOptions.InputFiles = &argv[i + 1];
        gOptions.InputFileCount = 1;
        i++;
      }
    } else if (strcmp (argv[i], "-verify_arch") == 0) {
      gOptions.Command = CMD_VERIFY_ARCH;
      if (i + 1 < argc) {
        gOptions.VerifyArch = argv[i + 1];
        i++;
        if (i + 1 < argc) {
          gOptions.InputFiles = &argv[i + 1];
          gOptions.InputFileCount = 1;
          i++;
        }
      }
    } else if (strcmp (argv[i], "-create") == 0) {
      gOptions.Command = CMD_CREATE;
    } else if (strcmp (argv[i], "-thin") == 0) {
      gOptions.Command = CMD_THIN;
      if (i + 1 < argc) {
        gOptions.ThinArch = argv[i + 1];
        i++;
      }
    } else if (strcmp (argv[i], "-remove") == 0) {
      gOptions.Command = CMD_REMOVE;
      if (i + 1 < argc) {
        gOptions.RemoveArch = argv[i + 1];
        i++;
      }
    } else if (strcmp (argv[i], "-replace") == 0) {
      gOptions.Command = CMD_REPLACE;
      if (i + 1 < argc) {
        gOptions.ReplaceArch = argv[i + 1];
        i++;
      }
    } else if (strcmp (argv[i], "-output") == 0) {
      if (i + 1 < argc) {
        gOptions.OutputFile = argv[i + 1];
        i++;
      }
    } else if (gOptions.Command == CMD_REPLACE && gOptions.ReplaceArch != NULL && gOptions.ReplaceFile == NULL) {
      gOptions.ReplaceFile = argv[i];
    } else if (gOptions.InputFiles == NULL || gOptions.Command == CMD_CREATE) {
      gOptions.InputFiles = &argv[i];
      gOptions.InputFileCount = argc - i;
      break;
    }
  }

  //
  // Execute command
  //
  switch (gOptions.Command) {
    case CMD_INFO:
      if (gOptions.InputFileCount != 1) {
        fprintf (stderr, "lipo: -info requires one input file\n");
        return 1;
      }
      Result = DisplayInfo (gOptions.InputFiles[0], FALSE);
      break;

    case CMD_DETAILED_INFO:
      if (gOptions.InputFileCount != 1) {
        fprintf (stderr, "lipo: -detailed_info requires one input file\n");
        return 1;
      }
      Result = DisplayInfo (gOptions.InputFiles[0], TRUE);
      break;

    case CMD_ARCHS:
      if (gOptions.InputFileCount != 1) {
        fprintf (stderr, "lipo: -archs requires one input file\n");
        return 1;
      }
      Result = ListArchs (gOptions.InputFiles[0]);
      break;

    case CMD_VERIFY_ARCH:
      if (gOptions.InputFileCount != 1) {
        fprintf (stderr, "lipo: -verify_arch requires one input file\n");
        return 1;
      }
      Result = VerifyArch (gOptions.InputFiles[0], gOptions.VerifyArch);
      break;

    case CMD_CREATE:
      Result = CreateFat (gOptions.InputFiles, gOptions.InputFileCount, gOptions.OutputFile);
      break;

    case CMD_THIN:
      if (gOptions.InputFileCount < 1) {
        fprintf (stderr, "lipo: -thin requires input file\n");
        return 1;
      }
      Result = ExtractThin (gOptions.InputFiles[0], gOptions.ThinArch, gOptions.OutputFile);
      break;

    case CMD_REMOVE:
      if (gOptions.InputFileCount < 1) {
        fprintf (stderr, "lipo: -remove requires input file\n");
        return 1;
      }
      Result = RemoveArch (gOptions.InputFiles[0], gOptions.RemoveArch, gOptions.OutputFile);
      break;

    case CMD_REPLACE:
      if (gOptions.InputFileCount < 1) {
        fprintf (stderr, "lipo: -replace requires fat file\n");
        return 1;
      }
      Result = ReplaceArch (gOptions.InputFiles[0], gOptions.ReplaceArch,
                            gOptions.ReplaceFile, gOptions.OutputFile);
      break;

    default:
      fprintf (stderr, "lipo: no command specified\n");
      PrintUsage ();
      Result = 1;
      break;
  }

  return Result;
}
