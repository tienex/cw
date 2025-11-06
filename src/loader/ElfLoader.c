/** @file
  ELF binary loader implementation.

  This file implements loading of ELF (Executable and Linkable Format)
  binaries in both 32-bit and 64-bit variants, with support for both
  little-endian and big-endian formats.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/MmixLoader.h"
#include "../../include/MmixMemory.h"

//
// ELF identification indices
//

#define EI_MAG0       0  // File identification
#define EI_MAG1       1  // File identification
#define EI_MAG2       2  // File identification
#define EI_MAG3       3  // File identification
#define EI_CLASS      4  // File class (32/64-bit)
#define EI_DATA       5  // Data encoding (endianness)
#define EI_VERSION    6  // File version
#define EI_OSABI      7  // OS/ABI identification
#define EI_ABIVERSION 8  // ABI version
#define EI_NIDENT     16 // Size of e_ident[]

//
// ELF class
//

#define ELFCLASS32    1  // 32-bit objects
#define ELFCLASS64    2  // 64-bit objects

//
// ELF data encoding
//

#define ELFDATA2LSB   1  // Little-endian
#define ELFDATA2MSB   2  // Big-endian

//
// ELF program header types
//

#define PT_NULL       0  // Unused entry
#define PT_LOAD       1  // Loadable segment
#define PT_DYNAMIC    2  // Dynamic linking information
#define PT_INTERP     3  // Interpreter path
#define PT_NOTE       4  // Auxiliary information
#define PT_SHLIB      5  // Reserved
#define PT_PHDR       6  // Program header table
#define PT_TLS        7  // Thread-Local Storage

//
// ELF32 structures
//

#pragma pack(push, 1)

typedef struct {
  UINT8   e_ident[EI_NIDENT];  // ELF identification
  UINT16  e_type;              // Object file type
  UINT16  e_machine;           // Machine type
  UINT32  e_version;           // Object file version
  UINT32  e_entry;             // Entry point address
  UINT32  e_phoff;             // Program header offset
  UINT32  e_shoff;             // Section header offset
  UINT32  e_flags;             // Processor-specific flags
  UINT16  e_ehsize;            // ELF header size
  UINT16  e_phentsize;         // Program header entry size
  UINT16  e_phnum;             // Program header entry count
  UINT16  e_shentsize;         // Section header entry size
  UINT16  e_shnum;             // Section header entry count
  UINT16  e_shstrndx;          // Section name string table index
} Elf32_Ehdr;

typedef struct {
  UINT32  p_type;              // Segment type
  UINT32  p_offset;            // Segment offset in file
  UINT32  p_vaddr;             // Virtual address
  UINT32  p_paddr;             // Physical address
  UINT32  p_filesz;            // Size in file
  UINT32  p_memsz;             // Size in memory
  UINT32  p_flags;             // Segment flags
  UINT32  p_align;             // Alignment
} Elf32_Phdr;

//
// ELF64 structures
//

typedef struct {
  UINT8   e_ident[EI_NIDENT];  // ELF identification
  UINT16  e_type;              // Object file type
  UINT16  e_machine;           // Machine type
  UINT32  e_version;           // Object file version
  UINT64  e_entry;             // Entry point address
  UINT64  e_phoff;             // Program header offset
  UINT64  e_shoff;             // Section header offset
  UINT32  e_flags;             // Processor-specific flags
  UINT16  e_ehsize;            // ELF header size
  UINT16  e_phentsize;         // Program header entry size
  UINT16  e_phnum;             // Program header entry count
  UINT16  e_shentsize;         // Section header entry size
  UINT16  e_shnum;             // Section header entry count
  UINT16  e_shstrndx;          // Section name string table index
} Elf64_Ehdr;

typedef struct {
  UINT32  p_type;              // Segment type
  UINT32  p_flags;             // Segment flags
  UINT64  p_offset;            // Segment offset in file
  UINT64  p_vaddr;             // Virtual address
  UINT64  p_paddr;             // Physical address
  UINT64  p_filesz;            // Size in file
  UINT64  p_memsz;             // Size in memory
  UINT64  p_align;             // Alignment
} Elf64_Phdr;

#pragma pack(pop)

/**
  Convert 16-bit value based on endianness.

  @param[in]  Value             Value to convert.
  @param[in]  LittleEndian      TRUE if little-endian.

  @return Converted value.

**/
STATIC
UINT16
ElfConvert16 (
  IN UINT16   Value,
  IN BOOLEAN  LittleEndian
  )
{
  if (LittleEndian) {
    return Value;  // Host is little-endian
  } else {
    return ((Value & 0xFF) << 8) | ((Value >> 8) & 0xFF);
  }
}

/**
  Convert 32-bit value based on endianness.

  @param[in]  Value             Value to convert.
  @param[in]  LittleEndian      TRUE if little-endian.

  @return Converted value.

**/
STATIC
UINT32
ElfConvert32 (
  IN UINT32   Value,
  IN BOOLEAN  LittleEndian
  )
{
  if (LittleEndian) {
    return Value;
  } else {
    return ((Value & 0xFF) << 24) |
           (((Value >> 8) & 0xFF) << 16) |
           (((Value >> 16) & 0xFF) << 8) |
           ((Value >> 24) & 0xFF);
  }
}

