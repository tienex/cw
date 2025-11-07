/** @file
  Binary Format Helper Functions Implementation.

  This file implements helper functions and default implementations that
  tool developers can use without reinventing the wheel.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#ifdef __linux__
  #define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#ifdef __linux__
  #include <fcntl.h>
#endif
#include "../../include/binformat/BinFormat.h"

//
// Array of all supported format APIs
//
STATIC CONST BINFORMAT_API * (*gFormatApis[])(VOID) = {
  ElfGetApi,
  CoffGetApi,
  MachoGetApi,
  AoutGetApi,
  OmfGetApi,
  OrfGetApi,
  MinidumpGetApi,
  NULL
};

/**
  Detect binary format from file.

  This function tries to detect the binary format by attempting to initialize
  with each known format library. Returns the API of the first library that
  successfully recognizes the format.

  @param[in]   FilePath          Path to binary file.
  @param[out]  Context           Pointer to receive context handle.
  @param[in]   ReadOnly          TRUE for read-only access.

  @retval Pointer to API table if format detected.
  @retval NULL if format not recognized.
**/
CONST BINFORMAT_API *
BinFormatDetectFile(
  IN  CONST CHAR8        *FilePath,
  OUT BINFORMAT_CONTEXT  **Context,
  IN  BOOLEAN            ReadOnly
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_STATUS     Status;
  UINT32               i;

  if (FilePath == NULL || Context == NULL) {
    return NULL;
  }

  *Context = NULL;

  //
  // Try each format in order
  //
  for (i = 0; gFormatApis[i] != NULL; i++) {
    Api = gFormatApis[i]();

    Status = Api->InitFile (Context, FilePath, ReadOnly);
    if (Status == BINFORMAT_SUCCESS) {
      return Api;
    }
  }

  return NULL;
}

/**
  Detect binary format from memory buffer.

  @param[in]   Buffer            Pointer to binary data.
  @param[in]   Size              Size of binary data.
  @param[out]  Context           Pointer to receive context handle.

  @retval Pointer to API table if format detected.
  @retval NULL if format not recognized.
**/
CONST BINFORMAT_API *
BinFormatDetectMemory(
  IN  CONST VOID         *Buffer,
  IN  UINT64             Size,
  OUT BINFORMAT_CONTEXT  **Context
  )
{
  CONST BINFORMAT_API  *Api;
  BINFORMAT_STATUS     Status;
  UINT32               i;

  if (Buffer == NULL || Size == 0 || Context == NULL) {
    return NULL;
  }

  *Context = NULL;

  //
  // Try each format in order
  //
  for (i = 0; gFormatApis[i] != NULL; i++) {
    Api = gFormatApis[i]();

    Status = Api->InitMemory (Context, Buffer, Size);
    if (Status == BINFORMAT_SUCCESS) {
      return Api;
    }
  }

  return NULL;
}

