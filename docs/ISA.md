# MMIX Instruction Set Architecture Reference

## 1. Instruction Encoding

### 1.1 Standard 32-bit Format

All standard MMIX instructions are 32 bits (4 bytes) wide:

```
 31    24 23    16 15     8 7      0
┌────────┬────────┬────────┬────────┐
│   OP   │   X    │   Y    │   Z    │  Register format
└────────┴────────┴────────┴────────┘
│   OP   │   X    │      YZ        │  Immediate format (16-bit)
└────────┴────────┴────────────────┘
│   OP   │          XYZ            │  Immediate format (24-bit)
└────────┴────────────────────────┘
```

- **OP**: 8-bit opcode
- **X**: 8-bit destination/source register
- **Y**: 8-bit source register or high immediate byte
- **Z**: 8-bit source register or low immediate byte
- **YZ**: 16-bit immediate value
- **XYZ**: 24-bit immediate value

### 1.2 Compressed 16-bit Format

Compressed instructions are 16 bits wide for improved code density:

```
 15    12 11     8 7      4 3      0
┌────────┬────────┬────────┬────────┐
│  c.OP  │  rs2   │  rs1   │  func  │  CR format
└────────┴────────┴────────┴────────┘
│  c.OP  │    imm[7:4]     │  imm  │  CI format
└────────┴────────┴────────┴────────┘
│  c.OP  │      offset[11:0]       │  CJ format
└────────┴──────────────────────────┘
```

### 1.3 Encoding Examples

**Example 1: ADD $5,$10,$15**
```
Instruction: ADD $5,$10,$15
Encoding:    0x20 05 0A 0F
Binary:      00100000 00000101 00001010 00001111
             ─┬────── ───┬─── ───┬─── ───┬───
              │         │       │       └─ Z = 15 ($15)
              │         │       └───────── Y = 10 ($10)
              │         └─────────────────── X = 5 ($5)
              └─────────────────────────────── OP = 0x20 (ADD)
```

**Example 2: ADDI $5,$10,42**
```
Instruction: ADDI $5,$10,42
Encoding:    0x21 05 0A 2A
Binary:      00100001 00000101 00001010 00101010
             ─┬────── ───┬─── ─────┬─────────
              │         │         └────────── YZ = 42 (immediate)
              │         └──────────────────── X = 5 ($5)
              └────────────────────────────── OP = 0x21 (ADDI)
```

**Example 3: SETL $5,0x1234**
```
Instruction: SETL $5,0x1234
Encoding:    0xE3 05 12 34
Binary:      11100011 00000101 00010010 00110100
             ─┬────── ───┬─── ─────┬─────────
              │         │         └────────── YZ = 0x1234 (immediate)
              │         └──────────────────── X = 5 ($5)
              └────────────────────────────── OP = 0xE3 (SETL)
```

**Example 4: JMP offset**
```
Instruction: JMP forward+100
Encoding:    0xF0 00 00 64
Binary:      11110000 00000000 00000000 01100100
             ─┬────── ─────────┬─────────────
              │                └───────────── XYZ = 100 (offset in tetras)
              └────────────────────────────── OP = 0xF0 (JMP)
```

**Example 5: LDO $1,$254,8** (Load from stack)
```
Instruction: LDO $1,$254,8
Encoding:    0x8B 01 FE 08
Binary:      10001011 00000001 11111110 00001000
             ─┬────── ───┬─── ───┬─── ───┬───
              │         │       │       └─ Z = 8 (offset)
              │         │       └───────── Y = 254 ($254, stack pointer)
              │         └─────────────────── X = 1 ($1)
              └─────────────────────────────── OP = 0x8B (LDO)
```

**Example 6: Compressed ADD** (when operands fit)
```
Instruction: ADD $5,$10,$15 (compressed)
Encoding:    0x80 0A
Binary:      10000000 00001010
             ─┬────── ───┬───
              │         └────── Compressed operands (4-bit registers)
              └──────────────── Compressed opcode
Note: Only possible if all registers fit in 4 bits
```

**Example 7: FADD F5,F10,F15** (Floating-point)
```
Instruction: FADD F5,F10,F15
Encoding:    0x60 05 0A 0F
Binary:      01100000 00000101 00001010 00001111
             ─┬────── ───┬─── ───┬─── ───┬───
              │         │       │       └─ Z = 15 (F15)
              │         │       └───────── Y = 10 (F10)
              │         └─────────────────── X = 5 (F5)
              └─────────────────────────────── OP = 0x60 (FADD)
```

**Example 8: VADD V5,V10,V15** (Vector unmasked)
```
Instruction: VADD V5,V10,V15
Encoding:    0xE0 05 0A 0F
Binary:      11100000 00000101 00001010 00001111
             ─┬────── ───┬─── ───┬─── ───┬───
              │         │       │       └─ VS2 = 15 (V15)
              │         │       └───────── VS1 = 10 (V10)
              │         └─────────────────── VD = 5 (V5)
              └─────────────────────────────── OP = 0xE0 (VADD)
```

**Example 9: VADD V5,V10,V15 with predicate P3** (Two-instruction sequence)
```
1. Set predicate:    SETP P3
   Encoding:         0xE7 03 00 00

2. Masked vector op: VADD.M V5,V10,V15
   Encoding:         0xE1 05 0A 0F

Note: Predicate-masked operations use implicit predicate register.
The .M suffix indicates masked operation (uses current predicate from SETP).
Alternative: Use rVP (vector predicate special register) to control masking.
```

