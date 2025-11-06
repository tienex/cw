# MMIX Virtual Memory Implementations

## Overview

MMIX supports three virtual memory implementation variants, each with different hardware complexity and performance characteristics.

## Variant 1: TLB-Only Mode

### Description
The simplest implementation uses only Translation Lookaside Buffers (TLBs) without hardware page table walking. All page table operations are performed in software by the OS.

### Characteristics
- **Hardware**: TLB entries only (16-256 entries typical)
- **Page Table Format**: OS-defined (flexible)
- **TLB Miss Handling**: Software trap to OS
- **Performance**: Higher miss penalty, lower hardware complexity
- **Power**: Lower static power (less hardware)

### Implementation
```c
typedef struct {
  // TLB entries
  TLB_ENTRY  Entries[MMIX_TLB_SIZE];
  UINT32     EntryCount;

  // TLB miss causes exception
  // OS handles page table walk in software
  BOOLEAN    SoftwareManaged;
} MMIX_TLB_ONLY_VM;
```

### TLB Miss Flow
1. Virtual address translation fails in TLB
2. CPU raises TLB miss exception
3. OS exception handler:
   - Walks page tables in memory
   - Finds physical address
   - Loads entry into TLB (TLBWR instruction)
   - Returns from exception
4. Instruction retries and succeeds

### Advantages
- Simple hardware
- Flexible page table formats
- Lower die area and power
- Easier to verify

### Disadvantages
- High TLB miss penalty (50-100+ cycles)
- OS overhead on every miss
- Not suitable for large working sets

## Variant 2: Hardware Page Table (Intel-style EPT/IHPT)

### Description
Hardware page table walker (PTW) that automatically walks page tables on TLB miss, similar to Intel's x86/EPT architecture.

### Characteristics
- **Hardware**: TLB + PTW state machine
- **Page Table Format**: Fixed hierarchical (4-level)
- **TLB Miss Handling**: Hardware automatic
- **Performance**: Lower miss penalty (10-30 cycles)
- **Power**: Moderate (PTW logic active on misses)

### Page Table Format
```
Level 4 (PML4): 512 entries, covers 256 TiB
Level 3 (PDPT): 512 entries, covers 512 GiB
Level 2 (PD):   512 entries, covers 1 GiB
Level 1 (PT):   512 entries, covers 2 MiB

Each entry: 64 bits
  [63]    : Execute disable
  [62:52] : Available
  [51:12] : Physical address bits
  [11:9]  : Available
  [8]     : Global
  [7]     : Page size (1=large)
  [6]     : Dirty
  [5]     : Accessed
  [4]     : Cache disable
  [3]     : Write through
  [2]     : User/supervisor
  [1]     : Read/write
  [0]     : Present
```

### Implementation
```c
typedef struct {
  // TLB for fast lookups
  TLB_ENTRY  TlbEntries[MMIX_TLB_SIZE];

  // Page table base register (like CR3)
  UINT64     PageTableBase;

  // PTW cache for partial walks
  struct {
    UINT64  VirtualAddr;
    UINT64  L4Entry;
    UINT64  L3Entry;
    UINT64  L2Entry;
    BOOLEAN Valid;
  } PtwCache[4];

  // Hardware walker enabled
  BOOLEAN    HardwareWalkerEnabled;
} MMIX_IHPT_VM;
```

### TLB Miss Flow
1. Virtual address translation fails in TLB
2. Hardware PTW activates:
   - Load PML4 entry from PageTableBase
   - Check present bit, permissions
   - Load PDPT entry
   - Load PD entry
   - Load PT entry
   - Check all permission bits cumulative
3. If successful:
   - Load TLB with translation
   - Retry instruction
4. If fault:
   - Raise page fault exception
   - OS handles fault

### Advantages
- Low TLB miss penalty
- Transparent to software
- Well-understood design (Intel/AMD compatible)
- Good for general-purpose workloads

### Disadvantages
- Fixed page table format
- Higher hardware cost
- PTW cache adds complexity
- Power consumption on walks

## Variant 3: Nested Virtualization (Intel-style IVHPT)

### Description
Two-level address translation for hypervisors. Guest virtual → Guest physical (gVA→gPA) and Guest physical → Host physical (gPA→hPA).

### Characteristics
- **Hardware**: TLB + dual PTW + combined TLB
- **Page Tables**: Two independent hierarchies
- **TLB Miss Handling**: Nested hardware walk
- **Performance**: Higher latency (20-60 cycles)
- **Power**: Highest (two PTWs potentially active)

### Page Table Structure
```
Guest Page Tables (gVA → gPA):
  Level 4 → Level 3 → Level 2 → Level 1

Host Page Tables (gPA → hPA):
  Level 4 → Level 3 → Level 2 → Level 1

Each guest PTE address must be translated through host tables!
```