/**
  Get human-readable name for machine type.

  @param[in]   Machine           Machine type.

  @return Pointer to machine name string.
**/
CONST CHAR8 *
BinFormatGetMachineName(
  IN  BINFORMAT_MACHINE  Machine
  )
{
  switch (Machine) {
    case BinMachineMMIX:          return "MMIX";
    case BinMachineX86:           return "x86";
    case BinMachineX64:           return "x86-64";
    case BinMachineARM:           return "ARM";
    case BinMachineARM64:         return "ARM64";
    case BinMachinePowerPC:       return "PowerPC";
    case BinMachinePowerPC64:     return "PowerPC64";
    case BinMachineMIPS:          return "MIPS";
    case BinMachineMIPS64:        return "MIPS64";
    case BinMachineSPARC:         return "SPARC";
    case BinMachineSPARC64:       return "SPARC64";
    case BinMachineRISCV32:       return "RISC-V 32";
    case BinMachineRISCV64:       return "RISC-V 64";
    case BinMachineIA64:          return "IA-64";
    case BinMachineAlpha:         return "Alpha";
    case BinMachineM68K:          return "m68k";
    case BinMachineVAX:           return "VAX";
    case BinMachinePDP11:         return "PDP-11";
    case BinMachineS390:          return "S/390";
    case BinMachineS390X:         return "S/390X";
    case BinMachineWE32K:         return "WE32000";
    case BinMachineNS32K:         return "NS32000";
    case BinMachineTahoe:         return "Tahoe";
    case BinMachinePDP10:         return "PDP-10";
    case BinMachineM88K:          return "m88k";
    case BinMachineDLX:           return "DLX";
    case BinMachineMoxie:         return "Moxie";
    case BinMachineLoongArch32:   return "LoongArch32";
    case BinMachineLoongArch64:   return "LoongArch64";
    case BinMachineConvex:        return "Convex";
    case BinMachinePyramid:       return "Pyramid";
    case BinMachineCray:          return "Cray";
    case BinMachineHPFocus:       return "HP Focus";
    case BinMachineEBC:           return "EBC";
    case BinMachineI8086:         return "8086";
    case BinMachineI80286:        return "80286";
    case BinMachineAM29K:         return "AMD 29000";
    case BinMachineAVR:           return "AVR";
    case BinMachineAVR32:         return "AVR32";
    case BinMachineNios2:         return "Nios II";
    case BinMachineMicroBlaze:    return "MicroBlaze";
    case BinMachineOpenRISC:      return "OpenRISC";
    case BinMachineMSP430:        return "MSP430";
    case BinMachineLanai:         return "Lanai";
    case BinMachineElbrus2K:      return "Elbrus 2000";
    case BinMachineClipper:       return "Clipper";
    case BinMachineBPF:           return "BPF";
    case BinMachineHexagon:       return "Hexagon";
    case BinMachineCSKY:          return "C-SKY";
    case BinMachineFR30:          return "FR30";
    case BinMachineMN10200:       return "MN10200";
    case BinMachineMN10300:       return "MN10300";
    case BinMachineFRV:           return "FR-V";
    case BinMachineNECVE:         return "NEC VE";
    case BinMachineSuperH:        return "SuperH";
    case BinMachinePARISC:        return "PA-RISC";
    case BinMachineXtensa:        return "Xtensa";
    case BinMachineI860:          return "i860";
    case BinMachineI960:          return "i960";
    case BinMachineTIC54X:        return "TMS320C54x";
    case BinMachineTIC55X:        return "TMS320C55x";
    case BinMachineTIC6X:         return "TMS320C6x";
    case BinMachineBlackfin:      return "Blackfin";
    case BinMachineEpiphany:      return "Epiphany";
    case BinMachineM32R:          return "M32R";
    case BinMachineM32C:          return "M32C";
    case BinMachineV850:          return "V850";
    case BinMachineZ80:           return "Z80";
    case BinMachineZ8000:         return "Z8000";
    case BinMachineZ80000:        return "Z80000";
    case BinMachine6502:          return "6502";
    case BinMachine65816:         return "65816";
    case BinMachine65832:         return "65832";
    case BinMachineMCore:         return "MCore";
    case BinMachineTriCore:       return "TriCore";
    case BinMachineWASM:          return "WebAssembly";
    default:                      return "Unknown";
  }
}

/**
  Get human-readable name for file type.

  @param[in]   FileType          File type.

  @return Pointer to file type name string.
**/
CONST CHAR8 *
BinFormatGetFileTypeName(
  IN  BINFORMAT_FILE_TYPE  FileType
  )
{
  switch (FileType) {
    case BinFileTypeRelocatable:    return "Relocatable";
    case BinFileTypeExecutable:     return "Executable";
    case BinFileTypeSharedLibrary:  return "Shared Library";
    case BinFileTypeCore:           return "Core Dump";
    case BinFileTypeStaticLibrary:  return "Static Library";
    default:                        return "Unknown";
  }
}

/**
  Get human-readable name for symbol type.

  @param[in]   SymbolType        Symbol type.

  @return Pointer to symbol type name string.
**/
CONST CHAR8 *
BinFormatGetSymbolTypeName(
  IN  BINFORMAT_SYMBOL_TYPE  SymbolType
  )
{
  switch (SymbolType) {
    case BinSymbolTypeNone:     return "NOTYPE";
    case BinSymbolTypeObject:   return "OBJECT";
    case BinSymbolTypeFunc:     return "FUNC";
    case BinSymbolTypeSection:  return "SECTION";
    case BinSymbolTypeFile:     return "FILE";
    case BinSymbolTypeCommon:   return "COMMON";
    case BinSymbolTypeTls:      return "TLS";
    default:                    return "UNKNOWN";
  }
}