## 2. Standard MMIX Opcode List

### 2.1 Load Instructions (0x00 - 0x0F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x00   | LDB      | $X,$Y,$Z | Load byte (1 byte) unsigned |
| 0x01   | LDBU     | $X,$Y,$Z | Load byte (1 byte) unsigned (alias) |
| 0x02   | LDW      | $X,$Y,$Z | Load wyde (2 bytes) unsigned |
| 0x03   | LDWU     | $X,$Y,$Z | Load wyde (2 bytes) unsigned (alias) |
| 0x04   | LDT      | $X,$Y,$Z | Load tetra (4 bytes) unsigned |
| 0x05   | LDTU     | $X,$Y,$Z | Load tetra (4 bytes) unsigned (alias) |
| 0x06   | LDO      | $X,$Y,$Z | Load octa (8 bytes) |
| 0x07   | LDOU     | $X,$Y,$Z | Load octa (8 bytes) unsigned (alias) |
| 0x08   | LDBS     | $X,$Y,$Z | Load byte (1 byte) signed |
| 0x09   | LDWS     | $X,$Y,$Z | Load wyde (2 bytes) signed |
| 0x0A   | LDTS     | $X,$Y,$Z | Load tetra (4 bytes) signed |
| 0x0B   | LDOS     | $X,$Y,$Z | Load octa (8 bytes) signed (alias) |
| 0x0C   | LDHT     | $X,$Y,$Z | Load high tetra (4 bytes) |
| 0x0D   | LDSF     | $X,$Y,$Z | Load short float (4 bytes) |
| 0x0E   | LDVTS    | $X,$Y,$Z | Load virtual translation (8 bytes) |
| 0x0F   | LDUNC    | $X,$Y,$Z | Load uncached (8 bytes) |

### 2.2 Store Instructions (0x10 - 0x1F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x10   | STB      | $X,$Y,$Z | Store byte (1 byte) |
| 0x11   | STBU     | $X,$Y,$Z | Store byte (1 byte) uncached |
| 0x12   | STW      | $X,$Y,$Z | Store wyde (2 bytes) |
| 0x13   | STWU     | $X,$Y,$Z | Store wyde (2 bytes) uncached |
| 0x14   | STT      | $X,$Y,$Z | Store tetra (4 bytes) |
| 0x15   | STTU     | $X,$Y,$Z | Store tetra (4 bytes) uncached |
| 0x16   | STO      | $X,$Y,$Z | Store octa (8 bytes) |
| 0x17   | STOU     | $X,$Y,$Z | Store octa (8 bytes) uncached |
| 0x18   | STBS     | $X,$Y,$Z | Store byte (1 byte) signed |
| 0x19   | STWS     | $X,$Y,$Z | Store wyde (2 bytes) signed |
| 0x1A   | STTS     | $X,$Y,$Z | Store tetra (4 bytes) signed |
| 0x1B   | STOS     | $X,$Y,$Z | Store octa (8 bytes) signed |
| 0x1C   | STHT     | $X,$Y,$Z | Store high tetra (4 bytes) |
| 0x1D   | STCO     | $X,$Y,$Z | Store conditional (8 bytes) |
| 0x1E   | STUNC    | $X,$Y,$Z | Store uncached (8 bytes) |
| 0x1F   | SYNCD    | $X,$Y,$Z | Synchronize data |

### 2.3 Arithmetic Instructions (0x20 - 0x3F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x20   | ADD      | $X,$Y,$Z | Add |
| 0x21   | ADDI     | $X,$Y,Z | Add immediate |
| 0x22   | ADDU     | $X,$Y,$Z | Add unsigned |
| 0x23   | ADDUI    | $X,$Y,Z | Add unsigned immediate |
| 0x24   | SUB      | $X,$Y,$Z | Subtract |
| 0x25   | SUBI     | $X,$Y,Z | Subtract immediate |
| 0x26   | SUBU     | $X,$Y,$Z | Subtract unsigned |
| 0x27   | SUBUI    | $X,$Y,Z | Subtract unsigned immediate |
| 0x28   | MUL      | $X,$Y,$Z | Multiply |
| 0x29   | MULI     | $X,$Y,Z | Multiply immediate |
| 0x2A   | MULU     | $X,$Y,$Z | Multiply unsigned |
| 0x2B   | MULUI    | $X,$Y,Z | Multiply unsigned immediate |
| 0x2C   | DIV      | $X,$Y,$Z | Divide |
| 0x2D   | DIVI     | $X,$Y,Z | Divide immediate |
| 0x2E   | DIVU     | $X,$Y,$Z | Divide unsigned |
| 0x2F   | DIVUI    | $X,$Y,Z | Divide unsigned immediate |
| 0x30   | NEG      | $X,Y,$Z | Negate |
| 0x31   | NEGI     | $X,Y,Z | Negate immediate |
| 0x32   | NEGU     | $X,Y,$Z | Negate unsigned |
| 0x33   | NEGUI    | $X,Y,Z | Negate unsigned immediate |
| 0x34   | SL       | $X,$Y,$Z | Shift left |
| 0x35   | SLI      | $X,$Y,Z | Shift left immediate |
| 0x36   | SLU      | $X,$Y,$Z | Shift left unsigned |
| 0x37   | SLUI     | $X,$Y,Z | Shift left unsigned immediate |
| 0x38   | SR       | $X,$Y,$Z | Shift right |
| 0x39   | SRI      | $X,$Y,Z | Shift right immediate |
| 0x3A   | SRU      | $X,$Y,$Z | Shift right unsigned |
| 0x3B   | SRUI     | $X,$Y,Z | Shift right unsigned immediate |
| 0x3C   | CMP      | $X,$Y,$Z | Compare |
| 0x3D   | CMPI     | $X,$Y,Z | Compare immediate |
| 0x3E   | CMPU     | $X,$Y,$Z | Compare unsigned |
| 0x3F   | CMPUI    | $X,$Y,Z | Compare unsigned immediate |

