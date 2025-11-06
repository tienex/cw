# MMIX Emulator Architecture Documentation

## Overview

This document describes the architecture of a comprehensive MMIX emulator implementing modern features including virtualization, vector processing, machine learning acceleration, and complete hardware emulation.

## 1. MMIX Core Architecture

### 1.1 Base MMIX ISA

The emulator implements Donald Knuth's MMIX architecture with the following enhancements:

- **64-bit RISC architecture** with 256 general-purpose registers
- **Special registers**: rA (arithmetic status), rB (bootstrap), rC (cycle counter), rD (dividend), rE (epsilon), rF (failure location), rG (global threshold), rH (himult), rI (interval counter), rJ (return-jump), rK (interrupt mask), rL (local threshold), rM (multiplex mask), rN (serial number), rO (register stack offset), rP (prediction), rQ (interrupt request), rR (remainder), rS (register stack pointer), rT (trap address), rU (usage counter), rV (virtual translation), rW (where interrupted), rX (execution register), rY (Y operand), rZ (Z operand)
- **256 opcodes** including arithmetic, logical, memory, branch, and control operations
- **Register window mechanism** with local and global registers

### 1.2 Extended Features

- **Endianness switching**: Support for both big-endian (traditional MMIX) and little-endian modes
- **Privilege levels**: User mode, supervisor mode, hypervisor mode
- **Compressed instructions**: 16-bit instruction encoding for improved code density
- **Vector extensions**: Scalable vector and matrix operations
- **ML acceleration**: Specialized neural network instructions

## 2. Memory Management

### 2.1 Virtual Memory

- **64-bit virtual address space**
- **4-level page tables** supporting 4KB, 2MB, 1GB, and 512GB page sizes
- **TLB (Translation Lookaside Buffer)** with 256 entries
- **Memory protection**: Read, Write, Execute, User, Dirty, Accessed bits
- **ASID (Address Space Identifier)**: 16-bit ASID for efficient context switching

### 2.2 Page Table Structure

```
Level 4 (PML4): 512 entries, each covering 512GB
Level 3 (PDPT): 512 entries, each covering 1GB
Level 2 (PD):   512 entries, each covering 2MB
Level 1 (PT):   512 entries, each covering 4KB
```

### 2.3 Page Table Entry Format

```
Bits 63:52 - Reserved/Available for OS
Bits 51:12 - Physical Address
Bit  11    - AVL (Available)
Bit  10    - AVL (Available)
Bit  9     - AVL (Available)
Bit  8     - G (Global)
Bit  7     - PS (Page Size)
Bit  6     - D (Dirty)
Bit  5     - A (Accessed)
Bit  4     - PCD (Cache Disable)
Bit  3     - PWT (Write-Through)
Bit  2     - U/S (User/Supervisor)
Bit  1     - R/W (Read/Write)
Bit  0     - P (Present)
```

## 3. Hypervisor Architecture (MIPS VZ Style)

### 3.1 Virtualization Extensions

- **Guest/Host mode separation**
- **Virtual privilege levels**: Guest user, guest supervisor, host (hypervisor)
- **Two-stage address translation**: Guest Virtual → Guest Physical → Host Physical
- **Nested page tables** for guest memory management
- **Virtual interrupts** and exception delegation
- **VMCS (Virtual Machine Control Structure)** per guest

### 3.2 Hypervisor Special Registers

- **GUESTCTL0-3**: Guest control registers
- **GTOFFSET**: Guest timer offset
- **GTLBC**: Guest TLB control
- **GCONFIG**: Guest configuration
- **VMPRIOMASK**: VM priority mask

### 3.3 VM Exit/Entry

VM exits occur on:
- Privileged instruction execution
- External interrupts (when configured)
- Memory access violations
- I/O operations
- Explicit VMEXIT instruction

## 4. Vector and Matrix Extensions (ARM SVE/SME Style)

### 4.1 Scalable Vector Extension (SVE)

- **Variable vector length**: 128 to 2048 bits, runtime configurable
- **Predicate registers**: 64 predicate registers (P0-P63) for masked operations
- **Vector registers**: 256 vector registers (V0-V255)
- **Scalable operations**: Operations work on vectors of any implemented length

### 4.2 Scalable Matrix Extension (SME)

- **Matrix tiles**: Up to 8 matrix tiles (ZA0-ZA7)
- **Tile size**: Configurable from 128x128 to 2048x2048 bits
- **Streaming mode**: Separate register state for matrix operations
- **Matrix operations**: Outer product, accumulation, load/store

