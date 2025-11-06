# MMIX Emulator

A comprehensive MMIX emulator implementing Donald Knuth's MMIX architecture with modern extensions including virtualization, vector processing, machine learning acceleration, and complete hardware emulation.

## Overview

This emulator implements:

- **MMIX Core**: Full 64-bit RISC architecture with 256 general-purpose registers
- **Memory Management**: Virtual memory with 4-level page tables and TLB
- **Endian Switching**: Support for both big-endian and little-endian modes with runtime host detection
- **MIX Compatibility**: Run legacy MIX programs from "The Art of Computer Programming"
- **Hypervisor**: MIPS VZ-style virtualization extensions
- **Vector Extensions**: ARM SVE-style scalable vector operations
- **Matrix Extensions**: ARM SME-style matrix operations
- **Modern ABI**: Both 64-bit and 32-bit (x32-style) ABIs
- **Compressed Instructions**: 16-bit instruction encoding for code density
- **Advanced FPU**: IEEE 754-2008 compliant with decimal, FP16, FP8, BFloat16
- **ML/AI Instructions**: Neural network acceleration primitives
- **Hardware Emulation**: PCIe-based machine with complete device emulation

## Features

### CPU Architecture

- 256 general-purpose registers (64-bit)
- 32 special registers (rA-rZZ)
- 256 floating-point registers (128-bit for quad precision)
- 256 vector registers (scalable 128-2048 bits)
- 64 predicate registers (for masked operations)
- 8 matrix tile registers (for matrix operations)

### Instruction Set

- **Standard MMIX**: All original MMIX instructions
- **Compressed**: 16-bit instruction encoding
- **Vector**: Scalable vector operations (SVE-style)
- **Matrix**: Matrix tile operations (SME-style)
- **ML/AI**: Convolution, pooling, activation functions, normalization, quantization

### Memory System

- 4-level page tables (PML4/PDPT/PD/PT)
- Page sizes: 4KB, 2MB, 1GB, 512GB
- 256-entry TLB with ASID support
- Memory protection and permissions
- MMIO region support

### Floating-Point

- IEEE 754-2008 binary formats (16/32/64/128-bit)
- IEEE 754-2008 decimal formats (32/64/128-bit)
- Machine learning formats (FP8, BFloat16)
- Full denormal support
- All rounding modes

### Hardware Devices

- **Apple AIC**: Interrupt controller
- **Simple Framebuffer**: Display device
- **ADB Bus**: Keyboard and mouse
- **VMXNet3**: Paravirtualized network adapter
- **Zilog Serial**: UART interfaces
- **PCIe Infrastructure**: Device bus

## Building

### Requirements

- GCC or Clang with C11 support
- Make
- Standard C library

### Compilation

```bash
# Build emulator
make

# Build with debug symbols
make debug

# Build optimized release
make release

# Clean build artifacts
make clean
```

The compiled binary will be in `bin/mmix-emulator`.

## Usage

### Basic Usage

```bash
./bin/mmix-emulator [options] <image>
```

### Options

- `-m <size>` - Set memory size in MB (default: 256)
- `-pc <addr>` - Set initial PC address (default: 0x0)
- `-vl <bytes>` - Set vector length in bytes (default: 128)
- `-step` - Enable single-step mode for debugging
- `-dump` - Dump CPU and memory state after execution
- `-h, --help` - Show help message

### Examples

```bash
# Run a binary with 512MB memory
./bin/mmix-emulator -m 512 program.bin

# Run in single-step mode for debugging
./bin/mmix-emulator -step -dump test.bin

# Run with custom initial PC
./bin/mmix-emulator -pc 0x1000 bootloader.bin
```

## Architecture

### Directory Structure