### 2.4 Logical Instructions (0x40 - 0x4F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x40   | AND      | $X,$Y,$Z | Bitwise AND |
| 0x41   | ANDI     | $X,$Y,Z | Bitwise AND immediate |
| 0x42   | OR       | $X,$Y,$Z | Bitwise OR |
| 0x43   | ORI      | $X,$Y,Z | Bitwise OR immediate |
| 0x44   | XOR      | $X,$Y,$Z | Bitwise XOR |
| 0x45   | XORI     | $X,$Y,Z | Bitwise XOR immediate |
| 0x46   | ANDN     | $X,$Y,$Z | Bitwise AND NOT |
| 0x47   | ANDNI    | $X,$Y,Z | Bitwise AND NOT immediate |
| 0x48   | ORN      | $X,$Y,$Z | Bitwise OR NOT |
| 0x49   | ORNI     | $X,$Y,Z | Bitwise OR NOT immediate |
| 0x4A   | NAND     | $X,$Y,$Z | Bitwise NAND |
| 0x4B   | NANDI    | $X,$Y,Z | Bitwise NAND immediate |
| 0x4C   | NOR      | $X,$Y,$Z | Bitwise NOR |
| 0x4D   | NORI     | $X,$Y,Z | Bitwise NOR immediate |
| 0x4E   | NXOR     | $X,$Y,$Z | Bitwise NXOR |
| 0x4F   | NXORI    | $X,$Y,Z | Bitwise NXOR immediate |

### 2.5 Branch Instructions (0x50 - 0x5F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x50   | BN       | $X,Y,Z | Branch if negative |
| 0x51   | BNB      | $X,Y,Z | Branch if negative backward |
| 0x52   | BZ       | $X,Y,Z | Branch if zero |
| 0x53   | BZB      | $X,Y,Z | Branch if zero backward |
| 0x54   | BP       | $X,Y,Z | Branch if positive |
| 0x55   | BPB      | $X,Y,Z | Branch if positive backward |
| 0x56   | BOD      | $X,Y,Z | Branch if odd |
| 0x57   | BODB     | $X,Y,Z | Branch if odd backward |
| 0x58   | BNN      | $X,Y,Z | Branch if non-negative |
| 0x59   | BNNB     | $X,Y,Z | Branch if non-negative backward |
| 0x5A   | BNZ      | $X,Y,Z | Branch if non-zero |
| 0x5B   | BNZB     | $X,Y,Z | Branch if non-zero backward |
| 0x5C   | BNP      | $X,Y,Z | Branch if non-positive |
| 0x5D   | BNPB     | $X,Y,Z | Branch if non-positive backward |
| 0x5E   | BEV      | $X,Y,Z | Branch if even |
| 0x5F   | BEVB     | $X,Y,Z | Branch if even backward |

### 2.6 Floating-Point Instructions (0x60 - 0x7F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x60   | FADD     | $X,$Y,$Z | Floating-point add |
| 0x61   | FSUB     | $X,$Y,$Z | Floating-point subtract |
| 0x62   | FMUL     | $X,$Y,$Z | Floating-point multiply |
| 0x63   | FDIV     | $X,$Y,$Z | Floating-point divide |
| 0x64   | FREM     | $X,$Y,$Z | Floating-point remainder |
| 0x65   | FSQRT    | $X,$Y,$Z | Floating-point square root |
| 0x66   | FINT     | $X,$Y,$Z | Floating-point round to integer |
| 0x67   | FIX      | $X,$Y,$Z | Convert float to integer |
| 0x68   | FIXU     | $X,$Y,$Z | Convert float to unsigned integer |
| 0x69   | FLOT     | $X,$Y,$Z | Convert integer to float |
| 0x6A   | FLOTU    | $X,$Y,$Z | Convert unsigned integer to float |
| 0x6B   | SFLOT    | $X,$Y,$Z | Convert signed integer to float (short) |
| 0x6C   | SFLOTU   | $X,$Y,$Z | Convert unsigned integer to float (short) |
| 0x6D   | FCMP     | $X,$Y,$Z | Floating-point compare |
| 0x6E   | FEQL     | $X,$Y,$Z | Floating-point equal |
| 0x6F   | FUN      | $X,$Y,$Z | Floating-point unordered |
| 0x70   | FCMPE    | $X,$Y,$Z | Floating-point compare with exception |
| 0x71   | FUNE     | $X,$Y,$Z | Floating-point unordered with exception |
| 0x72   | FEQLE    | $X,$Y,$Z | Floating-point equal with exception |
| 0x73   | FMAX     | $X,$Y,$Z | Floating-point maximum |
| 0x74   | FMIN     | $X,$Y,$Z | Floating-point minimum |
| 0x75   | FMA      | $X,$Y,$Z | Floating-point multiply-add |