/**
  Get human-readable name for symbol binding.

  @param[in]   SymbolBind        Symbol binding.

  @return Pointer to symbol binding name string.
**/
CONST CHAR8 *
BinFormatGetSymbolBindName(
  IN  BINFORMAT_SYMBOL_BIND  SymbolBind
  )
{
  switch (SymbolBind) {
    case BinSymbolBindLocal:   return "LOCAL";
    case BinSymbolBindGlobal:  return "GLOBAL";
    case BinSymbolBindWeak:    return "WEAK";
    default:                   return "UNKNOWN";
  }
}

/**
  Get short symbol type character (for nm-style output).

  Returns characters like 'T' (text), 'D' (data), 'B' (BSS), 'U' (undefined).

  @param[in]   Symbol            Symbol descriptor.

  @return Symbol type character.
**/
CHAR8
BinFormatGetSymbolTypeChar(
  IN  CONST BINFORMAT_SYMBOL  *Symbol
  )
{
  CHAR8  TypeChar;

  //
  // Check for undefined symbol
  //
  if (Symbol->SectionIndex == 0) {
    return 'U';
  }

  //
  // Determine type based on section characteristics
  // This is a heuristic - exact behavior depends on section flags
  //
  switch (Symbol->Type) {
    case BinSymbolTypeFunc:
      TypeChar = 'T';  // Text (code)
      break;

    case BinSymbolTypeObject:
      //
      // Check if it's in BSS
      //
      if (Symbol->SectionIndex == 0xFFF1) {  // SHN_ABS in ELF
        TypeChar = 'A';  // Absolute
      } else {
        //
        // Assume data if not function
        // Would need section flags to distinguish D/B/R
        //
        TypeChar = 'D';  // Data
      }
      break;

    case BinSymbolTypeCommon:
      TypeChar = 'C';  // Common
      break;

    case BinSymbolTypeFile:
      TypeChar = 'f';  // File
      break;

    case BinSymbolTypeSection:
      TypeChar = 's';  // Section
      break;

    case BinSymbolTypeTls:
      TypeChar = 't';  // TLS
      break;

    default:
      TypeChar = '?';  // Unknown
      break;
  }

  //
  // Use lowercase for local symbols
  //
  if (Symbol->Bind == BinSymbolBindLocal && TypeChar >= 'A' && TypeChar <= 'Z') {
    TypeChar = TypeChar - 'A' + 'a';
  }

  return TypeChar;
}

