/** @file
  arch - Architecture selection and universal binary executor.

  This tool displays architecture information and executes binaries
  with specific architecture selection, optionally using emulation.
  Similar to macOS arch command.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <getopt.h>
#include "binformat/BinFormat.h"

typedef struct {
  CHAR8     *TargetArch;
  CHAR8     **Command;
  INT32     CommandArgc;
  BOOLEAN   Emulated;
  BOOLEAN   List32bit;
  BOOLEAN   List64bit;
  BOOLEAN   ListAll;
} ARCH_OPTIONS;

STATIC ARCH_OPTIONS gOptions = {0};

/**
  Print usage information.
**/
STATIC
VOID
PrintUsage (
  IN  CONST CHAR8  *ProgramName
  )
{
  printf ("Usage: %s [options] [command [args...]]\n", ProgramName);
  printf ("\n");
  printf ("Options:\n");
  printf ("  -arch <arch>      Execute command with specific architecture\n");
  printf ("  -emulated         Use emulation (QEMU) if architecture not native\n");
  printf ("  -32               List 32-bit architectures\n");
  printf ("  -64               List 64-bit architectures\n");
  printf ("  -h, --help        Show this help message\n");
  printf ("\n");
  printf ("Architectures:\n");
  printf ("  i386, x86_64, arm, arm64, ppc, ppc64, mips, mips64, riscv32, riscv64\n");
  printf ("\n");
  printf ("Examples:\n");
  printf ("  %s                        # Show current architecture\n", ProgramName);
  printf ("  %s -32                    # List 32-bit architectures\n", ProgramName);
  printf ("  %s -arch x86_64 ./app     # Run app as x86_64\n", ProgramName);
  printf ("  %s -arch arm64 -emulated ./app  # Run app as ARM64 via QEMU\n", ProgramName);
}

/**
  Normalize architecture name for comparison.

  @param[in]  Arch  Architecture name.

  @return Normalized name.
**/
STATIC
BOOLEAN
ArchNamesMatch (
  IN  CONST CHAR8  *Arch1,
  IN  CONST CHAR8  *Arch2
  )
{
  //
  // Direct match
  //
  if (strcmp (Arch1, Arch2) == 0) {
    return TRUE;
  }

  //
  // Handle variations: x86_64 vs x86-64, aarch64 vs arm64, etc.
  //
  if ((strcmp (Arch1, "x86_64") == 0 && strcmp (Arch2, "x86-64") == 0) ||
      (strcmp (Arch1, "x86-64") == 0 && strcmp (Arch2, "x86_64") == 0)) {
    return TRUE;
  }

  if ((strcmp (Arch1, "arm64") == 0 && strcmp (Arch2, "aarch64") == 0) ||
      (strcmp (Arch1, "aarch64") == 0 && strcmp (Arch2, "arm64") == 0)) {
    return TRUE;
  }

  if ((strcmp (Arch1, "i386") == 0 && strcmp (Arch2, "x86") == 0) ||
      (strcmp (Arch1, "x86") == 0 && strcmp (Arch2, "i386") == 0)) {
    return TRUE;
  }

  return FALSE;
}

/**
  Get current system architecture.

  @return String name of current architecture.
**/
STATIC
CONST CHAR8 *
GetCurrentArch (
  VOID
  )
{
#if defined(__x86_64__) || defined(_M_X64)
  return "x86_64";
#elif defined(__i386__) || defined(_M_IX86)
  return "i386";
#elif defined(__aarch64__) || defined(_M_ARM64)
  return "arm64";
#elif defined(__arm__) || defined(_M_ARM)
  return "arm";
#elif defined(__powerpc64__) || defined(__ppc64__)
  return "ppc64";
#elif defined(__powerpc__) || defined(__ppc__)
  return "ppc";
#elif defined(__mips64)
  return "mips64";
#elif defined(__mips__)
  return "mips";
#elif defined(__riscv) && (__riscv_xlen == 64)
  return "riscv64";
#elif defined(__riscv) && (__riscv_xlen == 32)
  return "riscv32";
#else
  return "unknown";
#endif
}