## 3. Extended Instruction Set

### 3.1 Compressed Instructions

**Note**: Compressed instructions use the same mnemonics as full 32-bit instructions. The assembler automatically selects compressed (16-bit) encoding when operands fit within compressed format constraints. No special syntax is required.

| Encoding | Mnemonic | Format | Description |
|----------|----------|--------|-------------|
| 0x8000   | ADD    | $X,$Y,$Z | Add (compressed if operands fit) |
| 0x8100   | SUB    | $X,$Y,$Z | Subtract (compressed if operands fit) |
| 0x8200   | MUL    | $X,$Y,$Z | Multiply (compressed if operands fit) |
| 0x8300   | AND    | $X,$Y,$Z | Bitwise AND (compressed if operands fit) |
| 0x8400   | OR     | $X,$Y,$Z | Bitwise OR (compressed if operands fit) |
| 0x8500   | XOR    | $X,$Y,$Z | Bitwise XOR (compressed if operands fit) |
| 0x9000   | ADDI   | $X,$Y,Z | Add immediate (compressed if imm fits) |
| 0x9100   | SUBI   | $X,$Y,Z | Subtract immediate (compressed if imm fits) |
| 0x9200   | ANDI   | $X,$Y,Z | AND immediate (compressed if imm fits) |
| 0x9300   | ORI    | $X,$Y,Z | OR immediate (compressed if imm fits) |
| 0x9400   | XORI   | $X,$Y,Z | XOR immediate (compressed if imm fits) |
| 0x9500   | SETL   | $X,Z | Set low (compressed if fits) |
| 0xA000   | LDO    | $X,$Y,Z | Load octa (8 bytes, compressed if offset fits) |
| 0xA100   | LDT    | $X,$Y,Z | Load tetra (4 bytes, compressed if offset fits) |
| 0xA200   | LDW    | $X,$Y,Z | Load wyde (2 bytes, compressed if offset fits) |
| 0xA300   | LDB    | $X,$Y,Z | Load byte (1 byte, compressed if offset fits) |
| 0xB000   | STO    | $X,$Y,Z | Store octa (8 bytes, compressed if offset fits) |
| 0xB100   | STT    | $X,$Y,Z | Store tetra (4 bytes, compressed if offset fits) |
| 0xB200   | STW    | $X,$Y,Z | Store wyde (2 bytes, compressed if offset fits) |
| 0xB300   | STB    | $X,$Y,Z | Store byte (1 byte, compressed if offset fits) |
| 0xC000   | JMP    | Addr | Jump (compressed if offset fits) |
| 0xC100   | GO     | $X,$Y,Z | Go to (compressed if fits) |
| 0xC200   | BR     | $X,Addr | Branch (compressed if offset fits) |
| 0xC300   | SET    | $X,$Y | Move/set (compressed if fits) |

**Compressed Encoding Rules**:
- Registers must be in range $0-$31 for most compressed instructions
- Immediates limited to smaller ranges (typically 5-8 bits)
- Offsets limited to scaled ranges (e.g., ±32 bytes)
- Assembler chooses encoding automatically based on operand constraints

### 3.2 Vector Instructions

**Note**: Vector instructions follow MMIX mnemonic conventions. Vector register operands are specified as V0-V255, predicate masks as P0-P63.

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xE0   | VADD    | VD,VS1,VS2,PM | Vector add |
| 0xE1   | VSUB    | VD,VS1,VS2,PM | Vector subtract |
| 0xE2   | VMUL    | VD,VS1,VS2,PM | Vector multiply |
| 0xE3   | VDIV    | VD,VS1,VS2,PM | Vector divide |
| 0xE4   | VFADD   | VD,VS1,VS2,PM | Vector FP add |
| 0xE5   | VFSUB   | VD,VS1,VS2,PM | Vector FP subtract |
| 0xE6   | VFMUL   | VD,VS1,VS2,PM | Vector FP multiply |
| 0xE7   | VFDIV   | VD,VS1,VS2,PM | Vector FP divide |
| 0xE8   | VAND    | VD,VS1,VS2,PM | Vector bitwise AND |
| 0xE9   | VOR     | VD,VS1,VS2,PM | Vector bitwise OR |
| 0xEA   | VXOR    | VD,VS1,VS2,PM | Vector bitwise XOR |
| 0xEB   | VSL     | VD,VS1,VS2,PM | Vector shift left |
| 0xEC   | VSR     | VD,VS1,VS2,PM | Vector shift right |
| 0xED   | VLD     | VD,$Y,Z | Vector load strided |
| 0xEE   | VST     | VS,$Y,Z | Vector store strided |
| 0xEF   | VGATHER | VD,$Y,VIDX | Vector gather |

**Vector Instruction Format**:
- VD, VS1, VS2: Vector registers (V0-V255)
- PM: Predicate mask (P0-P63), optional for masked operations
- $Y: Base address register
- Z: Stride/offset
- VIDX: Index vector for gather/scatter

### 3.3 Matrix Instructions