### 4.3 Vector Instructions

- **Arithmetic**: ADD, SUB, MUL, DIV, FADD, FSUB, FMUL, FDIV
- **Logical**: AND, ORR, EOR, BIC
- **Comparison**: CMP, FCMP with predicate result
- **Load/Store**: Contiguous, strided, gather/scatter
- **Reduction**: ADDV, MAXV, MINV
- **Permutation**: ZIP, UZP, TRN, REV

## 5. Application Binary Interface (ABI)

### 5.1 64-bit ABI

**Register Usage**:
- $0: Always zero
- $1-$8: Argument registers
- $9-$15: Temporary registers
- $16-$23: Saved registers (callee-saved)
- $24-$31: Reserved for special purposes
- $254: Stack pointer
- $255: Return address

**Calling Convention**:
- First 8 arguments in $1-$8
- Additional arguments on stack
- Return value in $1 (and $2 for 128-bit values)
- Stack 16-byte aligned

### 5.2 32-bit ABI (x32-style)

**Differences from 64-bit**:
- Uses 32-bit pointers and sizes
- 64-bit arithmetic operations still available
- Reduced memory footprint
- Same register file and calling convention

### 5.3 Endianness

- **Big-endian mode**: Traditional MMIX (default for compatibility)
- **Little-endian mode**: Modern systems compatibility
- **Per-mode configuration**: Separate settings for kernel and user space
- **Endian switch**: Via special register or privileged instruction

## 6. Compressed Instructions (16-bit Encoding)

### 6.1 Design Principles

- **Common operations**: Most frequent instructions get compressed forms
- **Register constraints**: Limited to registers $0-$15 or $16-$31
- **Immediate size**: Reduced immediate values
- **Alignment**: 16-bit aligned, can mix with 32-bit instructions

### 6.2 Compressed Instruction Formats

**Format CR (Register)**:
```
15:12 - opcode[3:0]
11:8  - rs2[3:0]
7:4   - rs1/rd[3:0]
3:0   - sub-opcode[3:0]
```

**Format CI (Immediate)**:
```
15:12 - opcode[3:0]
11:8  - imm[7:4]
7:4   - rs1/rd[3:0]
3:0   - imm[3:0]
```

**Format CJ (Jump)**:
```
15:12 - opcode[3:0]
11:0  - offset[11:0]
```

### 6.3 Compressed Instruction Set

- **C.ADD**: Add registers
- **C.ADDI**: Add immediate
- **C.SUB**: Subtract registers
- **C.LD**: Load doubleword
- **C.ST**: Store doubleword
- **C.LW**: Load word
- **C.SW**: Store word
- **C.J**: Jump
- **C.JR**: Jump register
- **C.BR**: Branch
- **C.MV**: Move register
- **C.LI**: Load immediate

## 7. Floating-Point Unit (FPU)

### 7.1 IEEE 754-2008 Compliance

Full support for:
- **Binary formats**: Binary16, Binary32, Binary64, Binary128
- **Decimal formats**: Decimal32, Decimal64, Decimal128
- **ML formats**: FP8 (E4M3, E5M2), BFloat16

### 7.2 Floating-Point Registers

- **256 FP registers**: F0-F255 (each 128-bit for quad precision)
- **FP status register**: Flags for inexact, underflow, overflow, divide-by-zero, invalid
- **Rounding modes**: Round to nearest (ties to even/away), toward zero, toward +∞, toward -∞
- **Denormal support**: Full support for denormalized numbers

#### 7.2.1 FPR Aliasing Mode (Compatibility Mode)

MMIX supports an optional FPR aliasing mode for compatibility with architectures that lacked separate floating-point register files (e.g., early RISC architectures where FP values were stored in GPRs).

**When Enabled** (`FprAliasedToGpr = TRUE`):
- F0-F255 are aliased to $0-$255 (general-purpose registers)
- No separate FP register file; FP operations use GPRs
- FP values are stored in lower 64 bits of GPRs
- Compatible with single-precision (32-bit) and double-precision (64-bit) FP
- Quad-precision (128-bit) operations require register pairs

**When Disabled** (`FprAliasedToGpr = FALSE`, default):
- F0-F255 are independent 128-bit FP registers
- Full quad-precision support per register
- Separate FP register file (modern architecture)

