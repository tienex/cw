# MIX Compatibility Mode

## Overview

The MIX compatibility layer allows the MMIX emulator to run programs written for Donald Knuth's original MIX computer, as described in "The Art of Computer Programming" volumes 1-3 (before MMIX was designed).

## MIX Architecture Summary

MIX is a hypothetical computer with:

- **Word-addressable memory**: 4000 words
- **Word size**: 5 bytes (30 bits) + sign bit
- **Byte size**: 6 bits (values 0-63)
- **Arithmetic**: Both decimal and binary modes
- **I/O**: Tape, disk, printer, card reader, etc.

### MIX Registers

| Register | Size | Purpose |
|----------|------|---------|
| A | 5 bytes | Accumulator |
| X | 5 bytes | Extension (for multiplication/division) |
| I1-I6 | 2 bytes | Index registers |
| J | 2 bytes | Jump address (saved return address) |

### Special Indicators

- **Comparison indicator**: LESS, EQUAL, GREATER
- **Overflow toggle**: Set when arithmetic overflow occurs

## MMIX Register Mapping

MIX registers are mapped to MMIX general-purpose registers:

```
$1  = A    (Accumulator, 5 bytes + sign)
$2  = X    (Extension, 5 bytes + sign)
$3  = I1   (Index register 1, 2 bytes)
$4  = I2   (Index register 2, 2 bytes)
$5  = I3   (Index register 3, 2 bytes)
$6  = I4   (Index register 4, 2 bytes)
$7  = I5   (Index register 5, 2 bytes)
$8  = I6   (Index register 6, 2 bytes)
$9  = J    (Jump address, 2 bytes)
$10 = CMP  (Comparison indicator: -1, 0, +1)
$11 = OVR  (Overflow toggle: 0 or 1)
```

## Memory Layout

MIX uses word-addressable memory (4000 words), which is mapped to MMIX byte-addressable memory:

- **MIX address 0** → MMIX address 0x0000 (bytes 0-7)
- **MIX address 1** → MMIX address 0x0008 (bytes 8-15)
- **MIX address N** → MMIX address N×8 (bytes N×8 to N×8+7)

Each MIX word occupies 8 bytes in MMIX memory (padded for alignment).

## MIX Word Format

MIX words consist of 5 bytes plus a sign:

```
[Sign] [Byte 1] [Byte 2] [Byte 3] [Byte 4] [Byte 5]
  ±      0-63     0-63     0-63     0-63     0-63

Total magnitude: 64^5 = 1,073,741,824 (30 bits)
```

### Field Specification (F)

MIX instructions use field specifications (L:R) to select portions of words:

- **0:0** - Sign only
- **1:5** - All 5 bytes (magnitude)
- **0:5** - Sign + all 5 bytes (full word)
- **1:1** - Byte 1 only
- **4:5** - Bytes 4-5 (last two bytes)

## API Functions

### Enable MIX Mode

```c
MMIX_STATUS MmixEnableMixMode(MMIX_CPU_STATE *CpuState);
```

Enables MIX compatibility mode and initializes MIX registers.

### Disable MIX Mode

```c
MMIX_STATUS MmixDisableMixMode(MMIX_CPU_STATE *CpuState);
```

Returns to native MMIX operation.

### Read MIX Register

```c
MMIX_STATUS MmixReadMixRegister(
  MMIX_CPU_STATE *CpuState,
  UINT8          Register,
  UINT64         *Value
);
```

Reads a MIX register (A, X, I1-I6, J, CMP, OVR).

**Example:**
```c
UINT64 Accumulator;
MmixReadMixRegister(CpuState, MIX_REG_A, &Accumulator);
```

### Write MIX Register

```c
MMIX_STATUS MmixWriteMixRegister(
  MMIX_CPU_STATE *CpuState,
  UINT8          Register,
  UINT64         Value
);
```

Writes to a MIX register with automatic size constraints.

**Example:**
```c
// Set index register I1 to 100
MmixWriteMixRegister(CpuState, MIX_REG_I1, 100);
```

### Convert MIX Word

```c
UINT64 MmixMixWordToUint64(CONST MIX_WORD *MixWord);
VOID MmixUint64ToMixWord(UINT64 Value, MIX_WORD *MixWord);
```

Converts between MIX word format and MMIX 64-bit values.

## Usage Example

```c
#include "MmixCore.h"
#include "MmixMix.h"

// Create CPU state
MMIX_CPU_STATE *Cpu = MmixCpuCreate();

// Enable MIX compatibility mode
MmixEnableMixMode(Cpu);

// Set MIX accumulator to 1000
MmixWriteMixRegister(Cpu, MIX_REG_A, 1000);

// Set index register I1 to 50
MmixWriteMixRegister(Cpu, MIX_REG_I1, 50);

// Read accumulator
UINT64 A;
MmixReadMixRegister(Cpu, MIX_REG_A, &A);
printf("Accumulator = %llu\n", (unsigned long long)A);

// Return to MMIX mode
MmixDisableMixMode(Cpu);

MmixCpuDestroy(Cpu);
```

## Limitations

Current implementation provides:

1. ✅ Register mapping (A, X, I1-I6, J, comparison, overflow)
2. ✅ Word format conversion (5 bytes + sign)
3. ✅ Memory layout (word-addressable on byte-addressable memory)
4. ❌ MIX instruction execution (not yet implemented)
5. ❌ I/O device emulation (not yet implemented)
6. ❌ Decimal arithmetic mode (not yet implemented)

## Differences from Original MIX

1. **Memory size**: Can exceed 4000 words (uses MMIX memory)
2. **Byte values**: Internally stored as 8-bit, masked to 6-bit
3. **No timing**: MIX instruction timing not emulated
4. **No I/O**: Device units not implemented yet

## References

- Donald E. Knuth, "The Art of Computer Programming, Volume 1: Fundamental Algorithms", Section 1.3.1
- MIX instruction set (Appendix A of TAOCP Volume 1)
- MMIX specification: https://mmix.cs.hm.edu/

## Future Enhancements

Planned additions:

1. Full MIX instruction set interpreter
2. I/O device emulation (tape, disk, printer, etc.)
3. Decimal vs binary mode support
4. MIX assembly language support in mmix-as
5. MIX object file loader

## See Also

- `include/MmixMix.h` - MIX compatibility API
- `src/core/Mix.c` - Implementation
- `docs/ARCHITECTURE.md` - MMIX architecture overview
