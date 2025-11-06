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

## 2. Standard MMIX Opcode List

### 2.1 Load Instructions (0x00 - 0x0F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x00   | LDB      | $X,$Y,$Z | Load byte unsigned |
| 0x01   | LDBU     | $X,$Y,$Z | Load byte unsigned (alias) |
| 0x02   | LDW      | $X,$Y,$Z | Load wyde (2 bytes) unsigned |
| 0x03   | LDWU     | $X,$Y,$Z | Load wyde unsigned (alias) |
| 0x04   | LDT      | $X,$Y,$Z | Load tetra (4 bytes) unsigned |
| 0x05   | LDTU     | $X,$Y,$Z | Load tetra unsigned (alias) |
| 0x06   | LDO      | $X,$Y,$Z | Load octa (8 bytes) |
| 0x07   | LDOU     | $X,$Y,$Z | Load octa unsigned (alias) |
| 0x08   | LDBS     | $X,$Y,$Z | Load byte signed |
| 0x09   | LDWS     | $X,$Y,$Z | Load wyde signed |
| 0x0A   | LDTS     | $X,$Y,$Z | Load tetra signed |
| 0x0B   | LDOS     | $X,$Y,$Z | Load octa signed (alias) |
| 0x0C   | LDHT     | $X,$Y,$Z | Load high tetra |
| 0x0D   | LDSF     | $X,$Y,$Z | Load short float |
| 0x0E   | LDVTS    | $X,$Y,$Z | Load virtual translation |
| 0x0F   | LDUNC    | $X,$Y,$Z | Load uncached |

### 2.2 Store Instructions (0x10 - 0x1F)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0x10   | STB      | $X,$Y,$Z | Store byte |
| 0x11   | STBU     | $X,$Y,$Z | Store byte uncached |
| 0x12   | STW      | $X,$Y,$Z | Store wyde |
| 0x13   | STWU     | $X,$Y,$Z | Store wyde uncached |
| 0x14   | STT      | $X,$Y,$Z | Store tetra |
| 0x15   | STTU     | $X,$Y,$Z | Store tetra uncached |
| 0x16   | STO      | $X,$Y,$Z | Store octa |
| 0x17   | STOU     | $X,$Y,$Z | Store octa uncached |
| 0x18   | STBS     | $X,$Y,$Z | Store byte signed |
| 0x19   | STWS     | $X,$Y,$Z | Store wyde signed |
| 0x1A   | STTS     | $X,$Y,$Z | Store tetra signed |
| 0x1B   | STOS     | $X,$Y,$Z | Store octa signed |
| 0x1C   | STHT     | $X,$Y,$Z | Store high tetra |
| 0x1D   | STCO     | $X,$Y,$Z | Store conditional |
| 0x1E   | STUNC    | $X,$Y,$Z | Store uncached |
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

### 3.1 Compressed Instructions (c.* prefix)

| Encoding | Mnemonic | Format | Description |
|----------|----------|--------|-------------|
| 0x8000   | C.ADD    | c.rd,c.rs1,c.rs2 | Compressed add |
| 0x8100   | C.SUB    | c.rd,c.rs1,c.rs2 | Compressed subtract |
| 0x8200   | C.MUL    | c.rd,c.rs1,c.rs2 | Compressed multiply |
| 0x8300   | C.AND    | c.rd,c.rs1,c.rs2 | Compressed AND |
| 0x8400   | C.OR     | c.rd,c.rs1,c.rs2 | Compressed OR |
| 0x8500   | C.XOR    | c.rd,c.rs1,c.rs2 | Compressed XOR |
| 0x9000   | C.ADDI   | c.rd,c.rs1,imm | Compressed add immediate |
| 0x9100   | C.SUBI   | c.rd,c.rs1,imm | Compressed subtract immediate |
| 0x9200   | C.ANDI   | c.rd,c.rs1,imm | Compressed AND immediate |
| 0x9300   | C.ORI    | c.rd,c.rs1,imm | Compressed OR immediate |
| 0x9400   | C.XORI   | c.rd,c.rs1,imm | Compressed XOR immediate |
| 0x9500   | C.LI     | c.rd,imm | Compressed load immediate |
| 0xA000   | C.LDO    | c.rd,offset(c.rs1) | Compressed load octa |
| 0xA100   | C.LDT    | c.rd,offset(c.rs1) | Compressed load tetra |
| 0xA200   | C.LDW    | c.rd,offset(c.rs1) | Compressed load wyde |
| 0xA300   | C.LDB    | c.rd,offset(c.rs1) | Compressed load byte |
| 0xB000   | C.STO    | c.rs2,offset(c.rs1) | Compressed store octa |
| 0xB100   | C.STT    | c.rs2,offset(c.rs1) | Compressed store tetra |
| 0xB200   | C.STW    | c.rs2,offset(c.rs1) | Compressed store wyde |
| 0xB300   | C.STB    | c.rs2,offset(c.rs1) | Compressed store byte |
| 0xC000   | C.J      | offset | Compressed jump |
| 0xC100   | C.JR     | c.rs1 | Compressed jump register |
| 0xC200   | C.BR     | c.rs1,offset | Compressed branch |
| 0xC300   | C.MV     | c.rd,c.rs1 | Compressed move |

