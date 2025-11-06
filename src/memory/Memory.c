/** @file
  MMIX Memory management implementation.

  This file implements the memory subsystem including physical memory emulation,
  virtual memory management with page tables, and TLB.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include "../../include/MmixMemory.h"
#include "../../include/MmixCore.h"

/**
  Initialize memory subsystem.

  Allocates physical memory and initializes memory management structures
  including TLB and page table support.

  @param[out]  MemoryState       Pointer to receive memory state pointer.
  @param[in]   PhysicalMemSize   Size of physical memory in bytes.

  @retval MMIX_SUCCESS           Memory initialized successfully.
  @retval MMIX_ERROR_OUT_OF_MEMORY  Failed to allocate memory.

**/
MMIX_STATUS
MmixMemoryInitialize (
  OUT MMIX_MEMORY_STATE  **MemoryState,
  IN  UINT64             PhysicalMemSize
  )
{
  MMIX_MEMORY_STATE  *Memory;
  UINT32             i;

  if (MemoryState == NULL || PhysicalMemSize == 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Allocate memory state structure
  //
  Memory = (MMIX_MEMORY_STATE *)malloc (sizeof (MMIX_MEMORY_STATE));
  if (Memory == NULL) {
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  memset (Memory, 0, sizeof (MMIX_MEMORY_STATE));

  //
  // Allocate physical memory
  //
  Memory->PhysicalMemory = (UINT8 *)malloc (PhysicalMemSize);
  if (Memory->PhysicalMemory == NULL) {
    free (Memory);
    return MMIX_ERROR_OUT_OF_MEMORY;
  }

  memset (Memory->PhysicalMemory, 0, PhysicalMemSize);
  Memory->PhysicalMemorySize = PhysicalMemSize;

  //
  // Initialize TLB
  //
  for (i = 0; i < MMIX_TLB_ENTRY_COUNT; i++) {
    Memory->TlbEntries[i].Valid = FALSE;
  }

  Memory->TlbNextReplace = 0;
  Memory->TlbHits = 0;
  Memory->TlbMisses = 0;

  //
  // Initialize paging (disabled by default)
  //
  Memory->PagingEnabled = FALSE;
  Memory->PageTableBase = 0;
  Memory->CurrentAsid = 0;

  //
  // Initialize region list (empty for now)
  //
  Memory->Regions = NULL;
  Memory->RegionCount = 0;

  *MemoryState = Memory;
  return MMIX_SUCCESS;
}

/**
  Read from virtual memory.

  Performs virtual to physical address translation if paging is enabled,
  checks permissions, and reads data from memory.

  @param[in]      MemoryState   Pointer to memory state.
  @param[in]      VirtualAddr   Virtual address to read from.
  @param[out]     Buffer        Buffer to receive data.
  @param[in]      Size          Number of bytes to read.
  @param[in]      Execute       TRUE if this is an instruction fetch.

  @retval MMIX_SUCCESS          Read completed successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Permission denied.
  @retval Others                Page fault or other error.

**/
MMIX_STATUS
MmixMemoryRead (
  IN  MMIX_MEMORY_STATE  *MemoryState,
  IN  UINT64             VirtualAddr,
  OUT VOID               *Buffer,
  IN  UINT64             Size,
  IN  BOOLEAN            Execute
  )
{
  MMIX_STATUS  Status;
  UINT64       PhysicalAddr;
  UINT8        Permissions;
  UINT32       i;

  if (MemoryState == NULL || Buffer == NULL || Size == 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // If paging is enabled, translate address
  //
  if (MemoryState->PagingEnabled) {
    Status = MmixMemoryTranslate (
               MemoryState,
               VirtualAddr,
               &PhysicalAddr,
               &Permissions
               );
    if (MMIX_IS_ERROR (Status)) {
      return Status;
    }

    //
    // Check read permission
    //
    if ((Permissions & MMIX_MEMORY_READ) == 0) {
      return MMIX_ERROR_ACCESS_DENIED;
    }

    //
    // Check execute permission if this is a fetch
    //
    if (Execute && ((Permissions & MMIX_MEMORY_EXECUTE) == 0)) {
      return MMIX_ERROR_ACCESS_DENIED;
    }
  } else {
    //
    // No translation, use physical address directly
    //
    PhysicalAddr = VirtualAddr;
  }

  //
  // Check physical address bounds
  //
  if (PhysicalAddr + Size > MemoryState->PhysicalMemorySize) {
    //
    // Check if this is an MMIO region
    //
    for (i = 0; i < MemoryState->RegionCount; i++) {
      if (PhysicalAddr >= MemoryState->Regions[i].BaseAddress &&
          PhysicalAddr < MemoryState->Regions[i].BaseAddress + MemoryState->Regions[i].Size) {
        if (MemoryState->Regions[i].Mmio && MemoryState->Regions[i].AccessHandler != NULL) {
          return MemoryState->Regions[i].AccessHandler (
                   MemoryState,
                   PhysicalAddr,
                   Buffer,
                   Size,
                   FALSE  // Read
                   );
        }
      }
    }

    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Read from physical memory
  //
  memcpy (Buffer, &MemoryState->PhysicalMemory[PhysicalAddr], Size);

  return MMIX_SUCCESS;
}

/**
  Write to virtual memory.

  Performs virtual to physical address translation if paging is enabled,
  checks permissions, and writes data to memory.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      VirtualAddr   Virtual address to write to.
  @param[in]      Buffer        Buffer containing data to write.
  @param[in]      Size          Number of bytes to write.

  @retval MMIX_SUCCESS          Write completed successfully.
  @retval MMIX_ERROR_ACCESS_DENIED  Permission denied.
  @retval MMIX_ERROR_WRITE_PROTECTED  Page is write-protected.
  @retval Others                Page fault or other error.

**/
MMIX_STATUS
MmixMemoryWrite (
  IN OUT MMIX_MEMORY_STATE  *MemoryState,
  IN     UINT64             VirtualAddr,
  IN     CONST VOID         *Buffer,
  IN     UINT64             Size
  )
{
  MMIX_STATUS  Status;
  UINT64       PhysicalAddr;
  UINT8        Permissions;
  UINT32       i;

  if (MemoryState == NULL || Buffer == NULL || Size == 0) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // If paging is enabled, translate address
  //
  if (MemoryState->PagingEnabled) {
    Status = MmixMemoryTranslate (
               MemoryState,
               VirtualAddr,
               &PhysicalAddr,
               &Permissions
               );
    if (MMIX_IS_ERROR (Status)) {
      return Status;
    }

    //
    // Check write permission
    //
    if ((Permissions & MMIX_MEMORY_WRITE) == 0) {
      return MMIX_ERROR_WRITE_PROTECTED;
    }
  } else {
    //
    // No translation
    //
    PhysicalAddr = VirtualAddr;
  }

  //
  // Check physical address bounds
  //
  if (PhysicalAddr + Size > MemoryState->PhysicalMemorySize) {
    //
    // Check if this is an MMIO region
    //
    for (i = 0; i < MemoryState->RegionCount; i++) {
      if (PhysicalAddr >= MemoryState->Regions[i].BaseAddress &&
          PhysicalAddr < MemoryState->Regions[i].BaseAddress + MemoryState->Regions[i].Size) {
        if (MemoryState->Regions[i].Mmio && MemoryState->Regions[i].AccessHandler != NULL) {
          return MemoryState->Regions[i].AccessHandler (
                   MemoryState,
                   PhysicalAddr,
                   (VOID *)Buffer,
                   Size,
                   TRUE  // Write
                   );
        }
      }
    }

    return MMIX_ERROR_DEVICE_ERROR;
  }

  //
  // Write to physical memory
  //
  memcpy (&MemoryState->PhysicalMemory[PhysicalAddr], Buffer, Size);

  return MMIX_SUCCESS;
}

/**
  Translate virtual address to physical address.

  Performs page table walk to translate a virtual address to physical,
  checking permissions and updating TLB.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      VirtualAddr   Virtual address to translate.
  @param[out]     PhysicalAddr  Pointer to receive physical address.
  @param[out]     Permissions   Pointer to receive permission flags.

  @retval MMIX_SUCCESS          Translation successful.
  @retval MMIX_ERROR_NOT_FOUND  Page not present.
  @retval Others                Translation error.

**/
MMIX_STATUS
MmixMemoryTranslate (
  IN OUT MMIX_MEMORY_STATE  *MemoryState,
  IN     UINT64             VirtualAddr,
  OUT    UINT64             *PhysicalAddr,
  OUT    UINT8              *Permissions
  )
{
  MMIX_STATUS  Status;
  TLB_ENTRY    *TlbEntry;

  if (MemoryState == NULL || PhysicalAddr == NULL || Permissions == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // First check TLB
  //
  Status = MmixTlbLookup (MemoryState, VirtualAddr, &TlbEntry);
  if (Status == MMIX_SUCCESS) {
    //
    // TLB hit
    //
    MemoryState->TlbHits++;
    *PhysicalAddr = TlbEntry->PhysicalAddress | (VirtualAddr & 0xFFF);
    *Permissions = TlbEntry->Flags;
    return MMIX_SUCCESS;
  }

  //
  // TLB miss - perform page table walk
  //
  MemoryState->TlbMisses++;
  Status = MmixPageTableWalk (
             MemoryState,
             VirtualAddr,
             PhysicalAddr,
             Permissions,
             TRUE,   // Set accessed bit
             FALSE   // Don't set dirty bit yet
             );
  if (MMIX_IS_ERROR (Status)) {
    return Status;
  }

  //
  // Insert into TLB
  //
  MmixTlbInsert (MemoryState, VirtualAddr, *PhysicalAddr, 0, *Permissions);

  return MMIX_SUCCESS;
}

/**
  Look up address in TLB.

  Searches the TLB for a matching virtual address and ASID.

  @param[in]      MemoryState   Pointer to memory state.
  @param[in]      VirtualAddr   Virtual address to look up.
  @param[out]     Entry         Pointer to receive TLB entry.

  @retval MMIX_SUCCESS          TLB hit, entry found.
  @retval MMIX_ERROR_NOT_FOUND  TLB miss.

**/
MMIX_STATUS
MmixTlbLookup (
  IN  MMIX_MEMORY_STATE  *MemoryState,
  IN  UINT64             VirtualAddr,
  OUT TLB_ENTRY          **Entry
  )
{
  UINT32  i;
  UINT64  VirtualPage;

  if (MemoryState == NULL || Entry == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  VirtualPage = VirtualAddr & ~0xFFFULL;  // Mask off page offset

  //
  // Search TLB for matching entry
  //
  for (i = 0; i < MMIX_TLB_ENTRY_COUNT; i++) {
    if (MemoryState->TlbEntries[i].Valid &&
        MemoryState->TlbEntries[i].VirtualAddress == VirtualPage &&
        MemoryState->TlbEntries[i].Asid == MemoryState->CurrentAsid) {
      *Entry = &MemoryState->TlbEntries[i];
      return MMIX_SUCCESS;
    }
  }

  return MMIX_ERROR_NOT_FOUND;
}

/**
  Insert entry into TLB.

  Adds or updates a TLB entry with the specified mapping.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      VirtualAddr   Virtual address.
  @param[in]      PhysicalAddr  Physical address.
  @param[in]      PageSize      Page size encoding.
  @param[in]      Flags         Permission and attribute flags.

  @retval MMIX_SUCCESS          Entry inserted successfully.

**/
MMIX_STATUS
MmixTlbInsert (
  IN OUT MMIX_MEMORY_STATE  *MemoryState,
  IN     UINT64             VirtualAddr,
  IN     UINT64             PhysicalAddr,
  IN     UINT8              PageSize,
  IN     UINT8              Flags
  )
{
  UINT32  Index;

  if (MemoryState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Use round-robin replacement
  //
  Index = MemoryState->TlbNextReplace;
  MemoryState->TlbNextReplace = (MemoryState->TlbNextReplace + 1) % MMIX_TLB_ENTRY_COUNT;

  //
  // Fill in TLB entry
  //
  MemoryState->TlbEntries[Index].VirtualAddress = VirtualAddr & ~0xFFFULL;
  MemoryState->TlbEntries[Index].PhysicalAddress = PhysicalAddr & ~0xFFFULL;
  MemoryState->TlbEntries[Index].Asid = MemoryState->CurrentAsid;
  MemoryState->TlbEntries[Index].PageSize = PageSize;
  MemoryState->TlbEntries[Index].Flags = Flags;
  MemoryState->TlbEntries[Index].Valid = TRUE;

  return MMIX_SUCCESS;
}

/**
  Flush TLB entries.

  Invalidates TLB entries based on flush type and parameters.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      FlushType     Type of flush to perform.
  @param[in]      Address       Address parameter (for address flush).
  @param[in]      Asid          ASID parameter (for ASID flush).

**/
VOID
MmixTlbFlush (
  IN OUT MMIX_MEMORY_STATE  *MemoryState,
  IN     TLB_FLUSH_TYPE     FlushType,
  IN     UINT64             Address,
  IN     UINT16             Asid
  )
{
  UINT32  i;

  if (MemoryState == NULL) {
    return;
  }

  switch (FlushType) {
    case TlbFlushAll:
      //
      // Invalidate all TLB entries
      //
      for (i = 0; i < MMIX_TLB_ENTRY_COUNT; i++) {
        MemoryState->TlbEntries[i].Valid = FALSE;
      }
      break;

    case TlbFlushAsid:
      //
      // Invalidate entries matching ASID
      //
      for (i = 0; i < MMIX_TLB_ENTRY_COUNT; i++) {
        if (MemoryState->TlbEntries[i].Valid &&
            MemoryState->TlbEntries[i].Asid == Asid) {
          MemoryState->TlbEntries[i].Valid = FALSE;
        }
      }
      break;

    case TlbFlushAddress:
      //
      // Invalidate entry matching address
      //
      for (i = 0; i < MMIX_TLB_ENTRY_COUNT; i++) {
        if (MemoryState->TlbEntries[i].Valid &&
            MemoryState->TlbEntries[i].VirtualAddress == (Address & ~0xFFFULL)) {
          MemoryState->TlbEntries[i].Valid = FALSE;
        }
      }
      break;
  }
}

/**
  Walk page tables.

  Performs a page table walk starting from the page table base to
  translate a virtual address. Updates accessed and dirty bits.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      VirtualAddr   Virtual address to translate.
  @param[out]     PhysicalAddr  Pointer to receive physical address.
  @param[out]     Permissions   Pointer to receive permissions.
  @param[in]      SetAccessed   Update accessed bit.
  @param[in]      SetDirty      Update dirty bit (for writes).

  @retval MMIX_SUCCESS          Walk completed successfully.
  @retval MMIX_ERROR_NOT_FOUND  Page not present.
  @retval Others                Error during walk.

**/
MMIX_STATUS
MmixPageTableWalk (
  IN OUT MMIX_MEMORY_STATE  *MemoryState,
  IN     UINT64             VirtualAddr,
  OUT    UINT64             *PhysicalAddr,
  OUT    UINT8              *Permissions,
  IN     BOOLEAN            SetAccessed,
  IN     BOOLEAN            SetDirty
  )
{
  //
  // Page table walk implementation would go here
  // This is a simplified placeholder
  //
  if (MemoryState == NULL || PhysicalAddr == NULL || Permissions == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // For now, identity mapping
  //
  *PhysicalAddr = VirtualAddr;
  *Permissions = MMIX_MEMORY_READ | MMIX_MEMORY_WRITE | MMIX_MEMORY_EXECUTE;

  return MMIX_SUCCESS;
}

/**
  Register memory region.

  Registers a memory region with specific attributes. Used for
  both RAM and MMIO regions.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      BaseAddress   Base physical address.
  @param[in]      Size          Region size in bytes.
  @param[in]      Mmio          TRUE if MMIO region.
  @param[in]      Handler       Access handler (for MMIO).

  @retval MMIX_SUCCESS          Region registered successfully.
  @retval MMIX_ERROR_OUT_OF_RESOURCES  Too many regions.

**/
MMIX_STATUS
MmixMemoryRegisterRegion (
  IN OUT MMIX_MEMORY_STATE        *MemoryState,
  IN     UINT64                   BaseAddress,
  IN     UINT64                   Size,
  IN     BOOLEAN                  Mmio,
  IN     MMIX_MEMORY_ACCESS_HANDLER Handler OPTIONAL
  )
{
  MEMORY_REGION  *NewRegions;

  if (MemoryState == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // Reallocate region array
  //
  NewRegions = (MEMORY_REGION *)realloc (
                                  MemoryState->Regions,
                                  (MemoryState->RegionCount + 1) * sizeof (MEMORY_REGION)
                                  );
  if (NewRegions == NULL) {
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  MemoryState->Regions = NewRegions;

  //
  // Add new region
  //
  MemoryState->Regions[MemoryState->RegionCount].BaseAddress = BaseAddress;
  MemoryState->Regions[MemoryState->RegionCount].Size = Size;
  MemoryState->Regions[MemoryState->RegionCount].Mmio = Mmio;
  MemoryState->Regions[MemoryState->RegionCount].AccessHandler = Handler;
  MemoryState->Regions[MemoryState->RegionCount].Storage = NULL;

  MemoryState->RegionCount++;

  return MMIX_SUCCESS;
}

/**
  Destroy memory state.

  Frees all resources associated with the memory subsystem.

  @param[in]  MemoryState       Pointer to memory state to destroy.

**/
VOID
MmixMemoryDestroy (
  IN MMIX_MEMORY_STATE  *MemoryState
  )
{
  if (MemoryState != NULL) {
    if (MemoryState->PhysicalMemory != NULL) {
      free (MemoryState->PhysicalMemory);
    }
    if (MemoryState->Regions != NULL) {
      free (MemoryState->Regions);
    }
    free (MemoryState);
  }
}

//
// Endianness conversion functions
//

UINT64
MmixBigEndianToHost (
  IN UINT64  Value,
  IN UINT32  Size
  )
{
  //
  // Simplified - assumes host is little-endian
  //
  switch (Size) {
    case 1:
      return Value & 0xFF;
    case 2:
      return ((Value & 0xFF) << 8) | ((Value >> 8) & 0xFF);
    case 4:
      return ((Value & 0xFF) << 24) |
             (((Value >> 8) & 0xFF) << 16) |
             (((Value >> 16) & 0xFF) << 8) |
             ((Value >> 24) & 0xFF);
    case 8:
      return ((Value & 0xFF) << 56) |
             (((Value >> 8) & 0xFF) << 48) |
             (((Value >> 16) & 0xFF) << 40) |
             (((Value >> 24) & 0xFF) << 32) |
             (((Value >> 32) & 0xFF) << 24) |
             (((Value >> 40) & 0xFF) << 16) |
             (((Value >> 48) & 0xFF) << 8) |
             ((Value >> 56) & 0xFF);
    default:
      return Value;
  }
}

UINT64
MmixHostToBigEndian (
  IN UINT64  Value,
  IN UINT32  Size
  )
{
  return MmixBigEndianToHost (Value, Size);  // Symmetric operation
}

UINT64
MmixLittleEndianToHost (
  IN UINT64  Value,
  IN UINT32  Size
  )
{
  //
  // Assumes host is little-endian - no conversion needed
  //
  return Value;
}

UINT64
MmixHostToLittleEndian (
  IN UINT64  Value,
  IN UINT32  Size
  )
{
  return Value;  // No conversion needed
}
