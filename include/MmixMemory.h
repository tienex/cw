/** @file
  MMIX Memory Management definitions.

  This file defines the memory management subsystem including virtual
  memory, page tables, TLB, and physical memory emulation.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef __MMIX_MEMORY_H__
#define __MMIX_MEMORY_H__

#include "MmixTypes.h"

//
// Memory configuration constants
//

#define MMIX_PHYSICAL_MEMORY_SIZE     (4ULL * 1024 * 1024 * 1024)  // 4GB default
#define MMIX_PAGE_SIZE_4KB            4096
#define MMIX_PAGE_SIZE_2MB            (2 * 1024 * 1024)
#define MMIX_PAGE_SIZE_1GB            (1024 * 1024 * 1024)
#define MMIX_PAGE_SIZE_512GB          (512ULL * 1024 * 1024 * 1024)

#define MMIX_TLB_ENTRY_COUNT          256
#define MMIX_PAGE_TABLE_ENTRIES       512

//
// Page table level definitions
//

typedef enum {
  PageTableLevel4 = 0,  ///< PML4 (512GB per entry)
  PageTableLevel3 = 1,  ///< PDPT (1GB per entry)
  PageTableLevel2 = 2,  ///< PD (2MB per entry)
  PageTableLevel1 = 3   ///< PT (4KB per entry)
} PAGE_TABLE_LEVEL;

//
// Memory access permissions
//

#define MMIX_MEMORY_READ      0x01
#define MMIX_MEMORY_WRITE     0x02
#define MMIX_MEMORY_EXECUTE   0x04
#define MMIX_MEMORY_USER      0x08
#define MMIX_MEMORY_CACHED    0x10

//
// TLB flush types
//

typedef enum {
  TlbFlushAll = 0,      ///< Flush all TLB entries
  TlbFlushAsid = 1,     ///< Flush entries matching ASID
  TlbFlushAddress = 2   ///< Flush specific address
} TLB_FLUSH_TYPE;

//
// Memory region structure
//

/**
  Physical memory region descriptor.

  Describes a contiguous region of physical memory with specific
  attributes and access handlers.
**/
typedef struct {
  ///
  /// Base physical address
  ///
  UINT64    BaseAddress;

  ///
  /// Size in bytes
  ///
  UINT64    Size;

  ///
  /// Memory-mapped I/O region (not normal RAM)
  ///
  BOOLEAN   Mmio;

  ///
  /// Access handler for MMIO regions
  ///
  MMIX_MEMORY_ACCESS_HANDLER  AccessHandler;

  ///
  /// Pointer to backing storage (for RAM regions)
  ///
  VOID      *Storage;
} MEMORY_REGION;

//
// Memory state structure
//

/**
  Complete memory subsystem state.

  Contains physical memory, page tables, TLB, and memory management
  configuration for address translation and access control.
**/
struct _MMIX_MEMORY_STATE {
  ///
  /// Physical memory storage
  ///
  UINT8     *PhysicalMemory;

  ///
  /// Physical memory size in bytes
  ///
  UINT64    PhysicalMemorySize;

  ///
  /// Page table base address (CR3 equivalent)
  ///
  UINT64    PageTableBase;

  ///
  /// Current Address Space ID
  ///
  UINT16    CurrentAsid;

  ///
  /// Translation Lookaside Buffer
  ///
  TLB_ENTRY TlbEntries[MMIX_TLB_ENTRY_COUNT];

  ///
  /// TLB next replacement index (round-robin)
  ///
  UINT32    TlbNextReplace;

  ///
  /// Paging enabled
  ///
  BOOLEAN   PagingEnabled;

  ///
  /// Memory regions for MMIO
  ///
  MEMORY_REGION *Regions;

  ///
  /// Number of memory regions
  ///
  UINT32    RegionCount;

  ///
  /// TLB statistics
  ///
  UINT64    TlbHits;
  UINT64    TlbMisses;

  ///
  /// Pointer to CPU state (for accessing CR registers)
  ///
  MMIX_CPU_STATE  *CpuState;
};

//
// Memory management functions
//

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

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
  );

/**
  Destroy memory state.

  Frees all resources associated with the memory subsystem.

  @param[in]  MemoryState       Pointer to memory state to destroy.

**/
VOID
MmixMemoryDestroy (
  IN MMIX_MEMORY_STATE  *MemoryState
  );

//
// Utility functions for endianness handling
//

/**
  Convert value from big-endian to host byte order.

  @param[in]  Value             Value in big-endian.
  @param[in]  Size              Size in bytes (1, 2, 4, or 8).

  @return Value in host byte order.

**/
UINT64
MmixBigEndianToHost (
  IN UINT64  Value,
  IN UINT32  Size
  );

/**
  Convert value from host byte order to big-endian.

  @param[in]  Value             Value in host byte order.
  @param[in]  Size              Size in bytes (1, 2, 4, or 8).

  @return Value in big-endian.

**/
UINT64
MmixHostToBigEndian (
  IN UINT64  Value,
  IN UINT32  Size
  );

/**
  Convert value from little-endian to host byte order.

  @param[in]  Value             Value in little-endian.
  @param[in]  Size              Size in bytes (1, 2, 4, or 8).

  @return Value in host byte order.

**/
UINT64
MmixLittleEndianToHost (
  IN UINT64  Value,
  IN UINT32  Size
  );

/**
  Convert value from host byte order to little-endian.

  @param[in]  Value             Value in host byte order.
  @param[in]  Size              Size in bytes (1, 2, 4, or 8).

  @return Value in little-endian.

**/
UINT64
MmixHostToLittleEndian (
  IN UINT64  Value,
  IN UINT32  Size
  );

#endif // __MMIX_MEMORY_H__