### 3.2 Vector Instructions (V.* prefix)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xE0   | V.ADD    | vd,vs1,vs2,vm | Vector add |
| 0xE1   | V.SUB    | vd,vs1,vs2,vm | Vector subtract |
| 0xE2   | V.MUL    | vd,vs1,vs2,vm | Vector multiply |
| 0xE3   | V.DIV    | vd,vs1,vs2,vm | Vector divide |
| 0xE4   | V.FADD   | vd,vs1,vs2,vm | Vector FP add |
| 0xE5   | V.FSUB   | vd,vs1,vs2,vm | Vector FP subtract |
| 0xE6   | V.FMUL   | vd,vs1,vs2,vm | Vector FP multiply |
| 0xE7   | V.FDIV   | vd,vs1,vs2,vm | Vector FP divide |
| 0xE8   | V.AND    | vd,vs1,vs2,vm | Vector bitwise AND |
| 0xE9   | V.OR     | vd,vs1,vs2,vm | Vector bitwise OR |
| 0xEA   | V.XOR    | vd,vs1,vs2,vm | Vector bitwise XOR |
| 0xEB   | V.SL     | vd,vs1,vs2,vm | Vector shift left |
| 0xEC   | V.SR     | vd,vs1,vs2,vm | Vector shift right |
| 0xED   | V.LD     | vd,base,stride | Vector load strided |
| 0xEE   | V.ST     | vs,base,stride | Vector store strided |
| 0xEF   | V.GATHER | vd,base,vindex | Vector gather |

### 3.3 Matrix Instructions (M.* prefix)

| Opcode | Mnemonic | Format | Description |
|--------|----------|--------|-------------|
| 0xF0   | M.MMUL   | za,vs1,vs2 | Matrix multiply |
| 0xF1   | M.MMLA   | za,vs1,vs2 | Matrix multiply-accumulate |
| 0xF2   | M.MFMA   | za,vs1,vs2 | Matrix FMA |
| 0xF3   | M.MTRANS | za,zb | Matrix transpose |
| 0xF4   | M.MLOAD  | za,base | Matrix tile load |
| 0xF5   | M.MSTORE | za,base | Matrix tile store |
| 0xF6   | M.MZERO  | za | Matrix tile zero |
| 0xF7   | M.MCOPY  | za,zb | Matrix tile copy |

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

## 10. ABI Details

### 10.1 Stack Frame Layout (64-bit)

```
High addresses
┌─────────────────────┐
│ Previous frame      │
├─────────────────────┤
│ Return address      │ ← $254 (on entry)
├─────────────────────┤
│ Saved registers     │
├─────────────────────┤
│ Local variables     │
├─────────────────────┤
│ Outgoing args (>8)  │
└─────────────────────┘ ← $254 (in function)
Low addresses
```

### 10.2 Register Allocation

**Caller-saved** (may be clobbered):
- $1-$8 (arguments/return)
- $9-$15 (temporaries)

**Callee-saved** (must be preserved):
- $16-$23
- $254 (stack pointer)

### 10.3 Data Type Alignment

| Type | Size | Alignment |
|------|------|-----------|
| byte | 1 | 1 |
| wyde | 2 | 2 |
| tetra | 4 | 4 |
| octa | 8 | 8 |
| vector | varies | 16 |
| matrix | varies | 64 |

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