### Implementation
```c
typedef struct {
  // Combined TLB (caches full gVA→hPA translation)
  struct {
    UINT64   GuestVirtual;
    UINT64   HostPhysical;
    UINT16   GuestAsid;
    UINT16   HostAsid;
    UINT8    Permissions;  // Combined guest & host
    BOOLEAN  Valid;
  } CombinedTlb[MMIX_TLB_SIZE];

  // Guest page table base
  UINT64  GuestPageTableBase;

  // Host/hypervisor page table base (like EPT pointer)
  UINT64  HostPageTableBase;

  // Virtualization enabled
  BOOLEAN VirtualizationEnabled;

  // PTW caches for both levels
  struct {
    UINT64  GuestWalkCache[4];
    UINT64  HostWalkCache[4];
  } PtwCaches;
} MMIX_IVHPT_VM;
```

### Nested TLB Miss Flow
1. gVA lookup in combined TLB fails
2. Guest PTW begins:
   - Translate gPML4_base through host tables → hPML4_base
   - Load gPML4 entry from memory
   - Translate gPDPT_base through host tables → hPDPT_base
   - Load gPDPT entry
   - Translate gPD_base through host tables → hPD_base
   - Load gPD entry
   - Translate gPT_base through host tables → hPT_base
   - Load gPT entry (gives gPA)
3. Host PTW for final gPA→hPA:
   - Walk host tables with gPA
   - Get final hPA
4. Load combined TLB with gVA→hPA + permissions
5. Retry instruction

### Worst Case
24 memory accesses for full nested walk:
- 4 guest table levels × (1 entry load + 1 host translation walk)
- Each host translation: up to 4 accesses
- = 4 × (1 + 4) = 20 accesses minimum
- Plus final gPA translation: +4
- Total: 24 memory accesses!

### Optimizations
1. **Combined TLB**: Cache full gVA→hPA mapping
2. **Host PTW Cache**: Cache hPA for guest table pages
3. **Large Pages**: Use 2MB/1GB pages to skip levels
4. **Tagged TLB**: Include ASID to avoid flushes

### Advantages
- Full virtualization support
- Nested guests possible
- Isolation guaranteed by hardware
- Intel EPT/AMD NPT compatible

### Disadvantages
- High complexity
- Worst-case latency very high
- Significant power consumption
- Large die area for PTW logic

## Comparison Table

| Feature              | TLB-Only | IHPT (EPT) | IVHPT (Nested) |
|---------------------|----------|------------|----------------|
| Hardware Cost       | Low      | Medium     | High           |
| TLB Miss Latency    | 50-100c  | 10-30c     | 20-60c         |
| Worst Case Latency  | ~100c    | ~30c       | ~200c          |
| Power (Active)      | Low      | Medium     | High           |
| Power (Idle)        | Lowest   | Low        | Medium         |
| Die Area            | ~5K gates| ~50K gates | ~150K gates    |
| Virtualization      | Software | Software   | Hardware       |
| Flexibility         | High     | Low        | Low            |
| OS Complexity       | High     | Low        | Low            |

## Configuration

MMIX supports runtime selection of VM mode:

```c
typedef enum {
  MmixVmModeTlbOnly,      // Software-managed TLBs
  MmixVmModeIhpt,         // Hardware page tables (Intel EPT-style)
  MmixVmModeIvhpt,        // Nested virtualization (Intel IVHPT-style)
} MMIX_VM_MODE;

// Set via special register
MMIX_STATUS MmixSetVmMode(MMIX_CPU_STATE *Cpu, MMIX_VM_MODE Mode);
```

## Use Cases

### TLB-Only
- Embedded systems
- Real-time systems with predictable timing
- Systems with small working sets
- Custom OS with specialized page table formats

### IHPT
- General-purpose operating systems
- Desktop/server workloads
- Large memory workloads
- Linux, Windows, BSD

### IVHPT
- Cloud computing platforms
- Hypervisors (KVM, Xen, VMware)
- Container isolation
- Trusted execution environments

## Performance Guidelines

### TLB-Only
- Keep TLB large (128+ entries)
- Use superpages (2MB/1GB) extensively
- Pin critical pages
- Optimize OS handler (<50 cycles ideal)

### IHPT
- TLB can be smaller (64 entries)
- PTW cache is critical (4-8 entries)
- Large pages still beneficial
- Page table locality matters

### IVHPT
- Combined TLB should be large (128+ entries)
- Host PTW cache essential
- Large pages critical (reduce walk depth)
- Minimize VM exits

## Implementation Files

- `src/memory/VmTlbOnly.c` - TLB-only implementation
- `src/memory/VmIhpt.c` - Hardware page table walker
- `src/memory/VmIvhpt.c` - Nested virtualization
- `include/MmixVm.h` - Common VM interfaces