**Use Cases**:
- Porting legacy code from MIPS I/II (FP in GPRs)
- Compatibility with early SPARC (FP registers overlaid on integer registers)
- Simplified hardware implementations (no separate FP register file)
- Reduced die area for embedded systems

**Performance Considerations**:
- Aliased mode: No register moves needed between FP and integer operations
- Separate mode: Parallel FP and integer execution possible
- Aliased mode: Register pressure increased (shared register file)

### 7.3 FPU Instructions

**Basic Arithmetic**:
- FADD, FSUB, FMUL, FDIV, FSQRT
- FMA (Fused Multiply-Add)
- FMIN, FMAX

**Conversions**:
- FCVT.{format1}.{format2}
- FCVT.W.{format} (float to integer)
- FCVT.{format}.W (integer to float)

**Comparisons**:
- FEQ, FLT, FLE (with NaN handling)

**Decimal Operations**:
- DADD, DSUB, DMUL, DDIV (decimal arithmetic)
- DQUANTIZE, DROUND (decimal quantization)

**ML Formats**:
- FP8.ADD, FP8.MUL, FP8.FMA
- BF16.ADD, BF16.MUL, BF16.FMA

## 8. Neural Network / Machine Learning Instructions

### 8.1 Matrix Operations

**Matrix Multiply**:
- **MMUL.{type}**: Matrix multiply for various types (INT8, INT16, FP16, BF16, FP32)
- **MMLA**: Matrix multiply-accumulate
- **Tile size**: Configurable tiles (e.g., 4x4, 8x8, 16x16)

### 8.2 Convolution Operations

**Convolution Instructions**:
- **CONV2D**: 2D convolution
- **DEPTHCONV**: Depthwise convolution
- **CONV3D**: 3D convolution
- **Configurable**: Stride, padding, dilation

### 8.3 Activation Functions

**Hardware-accelerated activations**:
- **RELU**: Rectified Linear Unit
- **GELU**: Gaussian Error Linear Unit
- **SIGMOID**: Sigmoid function
- **TANH**: Hyperbolic tangent
- **SOFTMAX**: Softmax normalization

### 8.4 Pooling Operations

- **MAXPOOL**: Max pooling
- **AVGPOOL**: Average pooling
- **GLOBPOOL**: Global pooling

### 8.5 Normalization

- **BATCHNORM**: Batch normalization
- **LAYERNORM**: Layer normalization
- **GROUPNORM**: Group normalization

### 8.6 Quantization

- **QUANTIZE**: Quantize FP32 to INT8/INT16
- **DEQUANTIZE**: Dequantize INT8/INT16 to FP32
- **QDOT**: Quantized dot product

## 9. Hardware Emulation

### 9.1 System Architecture

```
┌─────────────────────────────────────────────┐
│              MMIX CPU Core                  │
│  ┌────────┐  ┌──────┐  ┌─────────────┐    │
│  │  ALU   │  │ FPU  │  │   Vector    │    │
│  └────────┘  └──────┘  │   Engine    │    │
│  ┌────────┐  ┌──────┐  └─────────────┘    │
│  │  MMU   │  │ TLB  │                      │
│  └────────┘  └──────┘                      │
└─────────────────┬───────────────────────────┘
                  │
        ┌─────────┴─────────┐
        │    PCIe Root      │
        │     Complex       │
        └─────────┬─────────┘
                  │
    ┌─────────────┼─────────────┬──────────┬──────────┐
    │             │             │          │          │
┌───┴───┐   ┌────┴────┐   ┌────┴───┐  ┌──┴──┐   ┌───┴────┐
│  AIC  │   │Framebuf │   │ ADB    │  │Zilog│   │vmxnet3 │
│  IRQ  │   │ Device  │   │  Bus   │  │UART │   │ NIC    │
└───────┘   └─────────┘   └────────┘  └─────┘   └────────┘
```

### 9.2 Apple AIC (ARM Interrupt Controller)

**Features**:
- **Fast interrupt routing**: Low-latency interrupt delivery
- **Priority levels**: 16 priority levels
- **Interrupt types**: Edge-triggered, level-sensitive
- **Per-CPU interrupts**: Support for multi-core systems
- **IPI (Inter-Processor Interrupts)**: For SMP synchronization

**Registers**:
- **AIC_WHOAMI**: CPU identification
- **AIC_EVENT**: Current event/interrupt
- **AIC_IPI_SET**: Set IPI for target CPU
- **AIC_IPI_CLR**: Clear IPI
- **AIC_TARGET**: Interrupt routing table
- **AIC_PRIORITY**: Priority configuration