/**
  Convert 64-bit value based on endianness.

  @param[in]  Value             Value to convert.
  @param[in]  LittleEndian      TRUE if little-endian.

  @return Converted value.

**/
STATIC
UINT64
ElfConvert64 (
  IN UINT64   Value,
  IN BOOLEAN  LittleEndian
  )
{
  if (LittleEndian) {
    return Value;
  } else {
    return ((Value & 0xFF) << 56) |
           (((Value >> 8) & 0xFF) << 48) |
           (((Value >> 16) & 0xFF) << 40) |
           (((Value >> 24) & 0xFF) << 32) |
           (((Value >> 32) & 0xFF) << 24) |
           (((Value >> 40) & 0xFF) << 16) |
           (((Value >> 48) & 0xFF) << 8) |
           ((Value >> 56) & 0xFF);
  }
}

/**
  Load ELF32 binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to ELF file.
  @param[in]      LittleEndian  TRUE if little-endian.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          ELF loaded successfully.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
LoadElf32 (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  IN     BOOLEAN                LittleEndian,
  OUT    MMIX_BINARY_INFO       *Info
  )
{
  FILE         *File;
  Elf32_Ehdr   Ehdr;
  Elf32_Phdr   Phdr;
  UINT8        *SegmentData;
  UINT16       i;
  size_t       BytesRead;

  //
  // Open file
  //
  File = fopen (FilePath, "rb");
  if (File == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Read ELF header
  //
  BytesRead = fread (&Ehdr, 1, sizeof (Elf32_Ehdr), File);
  if (BytesRead != sizeof (Elf32_Ehdr)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Get entry point
  //
  Info->EntryPoint = ElfConvert32 (Ehdr.e_entry, LittleEndian);
  Info->Endianness = LittleEndian ? MmixEndianLittle : MmixEndianBig;
  Info->Bits = 32;

  //
  // Load program headers
  //
  for (i = 0; i < ElfConvert16 (Ehdr.e_phnum, LittleEndian); i++) {
    //
    // Seek to program header
    //
    fseek (File, ElfConvert32 (Ehdr.e_phoff, LittleEndian) + i * sizeof (Elf32_Phdr), SEEK_SET);

    //
    // Read program header
    //
    BytesRead = fread (&Phdr, 1, sizeof (Elf32_Phdr), File);
    if (BytesRead != sizeof (Elf32_Phdr)) {
      continue;
    }

    //
    // Only load PT_LOAD segments
    //
    if (ElfConvert32 (Phdr.p_type, LittleEndian) != PT_LOAD) {
      continue;
    }

    //
    // Get segment properties
    //
    UINT32  FileOffset = ElfConvert32 (Phdr.p_offset, LittleEndian);
    UINT32  VirtAddr = ElfConvert32 (Phdr.p_vaddr, LittleEndian);
    UINT32  FileSize = ElfConvert32 (Phdr.p_filesz, LittleEndian);
    UINT32  MemSize = ElfConvert32 (Phdr.p_memsz, LittleEndian);

    //
    // Allocate buffer for segment
    //
    SegmentData = (UINT8 *)malloc (FileSize);
    if (SegmentData == NULL) {
      fclose (File);
      return MMIX_ERROR_OUT_OF_MEMORY;
    }

    //
    // Read segment data
    //
    fseek (File, FileOffset, SEEK_SET);
    BytesRead = fread (SegmentData, 1, FileSize, File);
    if (BytesRead != FileSize) {
      free (SegmentData);
      continue;
    }

    //
    // Write to emulator memory
    //
    if (VirtAddr + FileSize <= Context->MemoryState->PhysicalMemorySize) {
      memcpy (&Context->MemoryState->PhysicalMemory[VirtAddr], SegmentData, FileSize);

      //
      // Zero-fill remaining space
      //
      if (MemSize > FileSize) {
        UINT32  ZeroSize = MemSize - FileSize;
        if (VirtAddr + MemSize <= Context->MemoryState->PhysicalMemorySize) {
          memset (&Context->MemoryState->PhysicalMemory[VirtAddr + FileSize], 0, ZeroSize);
        }
      }

      Info->LoadedSize += MemSize;
    }

    free (SegmentData);
  }

  fclose (File);

  //
  // Set entry point as PC
  //
  Context->CpuState->Pc = Info->EntryPoint;

  return MMIX_SUCCESS;
}

/**
  Load ELF64 binary.

  @param[in,out]  Context       Emulator context.
  @param[in]      FilePath      Path to ELF file.
  @param[in]      LittleEndian  TRUE if little-endian.
  @param[out]     Info          Pointer to receive binary info.

  @retval MMIX_SUCCESS          ELF loaded successfully.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
LoadElf64 (
  IN OUT MMIX_EMULATOR_CONTEXT  *Context,
  IN     CONST CHAR8            *FilePath,
  IN     BOOLEAN                LittleEndian,
  OUT    MMIX_BINARY_INFO       *Info
  )
{
  FILE         *File;
  Elf64_Ehdr   Ehdr;
  Elf64_Phdr   Phdr;
  UINT8        *SegmentData;
  UINT16       i;
  size_t       BytesRead;

  //
  // Open file
  //
  File = fopen (FilePath, "rb");
  if (File == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  //
  // Read ELF header
  //
  BytesRead = fread (&Ehdr, 1, sizeof (Elf64_Ehdr), File);
  if (BytesRead != sizeof (Elf64_Ehdr)) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Get entry point
  //
  Info->EntryPoint = ElfConvert64 (Ehdr.e_entry, LittleEndian);
  Info->Endianness = LittleEndian ? MmixEndianLittle : MmixEndianBig;
  Info->Bits = 64;

  //
  // Load program headers
  //
  for (i = 0; i < ElfConvert16 (Ehdr.e_phnum, LittleEndian); i++) {
    //
    // Seek to program header
    //
    fseek (File, ElfConvert64 (Ehdr.e_phoff, LittleEndian) + i * sizeof (Elf64_Phdr), SEEK_SET);

    //
    // Read program header
    //
    BytesRead = fread (&Phdr, 1, sizeof (Elf64_Phdr), File);
    if (BytesRead != sizeof (Elf64_Phdr)) {
      continue;
    }

    //
    // Only load PT_LOAD segments
    //
    if (ElfConvert32 (Phdr.p_type, LittleEndian) != PT_LOAD) {
      continue;
    }

    //
    // Get segment properties
    //
    UINT64  FileOffset = ElfConvert64 (Phdr.p_offset, LittleEndian);
    UINT64  VirtAddr = ElfConvert64 (Phdr.p_vaddr, LittleEndian);
    UINT64  FileSize = ElfConvert64 (Phdr.p_filesz, LittleEndian);
    UINT64  MemSize = ElfConvert64 (Phdr.p_memsz, LittleEndian);

    //
    // Allocate buffer for segment
    //
    SegmentData = (UINT8 *)malloc (FileSize);
    if (SegmentData == NULL) {
      fclose (File);
      return MMIX_ERROR_OUT_OF_MEMORY;
    }

    //
    // Read segment data
    //
    fseek (File, FileOffset, SEEK_SET);
    BytesRead = fread (SegmentData, 1, FileSize, File);
    if (BytesRead != FileSize) {
      free (SegmentData);
      continue;
    }

    //
    // Write to emulator memory
    //
    if (VirtAddr + FileSize <= Context->MemoryState->PhysicalMemorySize) {
      memcpy (&Context->MemoryState->PhysicalMemory[VirtAddr], SegmentData, FileSize);

      //
      // Zero-fill remaining space
      //
      if (MemSize > FileSize) {
        UINT64  ZeroSize = MemSize - FileSize;
        if (VirtAddr + MemSize <= Context->MemoryState->PhysicalMemorySize) {
          memset (&Context->MemoryState->PhysicalMemory[VirtAddr + FileSize], 0, ZeroSize);
        }
      }

      Info->LoadedSize += MemSize;
    }

    free (SegmentData);
  }

  fclose (File);

  //
  // Set entry point as PC
  //
  Context->CpuState->Pc = Info->EntryPoint;

  return MMIX_SUCCESS;
}

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
  )
{
  FILE           *File;
  UINT8          Ident[EI_NIDENT];
  BOOLEAN        LittleEndian;
  BOOLEAN        Is64Bit;
  MMIX_STATUS    Status;
  MMIX_BINARY_INFO  LocalInfo;

  if (Context == NULL || FilePath == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Open file and read identification
  //
  File = fopen (FilePath, "rb");
  if (File == NULL) {
    return MMIX_ERROR_NOT_FOUND;
  }

  if (fread (Ident, 1, EI_NIDENT, File) != EI_NIDENT) {
    fclose (File);
    return MMIX_ERROR_DEVICE_ERROR;
  }

  fclose (File);

  //
  // Check ELF magic
  //
  if (Ident[EI_MAG0] != 0x7F || Ident[EI_MAG1] != 'E' ||
      Ident[EI_MAG2] != 'L' || Ident[EI_MAG3] != 'F') {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Determine class and endianness
  //
  Is64Bit = (Ident[EI_CLASS] == ELFCLASS64);
  LittleEndian = (Ident[EI_DATA] == ELFDATA2LSB);

  //
  // Initialize info
  //
  memset (&LocalInfo, 0, sizeof (MMIX_BINARY_INFO));
  LocalInfo.Format = Is64Bit ? MmixBinaryFormatElf64 : MmixBinaryFormatElf32;

  //
  // Load based on class
  //
  if (Is64Bit) {
    Status = LoadElf64 (Context, FilePath, LittleEndian, &LocalInfo);
  } else {
    Status = LoadElf32 (Context, FilePath, LittleEndian, &LocalInfo);
  }

  //
  // Copy info if requested
  //
  if (Info != NULL && !MMIX_IS_ERROR (Status)) {
    memcpy (Info, &LocalInfo, sizeof (MMIX_BINARY_INFO));
  }

  return Status;
}
