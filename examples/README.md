# MMIX Emulator Examples

This directory contains example programs for the MMIX emulator.

## Test Programs

### simple_test.asm

A simple test program demonstrating:
- Integer arithmetic (ADD, SUB, MUL)
- Memory operations (LDO, STO)
- Comparison and branching (CMP, BP)
- Floating-point operations (FADD, FMUL, FSQRT)

### Creating Test Binaries

Since a full MMIX assembler is not included, you can create simple test binaries manually:

#### Method 1: Hex Editor

Use a hex editor to create raw binary files with MMIX instructions.

Example: Simple ADD instruction
```
Opcode: 0x20 (ADD)
X: 0x01 (destination $1)
Y: 0x02 (source $2)
Z: 0x03 (source $3)

Binary: 20 01 02 03
```

#### Method 2: Python Script

Create a simple Python script to generate test binaries:

```python
#!/usr/bin/env python3
import struct

def mmix_instruction(opcode, x, y, z):
    """Create a 32-bit MMIX instruction"""
    return struct.pack('>I', (opcode << 24) | (x << 16) | (y << 8) | z)

# Example: ADD $1,$2,$3
with open('test.bin', 'wb') as f:
    f.write(mmix_instruction(0x20, 1, 2, 3))  # ADD
    f.write(mmix_instruction(0x21, 1, 1, 10)) # ADDI $1,$1,10
    f.write(mmix_instruction(0x16, 1, 0, 0))  # STO $1,$0,0
```

#### Method 3: C Program

Compile and run a C program to generate test binaries:

```c
#include <stdio.h>
#include <stdint.h>

void write_inst(FILE *f, uint8_t op, uint8_t x, uint8_t y, uint8_t z) {
    uint32_t inst = (op << 24) | (x << 16) | (y << 8) | z;
    uint8_t bytes[4];
    bytes[0] = op;
    bytes[1] = x;
    bytes[2] = y;
    bytes[3] = z;
    fwrite(bytes, 1, 4, f);
}

int main() {
    FILE *f = fopen("test.bin", "wb");
    write_inst(f, 0x20, 1, 2, 3);  // ADD $1,$2,$3
    fclose(f);
    return 0;
}
```

## Running Examples

```bash
# Build the emulator
make

# Run a test program
./bin/mmix-emulator -m 256 -dump examples/test.bin
```

## Instruction Encoding Reference

### Standard 32-bit Format

```
31    24 23    16 15     8 7      0
┌────────┬────────┬────────┬────────┐
│   OP   │   X    │   Y    │   Z    │
└────────┴────────┴────────┴────────┘
```

### Common Opcodes

| Opcode | Instruction | Description |
|--------|-------------|-------------|
| 0x00   | LDB         | Load byte |
| 0x06   | LDO         | Load octa |
| 0x10   | STB         | Store byte |
| 0x16   | STO         | Store octa |
| 0x20   | ADD         | Add |
| 0x21   | ADDI        | Add immediate |
| 0x24   | SUB         | Subtract |
| 0x28   | MUL         | Multiply |
| 0x2C   | DIV         | Divide |
| 0x3C   | CMP         | Compare |
| 0x40   | AND         | Bitwise AND |
| 0x42   | OR          | Bitwise OR |
| 0x44   | XOR         | Bitwise XOR |
| 0x52   | BZ          | Branch if zero |
| 0x54   | BP          | Branch if positive |
| 0x60   | FADD        | FP add |
| 0x62   | FMUL        | FP multiply |
| 0x63   | FDIV        | FP divide |
| 0x65   | FSQRT       | FP square root |

## Compressed Instructions

Compressed 16-bit instructions have opcode >= 0x8:

```
15    12 11     8 7      4 3      0
┌────────┬────────┬────────┬────────┐
│  c.OP  │  Field1│  Field2│  Field3│
└────────┴────────┴────────┴────────┘
```

### Common Compressed Opcodes

| Opcode | Instruction | Description |
|--------|-------------|-------------|
| 0x8000 | C.ADD       | Compressed add |
| 0x9000 | C.ADDI      | Compressed add immediate |
| 0xA000 | C.LDO       | Compressed load octa |
| 0xB000 | C.STO       | Compressed store octa |
| 0xC000 | C.J         | Compressed jump |

## Creating Comprehensive Tests

To test various features:

1. **Arithmetic**: Test all arithmetic operations with edge cases
2. **Memory**: Test various load/store sizes and alignments
3. **Branches**: Test all branch conditions
4. **FP**: Test floating-point with special values (NaN, Inf, denormals)
5. **Exceptions**: Test exception handling (divide by zero, page faults)
6. **Compressed**: Test compressed instruction execution
7. **Endianness**: Test both big and little-endian modes

## Debugging

Use the `-step` flag for single-stepping:

```bash
./bin/mmix-emulator -step -dump test.bin
```

Use the `-dump` flag to see register state after execution:

```bash
./bin/mmix-emulator -dump test.bin
```