### 9.3 Simple Framebuffer

**Configuration**:
- **Resolution**: Configurable (default 1024x768)
- **Color depth**: 16/24/32-bit
- **Linear framebuffer**: Direct memory mapping
- **Double buffering**: Optional for smooth updates

**Memory Layout**:
```
Base Address: Configurable via PCI BAR
Size: width × height × bytes_per_pixel
Format: RGBA/BGRA/RGB565
```

### 9.4 ADB (Apple Desktop Bus)

**Devices**:
- **Keyboard**: Standard keyboard with scan codes
- **Mouse**: 3-button mouse with scroll wheel
- **Bus protocol**: Polled or interrupt-driven

**Registers**:
- **ADB_CMD**: Command register
- **ADB_STATUS**: Status register
- **ADB_DATA**: Data buffer
- **ADB_INT**: Interrupt enable/status

### 9.5 VMXNet3 Ethernet Controller

**Features**:
- **Paravirtualized NIC**: Optimized for virtualized environments
- **Multiple queues**: TX/RX queue pairs for performance
- **Offload support**: Checksum, TSO, LRO
- **Jumbo frames**: Up to 9000 bytes
- **VLAN support**: 802.1Q tagging

**Register Layout**:
- **VMXNET3_BAR0**: Device registers
- **VMXNET3_BAR1**: MSI-X vectors
- **Queues**: Ring buffer based

### 9.6 Zilog Serial Interfaces

**UART Configuration**:
- **Baud rates**: 300 to 115200 bps
- **Data format**: 5/6/7/8 data bits
- **Parity**: None, Even, Odd, Mark, Space
- **Stop bits**: 1, 1.5, 2
- **Flow control**: None, XON/XOFF, RTS/CTS

**Registers** (per UART):
- **DATA**: Data register
- **IER**: Interrupt Enable Register
- **IIR**: Interrupt Identification Register
- **LCR**: Line Control Register
- **MCR**: Modem Control Register
- **LSR**: Line Status Register
- **MSR**: Modem Status Register

### 9.7 PCIe Infrastructure

**Configuration Space**:
- **Type 0 Header**: For endpoints
- **Type 1 Header**: For bridges
- **Capability List**: Extended capabilities

**Base Address Registers (BARs)**:
- 6 BARs per device
- Support for memory and I/O mapping
- 32-bit and 64-bit addressing

**Interrupt Routing**:
- Legacy INTx (INTA-INTD)
- MSI (Message Signaled Interrupts)
- MSI-X (Extended MSI)

## 10. Coding Style and Documentation

### 10.1 NT Coding Style

**Naming Conventions**:
- **Types**: PascalCase with type suffix (e.g., `MMIX_CPU_STATE`, `PAGE_TABLE_ENTRY`)
- **Functions**: PascalCase with module prefix (e.g., `MmixExecuteInstruction`, `MmuTranslateAddress`)
- **Variables**: camelCase for locals, PascalCase for globals
- **Constants**: UPPER_CASE with underscores
- **Macros**: UPPER_CASE with underscores

**Function Pointers**:
```c
typedef
UINT64
(*MMIX_INSTRUCTION_HANDLER)(
    IN OUT MMIX_CPU_STATE *CpuState,
    IN UINT32 Instruction
    );
```

### 10.2 UEFI Commenting Style

**File Header**:
```c
/** @file
  Brief description of the file.

  Detailed description of the file, including purpose,
  dependencies, and design notes.

  Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/
```

**Function Header**:
```c
/**
  Brief description of the function.

  Detailed description including algorithm, side effects,
  and usage notes.

  @param[in]      ParameterName   Description of input parameter.
  @param[out]     ParameterName   Description of output parameter.
  @param[in,out]  ParameterName   Description of in/out parameter.

  @retval  RETURN_SUCCESS         Success condition.
  @retval  RETURN_ERROR_CODE      Error condition.

**/
```

**Structure Comments**:
```c
/**
  Brief description of the structure.

  Detailed description.
**/
typedef struct {
  /// Brief description of field1
  UINT64    Field1;

  /// Brief description of field2
  UINT32    Field2;

  /// Brief description of field3
  /// Additional details if needed
  UINT16    Field3;
} STRUCTURE_NAME;
```

## 11. Build System and Integration

