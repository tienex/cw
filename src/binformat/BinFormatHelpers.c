/** @file
  Binary Format Helper Functions Implementation.

  This file implements helper functions and default implementations that
  tool developers can use without reinventing the wheel.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    case BinMachineWebAssembly:   return "WebAssembly";
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
