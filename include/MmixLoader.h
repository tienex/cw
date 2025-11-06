/** @file
  MMIX Binary loader interface.

  This file defines the interface for loading various binary formats
  including ELF, Mach-O, PE/COFF, and MMO into the MMIX emulator.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_LOADER_H__
#define __MMIX_LOADER_H__

#include "MmixTypes.h"
#include "MmixEmulator.h"

//
// Binary format types
//

typedef enum {
  MmixBinaryFormatUnknown = 0,
  MmixBinaryFormatRaw = 1,      ///< Raw binary
  MmixBinaryFormatElf32 = 2,    ///< ELF 32-bit
  MmixBinaryFormatElf64 = 3,    ///< ELF 64-bit
  MmixBinaryFormatMacho32 = 4,  ///< Mach-O 32-bit
  MmixBinaryFormatMacho64 = 5,  ///< Mach-O 64-bit
  MmixBinaryFormatPe32 = 6,     ///< PE32
  MmixBinaryFormatPe64 = 7,     ///< PE32+
  MmixBinaryFormatMmo = 8       ///< MMIX Object format
} MMIX_BINARY_FORMAT;

//
// Binary information structure
//

typedef struct {
  ///
  /// Detected format
  ///
  MMIX_BINARY_FORMAT  Format;

  ///
  /// Entry point address
  ///
  UINT64              EntryPoint;

  ///
  /// Endianness (big or little)
  ///
  MMIX_ENDIAN_MODE    Endianness;

  ///
  /// Architecture (32-bit or 64-bit)
  ///
  UINT8               Bits;

  ///
  /// Total loaded size
  ///
  UINT64              LoadedSize;

  ///
  /// Base address
  ///
  UINT64              BaseAddress;
} MMIX_BINARY_INFO;

//
// Loader functions
//

/**
  Detect binary format from file.

  Reads the file header to determine the binary format.

  @param[in]   FilePath         Path to binary file.
  @param[out]  Format           Pointer to receive format.

  @retval MMIX_SUCCESS          Format detected successfully.
  @retval MMIX_ERROR_NOT_FOUND  File not found.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderDetectFormat (
  IN  CONST CHAR8         *FilePath,
  OUT MMIX_BINARY_FORMAT  *Format
  );

/**
  Load binary into emulator memory.

  Loads a binary file in any supported format into emulator memory,
  parsing headers and setting up the environment appropriately.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to binary file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          Binary loaded successfully.
  @retval MMIX_ERROR_NOT_FOUND  File not found.
  @retval MMIX_ERROR_UNSUPPORTED  Unsupported format.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadBinary (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  );

/**
  Load ELF binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to ELF file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          ELF loaded successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadElf (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  );

/**
  Load Mach-O binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to Mach-O file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          Mach-O loaded successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadMacho (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  );

/**
  Load PE/COFF binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to PE file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          PE loaded successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadPe (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  );

/**
  Load MMO (MMIX Object) binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to MMO file.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          MMO loaded successfully.
  @retval Others                Error occurred.

**/
MMIX_STATUS
MmixLoaderLoadMmo (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  OUT    MMIX_BINARY_INFO       *Info OPTIONAL
  );

/**
  Get format name string.

  @param[in]  Format            Binary format.

  @return String name of format.

**/
CONST CHAR8 *
MmixLoaderGetFormatName (
  IN MMIX_BINARY_FORMAT  Format
  );

#endif // __MMIX_LOADER_H__