**Note**: Matrix instructions follow MMIX mnemonic conventions. Matrix tile registers are specified as ZA0-ZA7.

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xF0   | MMUL   | ZA,VS1,VS2 | Matrix multiply |
| 0xF1   | MMLA   | ZA,VS1,VS2 | Matrix multiply-accumulate |
| 0xF2   | MFMA   | ZA,VS1,VS2 | Matrix FMA |
| 0xF3   | MTRANS | ZA,ZB | Matrix transpose |
| 0xF4   | MLOAD  | ZA,$Y,Z | Matrix tile load |
| 0xF5   | MSTORE | ZA,$Y,Z | Matrix tile store |
| 0xF6   | MZERO  | ZA | Matrix tile zero |
| 0xF7   | MCOPY  | ZA,ZB | Matrix tile copy |

**Matrix Instruction Format**:
- ZA, ZB: Matrix tile registers (ZA0-ZA7)
- VS1, VS2: Vector source registers for outer product
- $Y: Base address register
- Z: Offset

### 3.4 ML/AI Instructions

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xD0   | CONV2D   | vd,vs1,vs2,cfg | 2D convolution |
| 0xD1   | DEPTHCONV| vd,vs1,vs2,cfg | Depthwise convolution |
| 0xD2   | MAXPOOL  | vd,vs1,cfg | Max pooling |
| 0xD3   | AVGPOOL  | vd,vs1,cfg | Average pooling |
| 0xD4   | RELU     | vd,vs1 | ReLU activation |
| 0xD5   | GELU     | vd,vs1 | GELU activation |
| 0xD6   | SIGMOID  | vd,vs1 | Sigmoid activation |
| 0xD7   | TANH     | vd,vs1 | Tanh activation |
| 0xD8   | SOFTMAX  | vd,vs1 | Softmax activation |
| 0xD9   | BATCHNORM| vd,vs1,vs2,vs3 | Batch normalization |
| 0xDA   | LAYERNORM| vd,vs1,vs2,vs3 | Layer normalization |
| 0xDB   | QUANTIZE | vd,vs1,scale | Quantize to INT8 |
| 0xDC   | DEQUANT  | vd,vs1,scale | Dequantize from INT8 |
| 0xDD   | QDOT     | vd,vs1,vs2 | Quantized dot product |

### 3.5 Hypervisor Instructions

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xF8   | VMENTER  | - | Enter guest mode |
| 0xF9   | VMEXIT   | code | Exit guest mode |
| 0xFA   | VMREAD   | $X,field | Read VMCS field |
| 0xFB   | VMWRITE  | field,$Y | Write VMCS field |
| 0xFC   | VMCLEAR  | addr | Clear VMCS |
| 0xFD   | VMLOAD   | addr | Load VMCS |
| 0xFE   | VMSAVE   | addr | Save VMCS |

## 4. Special Registers

### 4.1 Register Summary

| Reg | Name | Purpose |
|-----|------|---------|
| rA  | Arithmetic status | Overflow, divide-by-zero flags |
| rB  | Bootstrap register | Bootstrap location |
| rC  | Cycle counter | Incremented each cycle |
| rD  | Dividend | High bits of dividend |
| rE  | Epsilon | Floating-point epsilon value |
| rF  | Failure location | Address of failed instruction |
| rG  | Global threshold | Separates local/global registers |
| rH  | High multiply result | High 64 bits of multiply |
| rI  | Interval counter | For periodic interrupts |
| rJ  | Return jump | Return address |
| rK  | Interrupt mask | Interrupt enable bits |
| rL  | Local threshold | Number of local registers |
| rM  | Multiplex mask | For MUX instruction |
| rN  | Serial number | CPU serial number |
| rO  | Register stack offset | Stack offset |
| rP  | Prediction | Branch prediction register |
| rQ  | Interrupt request | Pending interrupts |
| rR  | Remainder | Remainder from division |
| rS  | Register stack pointer | Stack pointer |
| rT  | Trap address | Address of trap handler |
| rU  | Usage counter | Counts register usage |
| rV  | Virtual translation | Virtual address translation |
| rW  | Where interrupted | PC at interrupt |
| rX  | Execution register | Interrupted instruction |
| rY  | Y operand | Saved Y operand |
| rZ  | Z operand | Saved Z operand |
| rBB | Bootstrap backup | Backup of bootstrap |
| rTT | Dynamic trap | Dynamic trap address |
| rWW | Where from interrupt | Alternative saved PC |
| rXX | Execution backup | Alternative saved instruction |
| rYY | Y backup | Alternative saved Y |
| rZZ | Z backup | Alternative saved Z |

### 4.2 Extended Special Registers

| Reg | Name | Purpose |
|-----|------|---------|
| rVL | Vector length | Current vector length in bytes |
| rVT | Vector type | Element type and grouping |
| rMT | Matrix tile config | Tile size and layout |
| rEN | Endianness | Big/little endian mode control |
| rPR | Privilege | Current privilege level |
| rPT | Page table base | Root page table address |
| rAS | Address space ID | Current ASID |
| rGC0-3 | Guest control | Hypervisor guest control |

## 5. Instruction Semantics

### 5.1 Load/Store

**LDO $X,$Y,$Z**: Load octa
```
address = $Y + $Z
$X = Memory[address]
```

**STO $X,$Y,$Z**: Store octa
```
address = $Y + $Z
Memory[address] = $X
```

### 5.2 Arithmetic

**ADD $X,$Y,$Z**: Add
```
$X = $Y + $Z
if overflow: rA |= OVERFLOW_BIT
```

**MUL $X,$Y,$Z**: Multiply
```
result128 = $Y * $Z (128-bit result)
$X = result128[63:0]
rH = result128[127:64]
```