/**
  Check if architecture is 64-bit.

  @param[in]  Arch  Architecture name.

  @return TRUE if 64-bit, FALSE otherwise.
**/
STATIC
BOOLEAN
Is64BitArch (
  IN  CONST CHAR8  *Arch
  )
{
  if (strcmp (Arch, "x86_64") == 0 ||
      strcmp (Arch, "arm64") == 0 ||
      strcmp (Arch, "ppc64") == 0 ||
      strcmp (Arch, "mips64") == 0 ||
      strcmp (Arch, "riscv64") == 0) {
    return TRUE;
  }
  return FALSE;
}

/**
  List supported architectures.

  @param[in]  Only32bit  List only 32-bit architectures.
  @param[in]  Only64bit  List only 64-bit architectures.
**/
STATIC
VOID
ListArchitectures (
  IN  BOOLEAN  Only32bit,
  IN  BOOLEAN  Only64bit
  )
{
  CONST CHAR8  *Archs[] = {
    "i386", "x86_64", "arm", "arm64", "ppc", "ppc64",
    "mips", "mips64", "riscv32", "riscv64", NULL
  };
  INT32  i;

  for (i = 0; Archs[i] != NULL; i++) {
    BOOLEAN Is64 = Is64BitArch (Archs[i]);

    if (Only32bit && Is64) {
      continue;
    }
    if (Only64bit && !Is64) {
      continue;
    }

    printf ("%s\n", Archs[i]);
  }
}

/**
  Get QEMU emulator name for architecture.

  @param[in]  Arch  Architecture name.

  @return QEMU binary name.
**/
STATIC
CONST CHAR8 *
GetQemuEmulator (
  IN  CONST CHAR8  *Arch
  )
{
  if (strcmp (Arch, "x86_64") == 0) {
    return "qemu-x86_64";
  } else if (strcmp (Arch, "i386") == 0) {
    return "qemu-i386";
  } else if (strcmp (Arch, "arm64") == 0 || strcmp (Arch, "aarch64") == 0) {
    return "qemu-aarch64";
  } else if (strcmp (Arch, "arm") == 0) {
    return "qemu-arm";
  } else if (strcmp (Arch, "ppc64") == 0) {
    return "qemu-ppc64";
  } else if (strcmp (Arch, "ppc") == 0) {
    return "qemu-ppc";
  } else if (strcmp (Arch, "mips64") == 0) {
    return "qemu-mips64";
  } else if (strcmp (Arch, "mips") == 0) {
    return "qemu-mips";
  } else if (strcmp (Arch, "riscv64") == 0) {
    return "qemu-riscv64";
  } else if (strcmp (Arch, "riscv32") == 0) {
    return "qemu-riscv32";
  }

  return NULL;
}

