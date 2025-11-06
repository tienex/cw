/** @file
  PE/COFF binary loader implementation.

  This file implements loading of PE (Portable Executable) / COFF
  (Common Object File Format) binaries used by Windows, supporting
  both PE32 and PE32+ (64-bit) variants.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/MmixLoader.h"

//
// PE structures
//

#pragma pack(push, 1)

typedef struct {
  UINT16  e_magic;      // MZ signature
  UINT16  e_cblp;
  UINT16  e_cp;
  UINT16  e_crlc;
  UINT16  e_cparhdr;
  UINT16  e_minalloc;
  UINT16  e_maxalloc;
  UINT16  e_ss;
  UINT16  e_sp;
  UINT16  e_csum;
  UINT16  e_ip;
  UINT16  e_cs;
  UINT16  e_lfarlc;
  UINT16  e_ovno;
  UINT16  e_res[4];
  UINT16  e_oemid;
  UINT16  e_oeminfo;
  UINT16  e_res2[10];
  UINT32  e_lfanew;     // PE header offset
} IMAGE_DOS_HEADER;

typedef struct {
  UINT32  Signature;    // PE\0\0
  UINT16  Machine;
  UINT16  NumberOfSections;
  UINT32  TimeDateStamp;
  UINT32  PointerToSymbolTable;
  UINT32  NumberOfSymbols;
  UINT16  SizeOfOptionalHeader;
  UINT16  Characteristics;
} IMAGE_FILE_HEADER;

typedef struct {
  UINT16  Magic;        // 0x10B for PE32, 0x20B for PE32+
  UINT8   MajorLinkerVersion;
  UINT8   MinorLinkerVersion;
  UINT32  SizeOfCode;
  UINT32  SizeOfInitializedData;
  UINT32  SizeOfUninitializedData;
  UINT32  AddressOfEntryPoint;
  UINT32  BaseOfCode;
  UINT32  BaseOfData;   // Not in PE32+
  UINT32  ImageBase;
  UINT32  SectionAlignment;
  UINT32  FileAlignment;
  UINT16  MajorOperatingSystemVersion;
  UINT16  MinorOperatingSystemVersion;
  UINT16  MajorImageVersion;
  UINT16  MinorImageVersion;
  UINT16  MajorSubsystemVersion;
  UINT16  MinorSubsystemVersion;
  UINT32  Win32VersionValue;
  UINT32  SizeOfImage;
  UINT32  SizeOfHeaders;
  UINT32  CheckSum;
  UINT16  Subsystem;
  UINT16  DllCharacteristics;
  UINT32  SizeOfStackReserve;
  UINT32  SizeOfStackCommit;
  UINT32  SizeOfHeapReserve;
  UINT32  SizeOfHeapCommit;
  UINT32  LoaderFlags;
  UINT32  NumberOfRvaAndSizes;
} IMAGE_OPTIONAL_HEADER32;

typedef struct {
  UINT16  Magic;
  UINT8   MajorLinkerVersion;
  UINT8   MinorLinkerVersion;
  UINT32  SizeOfCode;
  UINT32  SizeOfInitializedData;
  UINT32  SizeOfUninitializedData;
  UINT32  AddressOfEntryPoint;
  UINT32  BaseOfCode;
  UINT64  ImageBase;
  UINT32  SectionAlignment;
  UINT32  FileAlignment;
  UINT16  MajorOperatingSystemVersion;
  UINT16  MinorOperatingSystemVersion;
  UINT16  MajorImageVersion;
  UINT16  MinorImageVersion;
  UINT16  MajorSubsystemVersion;
  UINT16  MinorSubsystemVersion;
  UINT32  Win32VersionValue;
  UINT32  SizeOfImage;
  UINT32  SizeOfHeaders;
  UINT32  CheckSum;
  UINT16  Subsystem;
  UINT16  DllCharacteristics;
  UINT64  SizeOfStackReserve;
  UINT64  SizeOfStackCommit;
  UINT64  SizeOfHeapReserve;
  UINT64  SizeOfHeapCommit;
  UINT32  LoaderFlags;
  UINT32  NumberOfRvaAndSizes;
} IMAGE_OPTIONAL_HEADER64;

typedef struct {
  CHAR8   Name[8];
  UINT32  VirtualSize;
  UINT32  VirtualAddress;
  UINT32  SizeOfRawData;
  UINT32  PointerToRawData;
  UINT32  PointerToRelocations;
  UINT32  PointerToLinenumbers;
  UINT16  NumberOfRelocations;
  UINT16  NumberOfLinenumbers;
  UINT32  Characteristics;
} IMAGE_SECTION_HEADER;

#pragma pack(pop)

#define IMAGE_NT_SIGNATURE  0x00004550  // "PE\0\0"

/**
  Load PE binary.

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
  )
{
  FILE                    *File;
  IMAGE_DOS_HEADER        DosHeader;
  IMAGE_FILE_HEADER       FileHeader;
  IMAGE_OPTIONAL_HEADER32 OptHeader32;
  IMAGE_OPTIONAL_HEADER64 OptHeader64;
  IMAGE_SECTION_HEADER    SecHeader;
  UINT32                  PeSignature;
  UINT16                  Magic;
  UINT32                  SectionOffset;
  UINT16                  i;
  BOOLEAN                 Is64Bit;
  MMIX_BINARY_INFO        LocalInfo;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file
  //
  File = fopen (FilePath, "rb");
  if (File == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Read DOS header
  //
  if (fread (&DosHeader, 1, sizeof (IMAGE_DOS_HEADER), File) != sizeof (IMAGE_DOS_HEADER)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Check MZ signature
  //
  if (DosHeader.e_magic != 0x5A4D) {
    fclose (File);
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Seek to PE header
  //
  fseek (File, DosHeader.e_lfanew, SEEK_SET);

  //
  // Read PE signature
  //
  if (fread (&PeSignature, 1, sizeof (UINT32), File) != sizeof (UINT32)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  if (PeSignature != IMAGE_NT_SIGNATURE) {
    fclose (File);
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Read file header
  //
  if (fread (&FileHeader, 1, sizeof (IMAGE_FILE_HEADER), File) != sizeof (IMAGE_FILE_HEADER)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Read optional header magic to determine 32/64-bit
  //
  if (fread (&Magic, 1, sizeof (UINT16), File) != sizeof (UINT16)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  Is64Bit = (Magic == 0x20B);

  //
  // Initialize info
  //
  memset (&LocalInfo, 0, sizeof (MMIX_BINARY_INFO));
  LocalInfo.Format = Is64Bit ? MmixBinaryFormatPe64 : MmixBinaryFormatPe32;
  LocalInfo.Endianness = MmixEndianLittle;  // PE is always little-endian
  LocalInfo.Bits = Is64Bit ? 64 : 32;

  //
  // Read full optional header
  //
  fseek (File, DosHeader.e_lfanew + sizeof (UINT32) + sizeof (IMAGE_FILE_HEADER), SEEK_SET);

  if (Is64Bit) {
    if (fread (&OptHeader64, 1, sizeof (IMAGE_OPTIONAL_HEADER64), File) != sizeof (IMAGE_OPTIONAL_HEADER64)) {
      fclose (File);
      return MMIX_ERROR_DEVICE_ERROR;
    }
    LocalInfo.EntryPoint = OptHeader64.ImageBase + OptHeader64.AddressOfEntryPoint;
    LocalInfo.BaseAddress = OptHeader64.ImageBase;
  } else {
    if (fread (&OptHeader32, 1, sizeof (IMAGE_OPTIONAL_HEADER32), File) != sizeof (IMAGE_OPTIONAL_HEADER32)) {
      fclose (File);
      return MMIX_ERROR_DEVICE_ERROR;
    }
    LocalInfo.EntryPoint = OptHeader32.ImageBase + OptHeader32.AddressOfEntryPoint;
    LocalInfo.BaseAddress = OptHeader32.ImageBase;
  }

  //
  // Calculate section header offset
  //
  SectionOffset = DosHeader.e_lfanew + sizeof (UINT32) + sizeof (IMAGE_FILE_HEADER) + FileHeader.SizeOfOptionalHeader;

  //
  // Load sections
  //
  for (i = 0; i < FileHeader.NumberOfSections; i++) {
    UINT8  *SectionData;

    fseek (File, SectionOffset + i * sizeof (IMAGE_SECTION_HEADER), SEEK_SET);
    if (fread (&SecHeader, 1, sizeof (IMAGE_SECTION_HEADER), File) != sizeof (IMAGE_SECTION_HEADER)) {
      continue;
    }

    //
    // Load section data
    //
    if (SecHeader.SizeOfRawData > 0) {
      SectionData = (UINT8 *)malloc (SecHeader.SizeOfRawData);
      if (SectionData != NULL) {
        fseek (File, SecHeader.PointerToRawData, SEEK_SET);
        if (fread (SectionData, 1, SecHeader.SizeOfRawData, File) == SecHeader.SizeOfRawData) {
          UINT64  VirtAddr = LocalInfo.BaseAddress + SecHeader.VirtualAddress;
          if (VirtAddr + SecHeader.SizeOfRawData <= Context->MemoryState->PhysicalMemorySize) {
            memcpy (&Context->MemoryState->PhysicalMemory[VirtAddr], SectionData, SecHeader.SizeOfRawData);
            LocalInfo.LoadedSize += SecHeader.SizeOfRawData;
          }
        }
        free (SectionData);
      }
    }
  }

  fclose (File);

  //
  // Set entry point
  //
  Context->CpuState->Pc = LocalInfo.EntryPoint;

  //
  // Copy info if requested
  //
  if (Info != NULL) {
    memcpy (Info, &LocalInfo, sizeof (MMIX_BINARY_INFO));
  }

  return MMIX_SUCCESS;
}
