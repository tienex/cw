/** @file
  arch - Architecture selector for universal binaries.

  Execute binaries with a specific architecture, using QEMU for emulation
  when necessary. Uses fexecve(2) for execution.

  Copyright (c) 2025. All rights reserved.
  SPDX-License-Identifier: MIT
**/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include "binformat/BinFormat.h"

typedef struct {
  CONST CHAR8  *Arch;
  BOOLEAN      Emulated;
} ARCH_OPTIONS;

STATIC ARCH_OPTIONS gOptions = {0};

/**
  Get current architecture.

  @return Architecture string.
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
  Get QEMU emulator for architecture.

  @param[in]  Arch  Architecture name.

  @return QEMU binary name or NULL.
**/
STATIC
CONST CHAR8 *
GetQemuEmulator (
  IN  CONST CHAR8  *Arch
  )
{
  if (BinFormatArchNamesMatch (Arch, "x86_64")) {
    return "qemu-x86_64";
  } else if (BinFormatArchNamesMatch (Arch, "i386")) {
    return "qemu-i386";
  } else if (BinFormatArchNamesMatch (Arch, "arm64")) {
    return "qemu-aarch64";
  } else if (BinFormatArchNamesMatch (Arch, "arm")) {
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
  Execute with specified architecture using fexecve.

  @param[in]  Arch     Target architecture.
  @param[in]  Command  Command and arguments.
  @param[in]  Argc     Number of arguments.

  @return Exit code.
**/
STATIC
INT32
ExecuteWithArch (
  IN  CONST CHAR8  *Arch,
  IN  CHAR8        **Command,
  IN  INT32        Argc
  )
{
  CONST BINFORMAT_API       *Api;
  BINFORMAT_CONTEXT         *FatContext;
  BINFORMAT_STATUS          Status;
  BINFORMAT_HEADER_INFO     HeaderInfo;
  CONST CHAR8               *CurrentArch;
  CONST CHAR8               *QemuBin;
  CHAR8                     *ExecArgs[256];
  INT32                     fd;
  BOOLEAN                   IsFat = FALSE;
  UINT32                    i;
  UINT32                    ArchIndex = 0;
  CHAR8                     ThinPath[256];
  BOOLEAN                   ExtractedThin = FALSE;

  if (Argc < 1) {
    fprintf (stderr, "arch: no command specified\n");
    return 1;
  }

  CurrentArch = GetCurrentArch ();

  //
  // Detect binary format
  //
  Api = BinFormatDetectFile (Command[0], &FatContext, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "arch: %s: cannot detect file format\n", Command[0]);
    return 1;
  }

  Status = Api->GetHeader (FatContext, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "arch: %s: cannot get header info\n", Command[0]);
    Api->Close (FatContext);
    return 1;
  }

  //
  // Check if it's a fat binary
  //
  IsFat = (HeaderInfo.ArchitectureCount > 1);

  //
  // Check if binary contains requested architecture
  //
  if (IsFat) {
    BOOLEAN ArchFound = FALSE;
    for (i = 0; i < HeaderInfo.ArchitectureCount; i++) {
      if (BinFormatArchNamesMatch (Arch, BinFormatGetMachineName (HeaderInfo.Architectures[i].Machine))) {
        ArchFound = TRUE;
        ArchIndex = i;
        break;
      }
    }

    if (!ArchFound) {
      fprintf (stderr, "arch: %s: binary does not contain %s slice\n", Command[0], Arch);
      Api->Close (FatContext);
      return 1;
    }
  }

  //
  // Determine if emulation is needed (only on Linux via QEMU)
  //
#ifdef __linux__
  BOOLEAN NeedEmulation = !BinFormatArchNamesMatch (Arch, CurrentArch) || gOptions.Emulated;
#else
  BOOLEAN NeedEmulation = !BinFormatArchNamesMatch (Arch, CurrentArch);
  // gOptions.Emulated is a no-op on non-Linux systems
#endif

  if (NeedEmulation) {
#ifndef __linux__
    fprintf (stderr, "arch: cannot execute %s binaries on this platform\n", Arch);
    Api->Close (FatContext);
    return 1;
#else
    //
    // Need emulation via QEMU (Linux only)
    //
    QemuBin = GetQemuEmulator (Arch);
    if (QemuBin == NULL) {
      fprintf (stderr, "arch: no QEMU emulator available for %s\n", Arch);
      Api->Close (FatContext);
      return 1;
    }

    //
    // For fat binaries, extract thin slice to memfd using splice
    //
    if (IsFat) {
      INT32  SourceFd;

      //
      // Open source file for splice
      //
      SourceFd = open (Command[0], O_RDONLY);
      if (SourceFd < 0) {
        perror ("arch: failed to open source file");
        Api->Close (FatContext);
        return 1;
      }

      //
      // Create memfd for the thin slice
      //
      fd = memfd_create ("arch_qemu", MFD_CLOEXEC);
      if (fd < 0) {
        perror ("arch: memfd_create failed");
        close (SourceFd);
        Api->Close (FatContext);
        return 1;
      }

      //
      // Extract thin slice directly using splice (zero-copy on Linux)
      //
      Status = BinFormatExtractThinToFd (Api, FatContext, ArchIndex, SourceFd, fd);
      close (SourceFd);

      if (BINFORMAT_IS_ERROR (Status)) {
        fprintf (stderr, "arch: failed to extract thin slice\n");
        close (fd);
        Api->Close (FatContext);
        return 1;
      }

      //
      // Build /proc/self/fd/N path
      //
      snprintf (ThinPath, sizeof (ThinPath), "/proc/self/fd/%d", fd);
      ExtractedThin = TRUE;
    }

    Api->Close (FatContext);

    //
    // Build QEMU command: qemu-<arch> /proc/self/fd/N <args...>
    //
    ExecArgs[0] = (CHAR8 *)QemuBin;
    ExecArgs[1] = ExtractedThin ? ThinPath : Command[0];
    for (i = 1; i < (UINT32)Argc && i < 254; i++) {
      ExecArgs[i + 1] = Command[i];
    }
    ExecArgs[i + 1] = NULL;

    //
    // Execute via QEMU (no fork needed, fd will be inherited)
    //
    execvp (QemuBin, ExecArgs);

    //
    // If we get here, execvp failed
    //
    perror ("arch: execvp failed");
    if (ExtractedThin) {
      close (fd);
    }
    return 1;
#endif  // __linux__
  }

  //
  // Native execution using fexecve
  //
  if (IsFat) {
    INT32  SourceFd;

    //
    // Open source file for extraction
    //
    SourceFd = open (Command[0], O_RDONLY);
    if (SourceFd < 0) {
      perror ("arch: failed to open source file");
      Api->Close (FatContext);
      return 1;
    }

    //
    // Create anonymous memory file descriptor for the thin slice
    //
#if defined(__linux__)
    fd = memfd_create ("arch_exec", MFD_CLOEXEC);
    if (fd < 0) {
      perror ("arch: memfd_create failed");
      close (SourceFd);
      Api->Close (FatContext);
      return 1;
    }
#elif defined(__FreeBSD__)
    //
    // FreeBSD: use shm_open with SHM_ANON
    //
    fd = shm_open (SHM_ANON, O_RDWR | O_CLOEXEC, 0600);
    if (fd < 0) {
      perror ("arch: shm_open failed");
      close (SourceFd);
      Api->Close (FatContext);
      return 1;
    }
#else
    //
    // Other systems: use regular tmpfile
    //
    FILE *tmpf = tmpfile ();
    if (tmpf == NULL) {
      perror ("arch: tmpfile failed");
      close (SourceFd);
      Api->Close (FatContext);
      return 1;
    }
    fd = fileno (tmpf);
#endif

    //
    // Extract thin slice directly (uses splice on Linux, read/write elsewhere)
    //
    Status = BinFormatExtractThinToFd (Api, FatContext, ArchIndex, SourceFd, fd);
    close (SourceFd);

    if (BINFORMAT_IS_ERROR (Status)) {
      fprintf (stderr, "arch: failed to extract thin slice\n");
      close (fd);
      Api->Close (FatContext);
      return 1;
    }

    Api->Close (FatContext);
  } else {
    //
    // For thin binaries, open the file directly
    //
    Api->Close (FatContext);

    fd = open (Command[0], O_RDONLY);
    if (fd < 0) {
      perror ("arch: open failed");
      return 1;
    }
  }

  //
  // Build argument array
  //
  for (i = 0; i < (UINT32)Argc && i < 255; i++) {
    ExecArgs[i] = Command[i];
  }
  ExecArgs[i] = NULL;

  //
  // Execute using fexecve
  //
  fexecve (fd, ExecArgs, environ);

  //
  // If we get here, fexecve failed
  //
  perror ("arch: fexecve failed");
  close (fd);
  return 1;
}