/**
  Normalize architecture name to canonical form.

  Handles various naming conventions:
  - x86_64, x86-64, amd64, x64 -> x86_64
  - i386, i486, i586, i686, x86 -> i386
  - arm64, aarch64 -> arm64
  - arm, armv7, armv7l -> arm

  @param[in]   ArchName          Architecture name to normalize.

  @return Canonical architecture name, or original if not recognized.
**/
CONST CHAR8 *
BinFormatNormalizeArchName(
  IN  CONST CHAR8  *ArchName
  )
{
  if (ArchName == NULL) {
    return NULL;
  }

  //
  // x86-64 variants
  //
  if (strcasecmp (ArchName, "x86_64") == 0 ||
      strcasecmp (ArchName, "x86-64") == 0 ||
      strcasecmp (ArchName, "amd64") == 0 ||
      strcasecmp (ArchName, "x64") == 0 ||
      strcasecmp (ArchName, "ia32e") == 0) {
    return "x86_64";
  }

  //
  // i386 variants (i?86 pattern, ix86, x86, ia32)
  //
  if (strcasecmp (ArchName, "i386") == 0 ||
      strcasecmp (ArchName, "i486") == 0 ||
      strcasecmp (ArchName, "i586") == 0 ||
      strcasecmp (ArchName, "i686") == 0 ||
      strcasecmp (ArchName, "i786") == 0 ||
      strcasecmp (ArchName, "i886") == 0 ||
      strcasecmp (ArchName, "i986") == 0 ||
      strcasecmp (ArchName, "ix86") == 0 ||
      strcasecmp (ArchName, "x86") == 0 ||
      strcasecmp (ArchName, "ia32") == 0) {
    return "i386";
  }

  //
  // ARM64 variants
  //
  if (strcasecmp (ArchName, "arm64") == 0 ||
      strcasecmp (ArchName, "aarch64") == 0 ||
      strcasecmp (ArchName, "arm64v8") == 0 ||
      strcasecmp (ArchName, "armv8") == 0 ||
      strcasecmp (ArchName, "armv8a") == 0 ||
      strcasecmp (ArchName, "armv8-a") == 0 ||
      strcasecmp (ArchName, "arm64e") == 0) {
    return "arm64";
  }

  //
  // ARM 32-bit variants
  //
  if (strcasecmp (ArchName, "arm") == 0 ||
      strcasecmp (ArchName, "armv7") == 0 ||
      strcasecmp (ArchName, "armv7l") == 0 ||
      strcasecmp (ArchName, "armv7a") == 0 ||
      strcasecmp (ArchName, "armv7-a") == 0 ||
      strcasecmp (ArchName, "armv7m") == 0 ||
      strcasecmp (ArchName, "armv7-m") == 0 ||
      strcasecmp (ArchName, "armv6") == 0 ||
      strcasecmp (ArchName, "armv6l") == 0 ||
      strcasecmp (ArchName, "armv5") == 0 ||
      strcasecmp (ArchName, "armv5t") == 0 ||
      strcasecmp (ArchName, "armv5te") == 0 ||
      strcasecmp (ArchName, "armv5tej") == 0 ||
      strcasecmp (ArchName, "armv4") == 0 ||
      strcasecmp (ArchName, "armv4t") == 0 ||
      strcasecmp (ArchName, "armhf") == 0 ||
      strcasecmp (ArchName, "armel") == 0) {
    return "arm";
  }

  //
  // PowerPC 32-bit variants
  //
  if (strcasecmp (ArchName, "ppc") == 0 ||
      strcasecmp (ArchName, "powerpc") == 0 ||
      strcasecmp (ArchName, "ppc32") == 0 ||
      strcasecmp (ArchName, "powerpc32") == 0) {
    return "ppc";
  }

  //
  // PowerPC 64-bit variants (big endian)
  //
  if (strcasecmp (ArchName, "ppc64") == 0 ||
      strcasecmp (ArchName, "powerpc64") == 0) {
    return "ppc64";
  }

  //
  // PowerPC 64-bit little endian
  //
  if (strcasecmp (ArchName, "ppc64le") == 0 ||
      strcasecmp (ArchName, "powerpc64le") == 0 ||
      strcasecmp (ArchName, "ppc64el") == 0) {
    return "ppc64le";
  }

  //
  // MIPS 32-bit variants
  //
  if (strcasecmp (ArchName, "mips") == 0 ||
      strcasecmp (ArchName, "mipsel") == 0 ||
      strcasecmp (ArchName, "mipseb") == 0 ||
      strcasecmp (ArchName, "mips32") == 0 ||
      strcasecmp (ArchName, "mips32el") == 0) {
    return "mips";
  }

  //
  // MIPS 64-bit variants
  //
  if (strcasecmp (ArchName, "mips64") == 0 ||
      strcasecmp (ArchName, "mips64el") == 0 ||
      strcasecmp (ArchName, "mips64r6") == 0 ||
      strcasecmp (ArchName, "mipsn32") == 0) {
    return "mips64";
  }

  //
  // RISC-V 64-bit variants
  //
  if (strcasecmp (ArchName, "riscv64") == 0 ||
      strcasecmp (ArchName, "riscv") == 0 ||
      strcasecmp (ArchName, "rv64") == 0 ||
      strcasecmp (ArchName, "riscv64gc") == 0) {
    return "riscv64";
  }

  //
  // RISC-V 32-bit variants
  //
  if (strcasecmp (ArchName, "riscv32") == 0 ||
      strcasecmp (ArchName, "rv32") == 0 ||
      strcasecmp (ArchName, "riscv32gc") == 0) {
    return "riscv32";
  }

  //
  // SPARC 32-bit variants
  //
  if (strcasecmp (ArchName, "sparc") == 0 ||
      strcasecmp (ArchName, "sparcv8") == 0 ||
      strcasecmp (ArchName, "sparcv7") == 0) {
    return "sparc";
  }

  //
  // SPARC 64-bit variants
  //
  if (strcasecmp (ArchName, "sparc64") == 0 ||
      strcasecmp (ArchName, "sparcv9") == 0 ||
      strcasecmp (ArchName, "ultrasparc") == 0) {
    return "sparc64";
  }

  //
  // Alpha variants
  //
  if (strcasecmp (ArchName, "alpha") == 0 ||
      strcasecmp (ArchName, "alphaev56") == 0 ||
      strcasecmp (ArchName, "alphaev6") == 0 ||
      strcasecmp (ArchName, "alphaev67") == 0 ||
      strcasecmp (ArchName, "alphaev68") == 0 ||
      strcasecmp (ArchName, "alphaev7") == 0) {
    return "alpha";
  }

  //
  // M68K variants
  //
  if (strcasecmp (ArchName, "m68k") == 0 ||
      strcasecmp (ArchName, "m68000") == 0 ||
      strcasecmp (ArchName, "68000") == 0 ||
      strcasecmp (ArchName, "68k") == 0) {
    return "m68k";
  }

  //
  // S390 variants
  //
  if (strcasecmp (ArchName, "s390") == 0) {
    return "s390";
  }

  if (strcasecmp (ArchName, "s390x") == 0) {
    return "s390x";
  }

  //
  // HPPA/PA-RISC variants
  //
  if (strcasecmp (ArchName, "hppa") == 0 ||
      strcasecmp (ArchName, "parisc") == 0) {
    return "hppa";
  }

  if (strcasecmp (ArchName, "hppa64") == 0 ||
      strcasecmp (ArchName, "parisc64") == 0) {
    return "hppa64";
  }

  //
  // IA-64 variants
  //
  if (strcasecmp (ArchName, "ia64") == 0 ||
      strcasecmp (ArchName, "itanium") == 0) {
    return "ia64";
  }

  //
  // SuperH variants
  //
  if (strcasecmp (ArchName, "sh") == 0 ||
      strcasecmp (ArchName, "sh4") == 0 ||
      strcasecmp (ArchName, "sh4a") == 0 ||
      strcasecmp (ArchName, "sh3") == 0 ||
      strcasecmp (ArchName, "sh2") == 0) {
    return "sh";
  }

  //
  // LoongArch variants
  //
  if (strcasecmp (ArchName, "loongarch64") == 0 ||
      strcasecmp (ArchName, "loong64") == 0) {
    return "loongarch64";
  }

  if (strcasecmp (ArchName, "loongarch32") == 0 ||
      strcasecmp (ArchName, "loong32") == 0) {
    return "loongarch32";
  }

  //
  // Return original if not recognized
  //
  return ArchName;
}