```
mmix-emulator/
├── docs/                    # Documentation
│   ├── ARCHITECTURE.md      # Architecture overview
│   ├── ISA.md              # Instruction set reference
│   ├── KESU_EXTENSION.md   # KESU 4-ring protection
│   └── MMIX_COMPLETE_REFERENCE.md  # Complete reference guide
├── include/                 # Public headers
│   ├── MmixEmulator.h      # Main API
│   ├── MmixTypes.h         # Type definitions
│   ├── MmixCore.h          # CPU core
│   ├── MmixMemory.h        # Memory management
│   └── devices/            # Device headers
├── src/                     # Source code
│   ├── main.c              # Entry point
│   ├── Emulator.c          # Emulator context
│   ├── core/               # CPU core
│   ├── memory/             # Memory management
│   ├── hypervisor/         # Virtualization
│   ├── vector/             # Vector extensions
│   ├── fpu/                # Floating-point
│   ├── ml/                 # ML instructions
│   ├── devices/            # Hardware devices
│   └── pcie/               # PCIe infrastructure
├── Makefile                 # Build configuration
└── README.md               # This file
```

### Key Components

#### CPU Core (`src/core/`)

Implements the MMIX CPU with all registers, execution modes, and instruction handling.

#### Memory Management (`src/memory/`)

Virtual memory with page tables, TLB, and physical memory emulation.

#### Execution Engine (`src/core/Execute.c`)

Instruction decoder and execution handlers for all instruction types.

#### Emulator Context (`src/Emulator.c`)

Top-level emulator management, configuration, and lifecycle.

## Coding Style

The project follows **NT (Windows NT kernel) coding style** with **UEFI commenting conventions**:

### Naming Conventions

- **Types**: PascalCase with type suffix (e.g., `MMIX_CPU_STATE`)
- **Functions**: PascalCase with module prefix (e.g., `MmixCpuExecuteInstruction`)
- **Variables**: camelCase for locals, PascalCase for globals
- **Constants**: UPPER_CASE with underscores

### Documentation

All functions include UEFI-style documentation:

```c
/**
  Brief description of the function.

  Detailed description including algorithm, side effects,
  and usage notes.

  @param[in]      ParameterName   Description.
  @param[out]     ParameterName   Description.
  @param[in,out]  ParameterName   Description.

  @retval  MMIX_SUCCESS           Success condition.
  @retval  MMIX_ERROR_CODE        Error condition.

**/
```

## Implementation Status

### Completed

- ✅ CPU state management
- ✅ General and special register access
- ✅ Basic instruction execution (load, store, arithmetic, logical, branch)
- ✅ Memory subsystem with physical memory
- ✅ TLB implementation
- ✅ Endianness conversion functions
- ✅ Exception and interrupt handling
- ✅ Emulator context and lifecycle
- ✅ Command-line interface

### In Progress / TODO

- ⚠️ Full instruction set implementation
- ⚠️ Page table walking
- ⚠️ Floating-point operations (IEEE 754-2008)
- ⚠️ Decimal floating-point
- ⚠️ ML format support (FP8, BFloat16)
- ⚠️ Vector operations (SVE-style)
- ⚠️ Matrix operations (SME-style)
- ⚠️ ML/AI instructions
- ⚠️ Compressed instruction execution
- ⚠️ Hypervisor extensions
- ⚠️ Device emulation (AIC, framebuffer, network, serial)
- ⚠️ PCIe infrastructure
- ⚠️ Test suite

## Documentation

Comprehensive documentation is available in the `docs/` directory:

- **docs/ARCHITECTURE.md**: Complete architecture documentation including CPU, memory, virtualization, vector/matrix extensions, FPU, ML instructions, and hardware devices
- **docs/ISA.md**: Instruction set reference with encoding, semantics, and examples
- **docs/KESU_EXTENSION.md**: VMS-style 4-ring protection with per-ring endianness, stack direction, and page tables
- **docs/MMIX_COMPLETE_REFERENCE.md**: Comprehensive 979-line reference consolidating all documentation
- **STATUS.md**: Current project status, recent accomplishments, and implementation progress

## Contributing

Contributions are welcome! Please ensure:

1. Code follows NT coding style and UEFI commenting conventions
2. All functions are properly documented
3. Changes are tested
4. Commit messages are descriptive

## License

BSD 2-Clause License

Copyright (c) 2025, MMIX Emulator Project. All rights reserved.

## References

- Donald Knuth, "MMIX - A RISC Computer for the New Millennium"
- IEEE 754-2008 Standard for Floating-Point Arithmetic
- ARM Architecture Reference Manual (SVE/SME)
- MIPS Architecture for Programmers Volume III (VZ Extensions)
- PCI Express Base Specification

## Contact

For questions, issues, or contributions, please open an issue on the project repository.