/**
  Main entry point.

  @param[in]  argc  Argument count.
  @param[in]  argv  Argument array.

  @return Exit code.
**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  INT32  CommandStart = 0;

  //
  // No arguments: print current architecture
  //
  if (argc < 2) {
    printf ("%s\n", GetCurrentArch ());
    return 0;
  }

  //
  // Parse options manually
  //
  for (INT32 i = 1; i < argc; i++) {
    if (strcmp (argv[i], "-arch") == 0) {
      if (i + 1 >= argc) {
        fprintf (stderr, "arch: -arch requires an argument\n");
        return 1;
      }
      gOptions.Arch = argv[i + 1];
      i++;
    } else if (strcmp (argv[i], "-emulated") == 0) {
      gOptions.Emulated = TRUE;
    } else if (strcmp (argv[i], "-h") == 0 || strcmp (argv[i], "--help") == 0) {
      printf ("Usage: arch [-arch <arch>] [-emulated] <command> [args...]\n");
      printf ("       arch (no arguments to show current architecture)\n");
      printf ("\n");
      printf ("Options:\n");
      printf ("  -arch <arch>  Execute with specified architecture\n");
      printf ("  -emulated     Force emulation via QEMU (Linux only, no-op elsewhere)\n");
      printf ("  -h, --help    Show this help\n");
      printf ("\n");
      printf ("Examples:\n");
      printf ("  arch                           # Show current architecture\n");
      printf ("  arch -arch x86_64 /bin/echo hi # Execute echo with x86_64\n");
      return 0;
    } else {
      //
      // First non-option is the command
      //
      CommandStart = i;
      break;
    }
  }

  if (gOptions.Arch == NULL) {
    fprintf (stderr, "arch: -arch option required when executing commands\n");
    return 1;
  }

  if (CommandStart == 0) {
    fprintf (stderr, "arch: no command specified\n");
    return 1;
  }

  return ExecuteWithArch (gOptions.Arch, &argv[CommandStart], argc - CommandStart);
}