### 5.3 Floating-Point

**FADD $X,$Y,$Z**: Floating-point add
```
$X = float_add($Y, $Z, rounding_mode)
Update rA with FP exception flags
```

**FMA $X,$Y,$Z**: Fused multiply-add
```
$X = ($Y * $Z) + $X (single rounding)
Update rA with FP exception flags
```

### 5.4 Vector Operations

**V.ADD vd,vs1,vs2,vm**: Vector add
```
for i in 0..VL-1:
  if vm[i]:
    vd[i] = vs1[i] + vs2[i]
```

**V.GATHER vd,base,vindex**: Vector gather
```
for i in 0..VL-1:
  vd[i] = Memory[base + vindex[i]]
```

### 5.5 Matrix Operations

**M.MMUL za,vs1,vs2**: Matrix multiply
```
for i in 0..TILE_ROWS-1:
  for j in 0..TILE_COLS-1:
    sum = 0
    for k in 0..TILE_K-1:
      sum += vs1[i,k] * vs2[k,j]
    za[i,j] = sum
```

## 6. Exception and Interrupt Handling

### 6.1 Exception Types

| Code | Exception | Description |
|------|-----------|-------------|
| 0x00 | Reset | Power-on or reset |
| 0x01 | Arithmetic overflow | Integer overflow |
| 0x02 | Divide by zero | Division by zero |
| 0x03 | Invalid operation | Invalid instruction |
| 0x04 | Privileged instruction | Illegal privilege |
| 0x05 | Page fault | Virtual memory fault |
| 0x06 | Protection fault | Memory protection violation |
| 0x07 | Alignment fault | Misaligned memory access |
| 0x08 | FP exception | Floating-point exception |
| 0x09 | Breakpoint | Debug breakpoint |
| 0x0A | Single step | Debug single step |
| 0x0B | VM exit | Virtual machine exit |

### 6.2 Interrupt Types

| Code | Interrupt | Description |
|------|-----------|-------------|
| 0x10 | Timer | Interval timer |
| 0x11 | External | External device |
| 0x12 | IPI | Inter-processor |
| 0x13 | Software | Software interrupt |

### 6.3 Exception Handling

When an exception occurs:
1. Save PC in rW
2. Save instruction in rX
3. Save operands in rY, rZ
4. Set privilege to supervisor
5. Jump to rT (trap handler)

## 7. Privilege Levels

| Level | Name | Access |
|-------|------|--------|
| 0 | User | User applications |
| 1 | Supervisor | Operating system |
| 2 | Hypervisor | Virtualization layer |

Privilege transition:
- Up: Via exception/interrupt
- Down: Via RESUME/VMENTER instruction

## 8. Endianness

### 8.1 Control

**rEN register bits**:
- Bit 0: User mode endianness (0=big, 1=little)
- Bit 1: Supervisor mode endianness
- Bit 2: Hypervisor mode endianness

### 8.2 Byte Order

**Big-endian** (traditional MMIX):
```
Address: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
Value:   [MSB]                             [LSB]
```

**Little-endian**:
```
Address: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07
Value:   [LSB]                             [MSB]
```

## 9. Floating-Point Formats

### 9.1 Format Summary

| Format | Bits | Exponent | Mantissa | Bias |
|--------|------|----------|----------|------|
| FP8 E5M2 | 8 | 5 | 2 | 15 |
| FP8 E4M3 | 8 | 4 | 3 | 7 |
| BFloat16 | 16 | 8 | 7 | 127 |
| Binary16 | 16 | 5 | 10 | 15 |
| Binary32 | 32 | 8 | 23 | 127 |
| Binary64 | 64 | 11 | 52 | 1023 |
| Binary128 | 128 | 15 | 112 | 16383 |
| Decimal32 | 32 | - | - | - |
| Decimal64 | 64 | - | - | - |
| Decimal128 | 128 | - | - | - |

### 9.2 Rounding Modes

| Mode | Code | Description |
|------|------|-------------|
| RNE | 0 | Round to nearest, ties to even |
| RTZ | 1 | Round toward zero |
| RDN | 2 | Round down (toward -∞) |
| RUP | 3 | Round up (toward +∞) |
| RMM | 4 | Round to nearest, ties away from zero |

## 10. Application Binary Interface (ABI)

### 10.1 Register Usage Convention

#### 10.1.1 General-Purpose Registers (64-bit ABI)

| Register | Name | Usage | Preserved |
|----------|------|-------|-----------|
| $0 | zero | Constant zero | N/A |
| $1-$8 | a0-a7 | Function arguments / return values | Caller |
| $9-$15 | t0-t6 | Temporary registers | Caller |
| $16-$23 | s0-s7 | Saved registers | Callee |
| $24 | gp | Global pointer (optional) | Callee |
| $25 | tp | Thread pointer | Callee |
| $252 | fp | Frame pointer (optional) | Callee |
| $253 | ra | Return address | Caller |
| $254 | sp | Stack pointer | Callee |
| $255 | — | Reserved for OS/kernel | Special |

#### 10.1.2 Floating-Point Registers

| Register | Usage | Preserved |
|----------|-------|-----------|
| F0-F7 | FP arguments / return values | Caller |
| F8-F15 | FP temporaries | Caller |
| F16-F23 | FP saved registers | Callee |
| F24-F255 | FP temporaries | Caller |

