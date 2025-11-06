/** @file
  MMIX Page table walking implementation.

  This file implements complete 4-level page table walking for virtual
  address translation.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <stdlib.h>
#include <string.h>
#include "../../include/MmixMemory.h"

/**
  Read page table entry from physical memory.

  @param[in]      MemoryState   Pointer to memory state.
  @param[in]      PhysicalAddr  Physical address of PTE.
  @param[out]     Entry         Pointer to receive page table entry.

  @retval MMIX_SUCCESS          Entry read successfully.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
ReadPageTableEntry (
  IN  MMIX_MEMORY_STATE  *MemoryState,
  IN  UINT64             PhysicalAddr,
  OUT PAGE_TABLE_ENTRY   *Entry
  )
{
  if (PhysicalAddr + sizeof (UINT64) > MemoryState->PhysicalMemorySize) {
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  //
  // Read 64-bit PTE from physical memory
  //
  memcpy (&Entry->Uint64, &MemoryState->PhysicalMemory[PhysicalAddr], sizeof (UINT64));

  return MMIX_SUCCESS;
}

/**
  Write page table entry to physical memory.

  @param[in,out]  MemoryState   Pointer to memory state.
  @param[in]      PhysicalAddr  Physical address of PTE.
  @param[in]      Entry         Page table entry to write.

  @retval MMIX_SUCCESS          Entry written successfully.
  @retval Others                Error occurred.

**/
STATIC
MMIX_STATUS
WritePageTableEntry (
  IN OUT MMIX_MEMORY_STATE  *MemoryState,
  IN     UINT64             PhysicalAddr,
  IN     PAGE_TABLE_ENTRY   *Entry
  )
{
  if (PhysicalAddr + sizeof (UINT64) > MemoryState->PhysicalMemorySize) {
    return MMIX_ERROR_OUT_OF_RESOURCES;
  }

  //
  // Write 64-bit PTE to physical memory
  //
  memcpy (&MemoryState->PhysicalMemory[PhysicalAddr], &Entry->Uint64, sizeof (UINT64));

  return MMIX_SUCCESS;
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
  MMIX_STATUS       Status;
  PAGE_TABLE_ENTRY  Entry;
  UINT64            TableBase;
  UINT64            EntryAddress;
  UINT32            Level;
  UINT8             PermsAccum;
  BOOLEAN           Modified;

  if (MemoryState == NULL || PhysicalAddr == NULL || Permissions == NULL) {
    return MMIX_ERROR_INVALID_PARAMETER;
  }

  //
  // If no page table base, use identity mapping
  //
  if (MemoryState->PageTableBase == 0) {
    *PhysicalAddr = VirtualAddr;
    *Permissions = MMIX_MEMORY_READ | MMIX_MEMORY_WRITE | MMIX_MEMORY_EXECUTE;
    return MMIX_SUCCESS;
  }

  //
  // Start with page table base
  //
  TableBase = MemoryState->PageTableBase;
  PermsAccum = 0xFF;  // Start with all permissions, will be ANDed

  //
  // Walk through 4 levels
  //
  for (Level = 0; Level < 4; Level++) {
    UINT32  Index;
    UINT32  ShiftAmount;

    //
    // Calculate index for this level
    // Level 0 (PML4): bits 47:39 (9 bits)
    // Level 1 (PDPT): bits 38:30 (9 bits)
    // Level 2 (PD):   bits 29:21 (9 bits)
    // Level 3 (PT):   bits 20:12 (9 bits)
    //
    ShiftAmount = 39 - (Level * 9);
    Index = (VirtualAddr >> ShiftAmount) & 0x1FF;

    //
    // Calculate entry address
    //
    EntryAddress = TableBase + (Index * sizeof (UINT64));

    //
    // Read page table entry
    //
    Status = ReadPageTableEntry (MemoryState, EntryAddress, &Entry);
    if (MMIX_IS_ERROR (Status)) {
      return Status;
    }

    //
    // Check if present
    //
    if (Entry.Bits.Present == 0) {
      return MMIX_ERROR_NOT_FOUND;
    }

    //
    // Accumulate permissions (AND together for most restrictive)
    //
    PermsAccum = 0;
    if (Entry.Bits.ReadWrite) {
      PermsAccum |= MMIX_MEMORY_READ | MMIX_MEMORY_WRITE;
    } else {
      PermsAccum |= MMIX_MEMORY_READ;
    }

    if (Entry.Bits.ExecuteDisable == 0) {
      PermsAccum |= MMIX_MEMORY_EXECUTE;
    }

    if (Entry.Bits.UserSupervisor) {
      PermsAccum |= MMIX_MEMORY_USER;
    }

    if (Entry.Bits.CacheDisable == 0) {
      PermsAccum |= MMIX_MEMORY_CACHED;
    }

    //
    // Update accessed bit if requested
    //
    Modified = FALSE;
    if (SetAccessed && Entry.Bits.Accessed == 0) {
      Entry.Bits.Accessed = 1;
      Modified = TRUE;
    }

    //
    // Check if this is a large page (not at last level)
    //
    if (Level < 3 && Entry.Bits.PageSize) {
      UINT64  PageMask;
      UINT64  PageOffset;

      //
      // This is a large page entry
      // Level 1: 1GB page (bits 29:0 are offset)
      // Level 2: 2MB page (bits 20:0 are offset)
      //
      if (Level == 1) {
        PageMask = 0x3FFFFFFFULL;  // 1GB - 1
      } else if (Level == 2) {
        PageMask = 0x1FFFFFULL;    // 2MB - 1
      } else {
        PageMask = 0x1FFFFFULL;
      }

      PageOffset = VirtualAddr & PageMask;
      *PhysicalAddr = (Entry.Bits.PhysicalAddress << 12) | PageOffset;
      *Permissions = PermsAccum;

      //
      // Update dirty bit for writes if requested
      //
      if (SetDirty && Entry.Bits.Dirty == 0) {
        Entry.Bits.Dirty = 1;
        Modified = TRUE;
      }

      //
      // Write back entry if modified
      //
      if (Modified) {
        WritePageTableEntry (MemoryState, EntryAddress, &Entry);
      }

      return MMIX_SUCCESS;
    }

    //
    // Update dirty bit at leaf level if requested
    //
    if (Level == 3 && SetDirty && Entry.Bits.Dirty == 0) {
      Entry.Bits.Dirty = 1;
      Modified = TRUE;
    }

    //
    // Write back entry if modified
    //
    if (Modified) {
      WritePageTableEntry (MemoryState, EntryAddress, &Entry);
    }

    //
    // If this is the last level (level 3), we have the final translation
    //
    if (Level == 3) {
      UINT64  PageOffset;

      //
      // 4KB page: bits 11:0 are offset
      //
      PageOffset = VirtualAddr & 0xFFFULL;
      *PhysicalAddr = (Entry.Bits.PhysicalAddress << 12) | PageOffset;
      *Permissions = PermsAccum;

      return MMIX_SUCCESS;
    }

    //
    // Move to next level table
    //
    TableBase = Entry.Bits.PhysicalAddress << 12;
  }

  //
  // Should not reach here
  //
  return MMIX_ERROR_DEVICE_ERROR;
}

