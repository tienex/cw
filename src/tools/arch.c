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
  Check if two architecture names match (handling variations).

  @param[in]  Arch1  First architecture name.
  @param[in]  Arch2  Second architecture name.

  @retval TRUE   Architectures match.
  @retval FALSE  Architectures differ.
**/
STATIC
BOOLEAN
ArchNamesMatch (
  IN  CONST CHAR8  *Arch1,
  IN  CONST CHAR8  *Arch2
  )
{
  if (strcmp (Arch1, Arch2) == 0) {
    return TRUE;
  }

  //
  // Handle common variations
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
      (strcmp (Arch1, "x86") == 0 && strcmp (Arch1, "i386") == 0)) {
    return TRUE;
  }

  return FALSE;
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
  if (ArchNamesMatch (Arch, "x86_64")) {
    return "qemu-x86_64";
  } else if (ArchNamesMatch (Arch, "i386")) {
    return "qemu-i386";
  } else if (ArchNamesMatch (Arch, "arm64")) {
    return "qemu-aarch64";
  } else if (ArchNamesMatch (Arch, "arm")) {
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
  Extract thin slice from fat binary to temporary file.

  @param[in]  BinaryPath  Path to fat binary.
  @param[in]  Arch        Architecture to extract.
  @param[out] ThinPath    Path to extracted thin binary.

  @retval 0  Success.
  @retval 1  Error.
**/
STATIC
INT32
ExtractThinSlice (
  IN  CONST CHAR8  *BinaryPath,
  IN  CONST CHAR8  *Arch,
  OUT CHAR8        *ThinPath,
  IN  size_t       ThinPathSize
  )
{
  CONST BINFORMAT_API    *Api;
  BINFORMAT_CONTEXT      *FatContext;
  BINFORMAT_CONTEXT      *ThinContext;
  BINFORMAT_STATUS       Status;
  BINFORMAT_HEADER_INFO  HeaderInfo;
  UINT32                 ArchIndex;
  UINT32                 i;
  BOOLEAN                Found = FALSE;

  //
  // Detect file format
  //
  Api = BinFormatDetectFile (BinaryPath, &FatContext, TRUE);
  if (Api == NULL) {
    fprintf (stderr, "arch: %s: cannot detect file format\n", BinaryPath);
    return 1;
  }

  //
  // Get header to find architecture index
  //
  Status = Api->GetHeader (FatContext, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "arch: %s: cannot get header\n", BinaryPath);
    Api->Close (FatContext);
    return 1;
  }

  //
  // Find architecture index
  //
  for (i = 0; i < HeaderInfo.ArchitectureCount; i++) {
    if (ArchNamesMatch (Arch, BinFormatGetMachineName (HeaderInfo.Architectures[i].Machine))) {
      ArchIndex = i;
      Found = TRUE;
      break;
    }
  }

  if (!Found) {
    fprintf (stderr, "arch: %s: architecture %s not found\n", BinaryPath, Arch);
    Api->Close (FatContext);
    return 1;
  }

  //
  // Check if ExtractThin is supported
  //
  if (Api->ExtractThin == NULL) {
    fprintf (stderr, "arch: %s: thin extraction not supported\n", BinaryPath);
    Api->Close (FatContext);
    return 1;
  }

  //
  // Extract thin slice
  //
  Status = Api->ExtractThin (FatContext, ArchIndex, &ThinContext);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "arch: failed to extract %s slice from %s\n", Arch, BinaryPath);
    Api->Close (FatContext);
    return 1;
  }

  //
  // Create temporary file path
  //
  snprintf (ThinPath, ThinPathSize, "/tmp/arch_thin_%d", getpid ());

  //
  // Write thin slice to file
  //
  if (Api->WriteFile == NULL) {
    fprintf (stderr, "arch: WriteFile not supported\n");
    Api->Close (ThinContext);
    Api->Close (FatContext);
    return 1;
  }

  Status = Api->WriteFile (ThinContext, ThinPath);
  if (BINFORMAT_IS_ERROR (Status)) {
    fprintf (stderr, "arch: failed to write thin slice to %s\n", ThinPath);
    Api->Close (ThinContext);
    Api->Close (FatContext);
    return 1;
  }

  Api->Close (ThinContext);
  Api->Close (FatContext);
  return 0;
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
  BINFORMAT_CONTEXT         *ThinContext;
  BINFORMAT_STATUS          Status;
  BINFORMAT_HEADER_INFO     HeaderInfo;
  CONST CHAR8               *CurrentArch;
  CONST CHAR8               *QemuBin;
  CHAR8                     *ExecArgs[256];
  INT32                     fd;
  pid_t                     pid;
  INT32                     ExitStatus;
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
      if (ArchNamesMatch (Arch, BinFormatGetMachineName (HeaderInfo.Architectures[i].Machine))) {
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
  // Determine if emulation is needed
  //
  if (!ArchNamesMatch (Arch, CurrentArch) || gOptions.Emulated) {
    //
    // Need emulation via QEMU
    //
    QemuBin = GetQemuEmulator (Arch);
    if (QemuBin == NULL) {
      fprintf (stderr, "arch: no QEMU emulator available for %s\n", Arch);
      Api->Close (FatContext);
      return 1;
    }

    //
    // For fat binaries, extract thin slice to memfd
    //
    if (IsFat) {
      if (Api->ExtractThin == NULL) {
        fprintf (stderr, "arch: ExtractThin not supported\n");
        Api->Close (FatContext);
        return 1;
      }

      Status = Api->ExtractThin (FatContext, ArchIndex, &ThinContext);
      if (BINFORMAT_IS_ERROR (Status)) {
        fprintf (stderr, "arch: failed to extract thin slice\n");
        Api->Close (FatContext);
        return 1;
      }

      //
      // Create memfd for the thin slice
      //
      fd = memfd_create ("arch_qemu", MFD_CLOEXEC);
      if (fd < 0) {
        perror ("arch: memfd_create failed");
        Api->Close (ThinContext);
        Api->Close (FatContext);
        return 1;
      }

      //
      // Write thin slice to memfd using WriteMemory
      //
      UINT8  *Buffer;
      UINT64 BufferSize = 64 * 1024 * 1024;  // 64MB should be enough
      UINT64 Written;

      Buffer = malloc (BufferSize);
      if (Buffer == NULL) {
        close (fd);
        Api->Close (ThinContext);
        Api->Close (FatContext);
        return 1;
      }

      Status = Api->WriteMemory (ThinContext, Buffer, BufferSize, &Written);
      if (BINFORMAT_IS_ERROR (Status)) {
        fprintf (stderr, "arch: failed to write thin slice to memory\n");
        free (Buffer);
        close (fd);
        Api->Close (ThinContext);
        Api->Close (FatContext);
        return 1;
      }

      if (write (fd, Buffer, Written) != (ssize_t)Written) {
        perror ("arch: write to memfd failed");
        free (Buffer);
        close (fd);
        Api->Close (ThinContext);
        Api->Close (FatContext);
        return 1;
      }

      free (Buffer);
      Api->Close (ThinContext);

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
  }

  //
  // Native execution using fexecve
  //
  if (IsFat) {
    //
    // For fat binaries, extract thin slice to memfd
    //
    if (Api->ExtractThin == NULL) {
      fprintf (stderr, "arch: ExtractThin not supported\n");
      Api->Close (FatContext);
      return 1;
    }

    Status = Api->ExtractThin (FatContext, ArchIndex, &ThinContext);
    if (BINFORMAT_IS_ERROR (Status)) {
      fprintf (stderr, "arch: failed to extract thin slice\n");
      Api->Close (FatContext);
      return 1;
    }

    //
    // Create memfd for the thin slice
    //
    fd = memfd_create ("arch_exec", MFD_CLOEXEC);
    if (fd < 0) {
      perror ("arch: memfd_create failed");
      Api->Close (ThinContext);
      Api->Close (FatContext);
      return 1;
    }

    //
    // Write thin slice to memfd using WriteMemory
    //
    UINT8  *Buffer;
    UINT64 BufferSize = 64 * 1024 * 1024;  // 64MB should be enough
    UINT64 Written;

    Buffer = malloc (BufferSize);
    if (Buffer == NULL) {
      close (fd);
      Api->Close (ThinContext);
      Api->Close (FatContext);
      return 1;
    }

    Status = Api->WriteMemory (ThinContext, Buffer, BufferSize, &Written);
    if (BINFORMAT_IS_ERROR (Status)) {
      fprintf (stderr, "arch: failed to write thin slice to memory\n");
      free (Buffer);
      close (fd);
      Api->Close (ThinContext);
      Api->Close (FatContext);
      return 1;
    }

    if (write (fd, Buffer, Written) != (ssize_t)Written) {
      perror ("arch: write to memfd failed");
      free (Buffer);
      close (fd);
      Api->Close (ThinContext);
      Api->Close (FatContext);
      return 1;
    }

    free (Buffer);

    //
    // Seek back to start
    //
    lseek (fd, 0, SEEK_SET);

    Api->Close (ThinContext);
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
      printf ("  -emulated     Force emulation via QEMU\n");
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