/**
  Compare two architecture names for equivalence.

  Handles naming variations (e.g., x86_64 == x86-64 == amd64).

  @param[in]   Arch1             First architecture name.
  @param[in]   Arch2             Second architecture name.

  @retval TRUE   Architecture names are equivalent.
  @retval FALSE  Architecture names are different.
**/
BOOLEAN
BinFormatArchNamesMatch(
  IN  CONST CHAR8  *Arch1,
  IN  CONST CHAR8  *Arch2
  )
{
  CONST CHAR8  *Normalized1;
  CONST CHAR8  *Normalized2;

  if (Arch1 == NULL || Arch2 == NULL) {
    return FALSE;
  }

  //
  // Normalize both names and compare
  //
  Normalized1 = BinFormatNormalizeArchName (Arch1);
  Normalized2 = BinFormatNormalizeArchName (Arch2);

  return (strcmp (Normalized1, Normalized2) == 0);
}

/**
  Extract thin slice from fat binary directly to file descriptor using splice.

  This provides zero-copy extraction on Linux using the splice() system call.
  On other platforms, falls back to read+write.

  This is backend-agnostic - it works with any format that supports fat binaries
  by using the BINFORMAT_ARCHITECTURE offset/size information.

  @param[in]   Api               Binary format API.
  @param[in]   Context           Fat binary context.
  @param[in]   ArchIndex         Index of architecture to extract.
  @param[in]   SourceFd          Source file descriptor (original fat binary).
  @param[in]   DestFd            Destination file descriptor (must be writable).

  @retval BINFORMAT_SUCCESS      Thin slice extracted successfully.
  @retval BINFORMAT_ERROR_*      Error occurred.
**/
BINFORMAT_STATUS
BinFormatExtractThinToFd(
  IN  CONST BINFORMAT_API  *Api,
  IN  BINFORMAT_CONTEXT    *Context,
  IN  UINT32               ArchIndex,
  IN  INT32                SourceFd,
  IN  INT32                DestFd
  )
{
  BINFORMAT_STATUS       Status;
  BINFORMAT_HEADER_INFO  HeaderInfo;
  UINT64                 Offset;
  UINT64                 Size;
  UINT64                 Remaining;

  if (Api == NULL || Context == NULL || SourceFd < 0 || DestFd < 0) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Get header info to access architecture list
  //
  Status = Api->GetHeader (Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR (Status)) {
    return Status;
  }

  //
  // Validate architecture index
  //
  if (ArchIndex >= HeaderInfo.ArchitectureCount) {
    return BINFORMAT_ERROR_INVALID_PARAMETER;
  }

  //
  // Get offset and size for the requested architecture
  //
  Offset = HeaderInfo.Architectures[ArchIndex].Offset;
  Size = HeaderInfo.Architectures[ArchIndex].Size;

#ifdef __linux__
  //
  // Linux: use splice() for zero-copy extraction
  //
  {
    off64_t  SpliceOffset;
    ssize_t  BytesSpliced;

    SpliceOffset = (off64_t)Offset;
    Remaining = Size;

    while (Remaining > 0) {
      //
      // splice() can transfer up to 2GB at a time on most systems
      //
      size_t ChunkSize = (Remaining > 0x7FFFF000) ? 0x7FFFF000 : (size_t)Remaining;

      BytesSpliced = splice (
                       SourceFd,
                       &SpliceOffset,
                       DestFd,
                       NULL,
                       ChunkSize,
                       0
                       );

      if (BytesSpliced < 0) {
        return BINFORMAT_ERROR_IO;
      }

      if (BytesSpliced == 0) {
        //
        // Unexpected EOF
        //
        return BINFORMAT_ERROR_INVALID_FORMAT;
      }

      Remaining -= (UINT64)BytesSpliced;
    }
  }
#else
  //
  // Other platforms: use lseek + read + write
  //
  {
    UINT8    *Buffer;
    size_t   ChunkSize;
    ssize_t  BytesRead;
    ssize_t  BytesWritten;

    //
    // Seek to the slice offset in the source file
    //
    if (lseek (SourceFd, (off_t)Offset, SEEK_SET) < 0) {
      return BINFORMAT_ERROR_IO;
    }

    //
    // Allocate buffer for copying (use 1MB chunks)
    //
    ChunkSize = 1024 * 1024;
    if ((UINT64)ChunkSize > Size) {
      ChunkSize = (size_t)Size;
    }

    Buffer = (UINT8 *)malloc (ChunkSize);
    if (Buffer == NULL) {
      return BINFORMAT_ERROR_OUT_OF_MEMORY;
    }

    //
    // Copy in chunks
    //
    Remaining = Size;
    while (Remaining > 0) {
      size_t ToRead = (Remaining > ChunkSize) ? ChunkSize : (size_t)Remaining;

      BytesRead = read (SourceFd, Buffer, ToRead);
      if (BytesRead < 0) {
        free (Buffer);
        return BINFORMAT_ERROR_IO;
      }

      if (BytesRead == 0) {
        //
        // Unexpected EOF
        //
        free (Buffer);
        return BINFORMAT_ERROR_INVALID_FORMAT;
      }

      BytesWritten = write (DestFd, Buffer, (size_t)BytesRead);
      if (BytesWritten != BytesRead) {
        free (Buffer);
        return BINFORMAT_ERROR_IO;
      }

      Remaining -= (UINT64)BytesRead;
    }

    free (Buffer);
  }
#endif

  //
  // Reset destination fd to beginning
  //
  lseek (DestFd, 0, SEEK_SET);

  return BINFORMAT_SUCCESS;
}