/**
  Create a page table entry.

  Helper function to create a properly formatted page table entry.

  @param[out]  Entry             Pointer to entry to fill.
  @param[in]   PhysicalAddress   Physical frame address (>> 12).
  @param[in]   Writable          TRUE if writable.
  @param[in]   UserMode          TRUE if user-accessible.
  @param[in]   Executable        TRUE if executable.
  @param[in]   LargePage         TRUE if large page.

**/
VOID
MmixCreatePageTableEntry (
  OUT PAGE_TABLE_ENTRY  *Entry,
  IN  UINT64            PhysicalAddress,
  IN  BOOLEAN           Writable,
  IN  BOOLEAN           UserMode,
  IN  BOOLEAN           Executable,
  IN  BOOLEAN           LargePage
  )
{
  if (Entry == NULL) {
    return;
  }

  memset (Entry, 0, sizeof (PAGE_TABLE_ENTRY));

  Entry->Bits.Present = 1;
  Entry->Bits.ReadWrite = Writable ? 1 : 0;
  Entry->Bits.UserSupervisor = UserMode ? 1 : 0;
  Entry->Bits.PhysicalAddress = PhysicalAddress;
  Entry->Bits.ExecuteDisable = Executable ? 0 : 1;
  Entry->Bits.PageSize = LargePage ? 1 : 0;
}