/**
  Execute command with specified architecture.

  @param[in]  Arch      Target architecture.
  @param[in]  Command   Command to execute.
  @param[in]  Argc      Argument count.
  @param[in]  Emulated  Use emulation if TRUE.

  @return Exit code from executed command.
**/
STATIC
INT32
ExecuteWithArch (
  IN  CONST CHAR8  *Arch,
  IN  CHAR8        **Command,
  IN  INT32        Argc,
  IN  BOOLEAN      Emulated
  )
{
  CONST CHAR8           *CurrentArch;
  CONST BINFORMAT_API   *Api;
  BINFORMAT_CONTEXT     *Context;
  BINFORMAT_STATUS      Status;
  BINFORMAT_HEADER_INFO HeaderInfo;
  pid_t                 pid;
  INT32                 ExitStatus;
  CONST CHAR8           *QemuBin;
  CHAR8                 **ExecArgs;
  INT32                 i;

  if (Argc == 0 || Command == NULL || Command[0] == NULL) {
    fprintf (stderr, "arch: no command specified\n");
    return 1;
  }

  CurrentArch = GetCurrentArch ();

  //
  // Check if binary exists and detect its format
  //
  Api = BinFormatDetectFile (Command[0], &Context, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "arch: %s: cannot execute: file format not recognized\n", Command[0]);
    return 127;
  }

  //
  // Get binary architecture
  //
  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "arch: %s: cannot get binary architecture\n", Command[0]);
    Api->Close (Context);
    return 127;
  }

  //
  // Check if binary supports requested architecture
  //
  if (HeaderInfo.ArchitectureCount > 1) {
    //
    // Fat binary - select appropriate slice
    //
    BOOLEAN Found = FALSE;
    for (UINT32 j = 0; j < HeaderInfo.ArchitectureCount; j++) {
      CONST CHAR8 *ArchName = BinFormatGetMachineName (HeaderInfo.Architectures[j].Machine);
      if (ArchNamesMatch (ArchName, Arch)) {
        Found = TRUE;
        if (Api->SelectArchitecture != NULL) {
          Api->SelectArchitecture (Context, j);
        }
        break;
      }
    }

    if (!Found) {
      fprintf (stderr, "arch: %s: architecture %s not found in fat binary\n", Command[0], Arch);
      Api->Close (Context);
      return 1;
    }
  } else {
    //
    // Thin binary - check if it matches
    //
    CONST CHAR8 *BinaryArch = BinFormatGetMachineName (HeaderInfo.Machine);
    if (!ArchNamesMatch (BinaryArch, Arch)) {
      fprintf (stderr, "arch: %s: binary is %s, not %s\n", Command[0], BinaryArch, Arch);
      Api->Close (Context);
      return 1;
    }
  }

  Api->Close (Context);

  //
  // Determine if emulation is needed
  //
  if (!ArchNamesMatch (Arch, CurrentArch) || Emulated) {
    //
    // Need emulation - use QEMU
    //
    QemuBin = GetQemuEmulator (Arch);
    if (QemuBin == NULL) {
      fprintf (stderr, "arch: no emulator available for %s\n", Arch);
      return 127;
    }

    //
    // Build QEMU command line: qemu-<arch> <binary> <args...>
    //
    ExecArgs = malloc (sizeof (CHAR8 *) * (Argc + 2));
    ExecArgs[0] = (CHAR8 *)QemuBin;
    for (i = 0; i < Argc; i++) {
      ExecArgs[i + 1] = Command[i];
    }
    ExecArgs[Argc + 1] = NULL;

    printf ("Executing via emulation: %s\n", QemuBin);

    pid = fork ();
    if (pid == 0) {
      //
      // Child process
      //
      execvp (QemuBin, ExecArgs);
      perror ("arch: execvp failed");
      exit (127);
    } else if (pid > 0) {
      //
      // Parent process
      //
      waitpid (pid, &ExitStatus, 0);
      free (ExecArgs);
      return WIFEXITED (ExitStatus) ? WEXITSTATUS (ExitStatus) : 1;
    } else {
      perror ("arch: fork failed");
      free (ExecArgs);
      return 127;
    }
  } else {
    //
    // Native execution
    //
    pid = fork ();
    if (pid == 0) {
      //
      // Child process
      //
      execvp (Command[0], Command);
      perror ("arch: execvp failed");
      exit (127);
    } else if (pid > 0) {
      //
      // Parent process
      //
      waitpid (pid, &ExitStatus, 0);
      return WIFEXITED (ExitStatus) ? WEXITSTATUS (ExitStatus) : 1;
    } else {
      perror ("arch: fork failed");
      return 127;
    }
  }
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
  //
  // Manual parsing for all options since -32 and -64 don't work with getopt
  //
  for (INT32 i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-arch") == 0) {
      if (i + 1 < argc) {
        gOptions.TargetArch = argv[i + 1];
        i++;  // Skip architecture name
      } else {
        fprintf (stderr, "arch: -arch requires architecture argument\n");
        return 1;
      }
    } else if (strcmp (argv[i], "-emulated") == 0) {
      gOptions.Emulated = TRUE;
    } else if (strcmp (argv[i], "-32") == 0) {
      gOptions.List32bit = TRUE;
    } else if (strcmp (argv[i], "-64") == 0) {
      gOptions.List64bit = TRUE;
    } else if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
      // Already handled
      continue;
    } else {
      //
      // Start of command
      //
      gOptions.Command = &argv[i];
      gOptions.CommandArgc = argc - i;
      break;
    }
  }

  //
  // Execute based on options
  //
  if (gOptions.List32bit || gOptions.List64bit) {
    ListArchitectures (gOptions.List32bit, gOptions.List64bit);
    return 0;
  }

  if (gOptions.Command == NULL) {
    //
    // No command - just display current architecture
    //
    printf ("%s\n", GetCurrentArch ());
    return 0;
  }

  if (gOptions.TargetArch != NULL) {
    //
    // Execute with specific architecture
    //
    return ExecuteWithArch (gOptions.TargetArch, gOptions.Command,
                            gOptions.CommandArgc, gOptions.Emulated);
  } else {
    //
    // Execute normally
    //
    return ExecuteWithArch (GetCurrentArch (), gOptions.Command,
                            gOptions.CommandArgc, FALSE);
  }
}