### 11.1 Directory Structure

```
mmix-emulator/
├── docs/                    # Documentation
│   ├── ARCHITECTURE.md      # This file
│   ├── ISA.md              # Instruction set reference
│   └── API.md              # API documentation
├── include/                 # Public headers
│   ├── MmixEmulator.h      # Main emulator API
│   ├── MmixTypes.h         # Type definitions
│   └── devices/            # Device headers
├── src/                     # Source code
│   ├── core/               # MMIX core
│   │   ├── Cpu.c
│   │   ├── Decoder.c
│   │   └── Execute.c
│   ├── memory/             # Memory management
│   │   ├── Mmu.c
│   │   ├── PageTable.c
│   │   └── Tlb.c
│   ├── hypervisor/         # Virtualization
│   │   ├── Vmx.c
│   │   └── GuestState.c
│   ├── vector/             # Vector extensions
│   │   ├── Sve.c
│   │   └── Sme.c
│   ├── fpu/                # Floating-point
│   │   ├── Fpu.c
│   │   ├── Decimal.c
│   │   └── MlFormats.c
│   ├── ml/                 # ML instructions
│   │   ├── MatrixOps.c
│   │   ├── Convolution.c
│   │   └── Activation.c
│   ├── devices/            # Hardware devices
│   │   ├── Aic.c
│   │   ├── Framebuffer.c
│   │   ├── Adb.c
│   │   ├── Vmxnet3.c
│   │   └── Zilog.c
│   ├── pcie/               # PCIe infrastructure
│   │   ├── PcieRoot.c
│   │   └── ConfigSpace.c
│   └── main.c              # Emulator entry point
├── tests/                   # Test suite
├── scripts/                 # Build scripts
├── Makefile                 # Build configuration
└── README.md               # Project readme
```

### 11.2 Build Configuration

**Compiler Flags**:
- `-std=c11`: C11 standard
- `-Wall -Wextra`: All warnings
- `-O2`: Optimization level 2 for release
- `-g`: Debug symbols for debug builds

**Dependencies**:
- Standard C library
- Optional: SDL2 for graphics display
- Optional: libpcap for network

## 12. Implementation Phases

### Phase 1: Core MMIX
- Basic CPU state and registers
- Instruction decoder
- Basic arithmetic and logical operations
- Memory load/store

### Phase 2: Memory Management
- Page table implementation
- TLB with address translation
- Protection checks

### Phase 3: Extended Features
- Endian switching
- Compressed instructions
- Privilege levels

### Phase 4: Virtualization
- Hypervisor mode
- Guest state management
- Two-stage translation

### Phase 5: Vector Extensions
- SVE implementation
- SME implementation
- Predicate operations

### Phase 6: FPU
- IEEE 754 binary formats
- Decimal floating-point
- ML formats (FP8, BFloat16)

### Phase 7: ML Acceleration
- Matrix operations
- Convolution
- Activation functions

### Phase 8: Hardware Devices
- PCIe infrastructure
- AIC interrupt controller
- Framebuffer
- ADB bus
- Networking
- Serial ports

### Phase 9: Integration and Testing
- System integration
- Performance optimization
- Test suite
- Documentation

## 13. Testing Strategy

### 13.1 Unit Tests
- Individual instruction testing
- Memory management tests
- Device tests

### 13.2 Integration Tests
- Full system boot
- Device interaction
- Multi-core scenarios

### 13.3 Performance Tests
- Instruction throughput
- Memory bandwidth
- Device I/O performance

### 13.4 Compliance Tests
- IEEE 754 compliance
- MMIX ISA compliance
- PCIe specification compliance

## 14. Future Enhancements

- **Multi-core support**: SMP with cache coherency
- **GPU emulation**: Integrated graphics processor
- **Storage controllers**: NVMe, AHCI
- **USB support**: USB 2.0/3.0 controllers
- **Audio**: HD Audio controller
- **Debugging**: GDB remote protocol support
- **Trace generation**: Instruction trace for analysis

## 15. References

- Donald Knuth, "MMIX - A RISC Computer for the New Millennium"
- IEEE 754-2008 Standard for Floating-Point Arithmetic
- ARM Architecture Reference Manual (for SVE/SME)
- MIPS Architecture for Programmers Volume III (for VZ)
- PCI Express Base Specification
- Apple AIC Documentation
- VMware VMXNet3 Specification
- Zilog Z8530 Serial Communications Controller