**FPR Aliasing Mode**: When `FprAliasedToGpr = TRUE`, F0-F255 are aliased to $0-$255 and follow GPR conventions.

#### 10.1.3 Vector Registers

| Register | Usage | Preserved |
|----------|-------|-----------|
| V0-V7 | Vector arguments / return values | Caller |
| V8-V31 | Vector temporaries | Caller |
| V32-V255 | Vector temporaries | Caller |

#### 10.1.4 Special Registers

| Register | Name | Usage |
|----------|------|-------|
| rA | Arithmetic status | FP exception flags |
| rB | Bootstrap | Boot address |
| rG | Global threshold | Register window boundary |
| rL | Local threshold | Register window boundary |
| rJ | Return jump | Function return address |
| rS | Stack pointer | Hardware stack management |
| rO | Stack offset | Register window offset |
| rT | Trap handler | Exception handler address |
| rW | Where | Exception PC |
| rX | Execution | Faulting instruction |
| rY, rZ | Operands | Exception operands |

### 10.2 Function Calling Convention

#### 10.2.1 Parameter Passing

**Integer / Pointer Arguments:**
- First 8 arguments: $1-$8 (a0-a7)
- Additional arguments: Stack (16-byte aligned)

**Floating-Point Arguments:**
- First 8 FP arguments: F1-F8
- Additional FP arguments: Stack

**Vector Arguments:**
- First 8 vector arguments: V1-V8
- Additional vector arguments: Stack (must be 16-byte aligned)

**Large Structures** (> 8 bytes):
- Passed by reference (pointer in argument register)
- Caller allocates space
- Caller responsible for copying if needed

**Variadic Functions:**
- Named arguments follow normal rules
- Variadic arguments always on stack
- Floating-point values passed in integer registers when mixed with integers

#### 10.2.2 Return Values

**Integer / Pointer Returns:**
- Single value: $1 (a0)
- Pair (128-bit): $1-$2 (a0-a1)
- Quad (256-bit): $1-$4 (a0-a3)

**Floating-Point Returns:**
- Single FP value: F1
- Multiple FP values: F1-F4
- Complex: Real in F1, Imaginary in F2

**Vector Returns:**
- Single vector: V1
- Multiple vectors: V1-V4

**Structures** (by value):
- ≤ 8 bytes: $1
- ≤ 16 bytes: $1-$2
- ≤ 32 bytes: $1-$4
- > 32 bytes: Returned via hidden pointer (passed in $1 by caller)

#### 10.2.3 Stack Frame Layout (Detailed)

```
High addresses
┌─────────────────────────┐
│ Caller's frame          │
├─────────────────────────┤
│ Arg 9                   │ +72 from entry SP
├─────────────────────────┤
│ Arg 10                  │ +64 from entry SP
├─────────────────────────┤
│ ...                     │
├─────────────────────────┤
│ (Alignment padding)     │
├═════════════════════════┤ ← Entry SP ($254 on entry)
│ Return address ($253)   │ -8
├─────────────────────────┤
│ Old FP ($252)           │ -16 (if using frame pointer)
├─────────────────────────┤
│ Saved register $23      │ -24
│ Saved register $22      │ -32
│ ...                     │
│ Saved register $16      │ -80
├─────────────────────────┤
│ Saved FP register F23   │ -88 (16 bytes, aligned)
│ ...                     │
│ Saved FP register F16   │ -216
├─────────────────────────┤
│ Local variable 1        │ -224
│ Local variable 2        │ -232
│ ...                     │
├─────────────────────────┤
│ (Alignment padding)     │
├─────────────────────────┤
│ Outgoing arg space (≥8) │
│ (for next function call)│
└─────────────────────────┘ ← Current SP ($254 in function)
Low addresses
```

**Stack Alignment:**
- Stack pointer must be 16-byte aligned at all times
- 32-byte alignment required for vector operations
- 64-byte alignment required for matrix operations

#### 10.2.4 Function Prologue Example

```asm
function_name:
    ; Save return address and frame pointer
    SUBU $254,$254,96       ; Allocate stack frame (96 bytes)
    STO  $253,$254,88       ; Save return address at offset 88
    STO  $252,$254,80       ; Save old frame pointer at offset 80
    ADDU $252,$254,96       ; Set new frame pointer

    ; Save callee-saved registers
    STO  $16,$254,0         ; Save $16
    STO  $17,$254,8         ; Save $17
    STO  $18,$254,16        ; Save $18
    ; ... save other needed registers

    ; Save callee-saved FP registers (if used)
    STOU F16,$254,32        ; Save F16 (16-byte aligned)
    STOU F17,$254,48        ; Save F17
    ; ... function body follows
```

#### 10.2.5 Function Epilogue Example

```asm
    ; Restore callee-saved FP registers
    LDOU F17,$254,48        ; Restore F17
    LDOU F16,$254,32        ; Restore F16

    ; Restore callee-saved registers
    LDO  $18,$254,16        ; Restore $18
    LDO  $17,$254,8         ; Restore $17
    LDO  $16,$254,0         ; Restore $16

    ; Restore frame pointer and return address
    LDO  $252,$254,80       ; Restore frame pointer
    LDO  $253,$254,88       ; Restore return address
    ADDU $254,$254,96       ; Deallocate stack frame

    ; Return to caller
    JMP  $253,0             ; Jump to return address
```

### 10.3 Position-Independent Code (PIC)

