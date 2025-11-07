/** @file
  Binary Format Libraries Test Program.

  This program tests all binary format libraries (libelf, libcoff, libaout,
  libmacho, libomf) using the unified BINFORMAT_API interface.

  Copyright (c) 2025. All rights reserved.

  SPDX-License-Identifier: MIT

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/binformat/BinFormat.h"

/**
  Test a binary format library.

  @param[in]  Api       Binary format API table.
  @param[in]  FilePath  Path to test file.

  @retval 0   Success.
  @retval 1   Failure.

**/
STATIC
INT32
TestBinaryFormat (
  IN  CONST BINFORMAT_API  *Api,
  IN  CONST CHAR8          *FilePath
  )
{
  BINFORMAT_CONTEXT      *Context;
  BINFORMAT_HEADER_INFO  HeaderInfo;
  BINFORMAT_STATUS       Status;

  printf("Testing %s with file: %s\n", Api->LibraryName, FilePath);

  //
  // Initialize from file
  //
  Status = Api->InitFile(&Context, FilePath, TRUE);
  if (BINFORMAT_IS_ERROR(Status)) {
    printf("  ERROR: Failed to initialize context (status: 0x%llX)\n",
           (unsigned long long)Status);
    return 1;
  }

  printf("  ✓ Context initialized successfully\n");

  //
  // Get header information
  //
  Status = Api->GetHeader(Context, &HeaderInfo);
  if (BINFORMAT_IS_ERROR(Status)) {
    printf("  ERROR: Failed to get header (status: 0x%llX)\n",
           (unsigned long long)Status);
    Api->Close(Context);
    return 1;
  }

  printf("  ✓ Header retrieved successfully\n");
  printf("    - File Type: %d\n", HeaderInfo.FileType);
  printf("    - Machine: %d\n", HeaderInfo.Machine);
  printf("    - Is 64-bit: %s\n", HeaderInfo.Is64Bit ? "Yes" : "No");
  printf("    - Entry Point: 0x%llX\n", (unsigned long long)HeaderInfo.EntryPoint);
  printf("    - Sections: %u\n", HeaderInfo.SectionCount);
  printf("    - Segments: %u\n", HeaderInfo.SegmentCount);
  printf("    - Symbols: %u\n", HeaderInfo.SymbolCount);

  if (HeaderInfo.ArchitectureCount > 0) {
    printf("    - Architectures: %u (Multi-arch binary)\n",
           HeaderInfo.ArchitectureCount);
    for (UINT32 i = 0; i < HeaderInfo.ArchitectureCount; i++) {
      printf("      [%u] Machine=%d, Offset=0x%llX, Size=%llu\n",
             i,
             HeaderInfo.Architectures[i].Machine,
             (unsigned long long)HeaderInfo.Architectures[i].Offset,
             (unsigned long long)HeaderInfo.Architectures[i].Size);
    }
  }

  //
  // Close context
  //
  Api->Close(Context);
  printf("  ✓ Context closed successfully\n\n");

  return 0;
}

/**
  Main entry point.

  @param[in]  argc  Argument count.
  @param[in]  argv  Argument vector.

  @retval 0   Success.
  @retval 1   Failure.

**/
INT32
main (
  IN  INT32   argc,
  IN  CHAR8   **argv
  )
{
  INT32  Result;

  printf("=================================================\n");
  printf("Binary Format Libraries Test Suite\n");
  printf("=================================================\n\n");

  printf("Library APIs initialized:\n");
  printf("  - %s v%u\n", ElfGetApi()->LibraryName, ElfGetApi()->Version);
  printf("  - %s v%u\n", CoffGetApi()->LibraryName, CoffGetApi()->Version);
  printf("  - %s v%u\n", AoutGetApi()->LibraryName, AoutGetApi()->Version);
  printf("  - %s v%u\n", MachoGetApi()->LibraryName, MachoGetApi()->Version);
  printf("  - %s v%u\n", OmfGetApi()->LibraryName, OmfGetApi()->Version);
  printf("  - %s v%u\n", OrfGetApi()->LibraryName, OrfGetApi()->Version);
  printf("\n");

  //
  // If file paths are provided, test them
  //
  if (argc > 1) {
    for (INT32 i = 1; i < argc; i++) {
      printf("=================================================\n");
      printf("Testing file: %s\n", argv[i]);
      printf("=================================================\n\n");

      //
      // Try each library to see which one can handle the file
      //
      printf("Attempting with libelf...\n");
      Result = TestBinaryFormat(ElfGetApi(), argv[i]);

      if (Result != 0) {
        printf("Attempting with libcoff...\n");
        Result = TestBinaryFormat(CoffGetApi(), argv[i]);
      }

      if (Result != 0) {
        printf("Attempting with libaout...\n");
        Result = TestBinaryFormat(AoutGetApi(), argv[i]);
      }

      if (Result != 0) {
        printf("Attempting with libmacho...\n");
        Result = TestBinaryFormat(MachoGetApi(), argv[i]);
      }

      if (Result != 0) {
        printf("Attempting with libomf...\n");
        Result = TestBinaryFormat(OmfGetApi(), argv[i]);
      }

      if (Result != 0) {
        printf("Attempting with liborf...\n");
        Result = TestBinaryFormat(OrfGetApi(), argv[i]);
      }

      if (Result != 0) {
        printf("ERROR: No library could handle this file\n\n");
      }
    }
  } else {
    printf("Usage: %s <binary_file1> [binary_file2] ...\n", argv[0]);
    printf("\nExample:\n");
    printf("  %s /bin/ls /usr/bin/gcc\n", argv[0]);
    printf("\n");
  }

  printf("=================================================\n");
  printf("Test suite completed\n");
  printf("=================================================\n");

  return 0;
}