#### 10.3.1 Global Offset Table (GOT)

```asm
; Load address of global variable via GOT
_GLOBAL_OFFSET_TABLE_:
    LDO  $24,got_base       ; Load GOT base into $24 (gp)
    LDO  $1,$24,var_offset  ; Load variable address from GOT
    LDO  $2,$1,0            ; Load variable value
```

#### 10.3.2 Procedure Linkage Table (PLT)

```asm
; Call external function via PLT
external_func@PLT:
    LDO  $1,$24,func_offset ; Load function address from GOT
    JMP  $1,0               ; Jump to function
```

### 10.4 Thread-Local Storage (TLS)

#### 10.4.1 TLS Access Models

**Local Exec (LE) Model:**
```asm
; Access thread-local variable (executable)
    LDO  $1,$25,var_offset  ; $25 = thread pointer
    LDO  $2,$1,0            ; Load TLS variable
```

**Initial Exec (IE) Model:**
```asm
; Access thread-local variable (shared library)
    LDO  $1,$24,tls_offset  ; Load TLS offset from GOT
    ADDU $1,$25,$1          ; Add to thread pointer
    LDO  $2,$1,0            ; Load TLS variable
```

**General Dynamic (GD) Model:**
```asm
; Access thread-local variable (dynamic)
    PUSHJ $253,__tls_get_addr  ; Get TLS address
    LDO  $2,$1,0            ; Load TLS variable
```

### 10.5 Data Type Sizes and Alignment

| Type | Size (bytes) | Alignment | Notes |
|------|--------------|-----------|-------|
| `char` | 1 | 1 | Signed or unsigned |
| `short` | 2 | 2 | 16-bit |
| `int` | 4 | 4 | 32-bit |
| `long` | 8 | 8 | 64-bit |
| `long long` | 8 | 8 | 64-bit |
| `pointer` | 8 | 8 | 64-bit address |
| `float` | 4 | 4 | IEEE 754 single |
| `double` | 8 | 8 | IEEE 754 double |
| `long double` | 16 | 16 | IEEE 754 quad |
| `__float128` | 16 | 16 | IEEE 754 quad |
| `vector` | 16-256 | 16 | Scalable vectors |
| `matrix tile` | Variable | 64 | Matrix operations |

**Structure Alignment:**
- Natural alignment for all members
- Padding inserted as needed
- Struct alignment = max member alignment
- Struct size rounded up to multiple of alignment

**Array Alignment:**
- Same as element type
- No padding between elements

**Union Alignment:**
- Alignment of largest member
- Size = size of largest member (rounded)

### 10.6 Varargs / Variable Arguments

**stdarg.h Implementation:**
```c
typedef struct {
    void *stack_ptr;        // Current stack position
    int arg_count;          // Number of arguments processed
    int fp_offset;          // FP register offset
    int vec_offset;         // Vector register offset
} va_list;

// Start of variadic arguments
void va_start(va_list *ap, last_named_arg);

// Get next argument
type va_arg(va_list *ap, type);

// End of variadic arguments
void va_end(va_list *ap);
```

**Argument Extraction:**
- Check if argument is in register (first 8)
- If in register: Copy from saved register area
- If on stack: Load from stack, advance pointer
- FP arguments use separate FP register tracking
- Vector arguments require 16-byte alignment

### 10.7 System V MMIX ABI Summary

**Key Differences from Other Architectures:**
- 256 general-purpose registers (vs 32 in RISC-V, ARM)
- Optional FPR aliasing to GPRs (compatibility mode)
- Per-ring endianness and stack direction (KESU extension)
- Scalable vector length (128-2048 bits)
- Hardware register windows (rG, rL, rO registers)

**Compatibility Notes:**
- Binary compatible with MMIX-ELF format
- Source compatible with ANSI C/C++/C23
- POSIX compliant
- GCC-compatible attributes supported

## 11. Performance Hints

### 11.1 Instruction Scheduling

- **Latency**: MUL (3 cycles), DIV (20 cycles), FADD (4 cycles), FDIV (25 cycles)
- **Throughput**: Most instructions 1/cycle, FDIV 1/25 cycles
- **Pipeline**: 5 stages (Fetch, Decode, Execute, Memory, Writeback)

### 11.2 Memory Hints

- **Prefetch**: LDVTS can prefetch data
- **Alignment**: Aligned accesses are faster
- **Caching**: LDUNC/STUNC bypass cache

### 11.3 Branch Prediction

- **Backward branches**: Predicted taken
- **Forward branches**: Predicted not taken
- **Indirect branches**: Use branch target buffer

## 12. Assembler Syntax

### 12.1 Register Notation

- `$N` or `$rN`: General register N (0-255)
- `rX`: Special register X
- `vN`: Vector register N (0-31)
- `pN`: Predicate register N (0-15)
- `zaN`: Matrix tile N (0-7)

### 12.2 Immediate Values

- Decimal: `123`
- Hexadecimal: `0x7B`
- Binary: `0b1111011`
- Character: `'A'`

### 12.3 Labels and Symbols

```asm
start:          ; Label
    ADD $1,$2,$3   ; Instruction
    BZ $1,end      ; Branch to label
end:
    JMP continue   ; Jump
```

### 12.4 Directives

```asm
.text           ; Code section
.data           ; Data section
.align 16       ; Align to 16 bytes
.octa value     ; Define 8-byte value
.string "text"  ; Define string
```
