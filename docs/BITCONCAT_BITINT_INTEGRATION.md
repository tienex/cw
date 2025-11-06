# Bit Concatenation and _BitInt Integration Plan

## Overview

This document describes how the bit concatenation operator (`..`) should integrate with C23's `_BitInt(N)` arbitrary-width integer type when it's fully implemented.

## Current Implementation (as of 2025-11-06)

The bit concatenation operator uses **explicit bit-width suffixes** on integer literals:

### Bit-Width Suffix Syntax
Integer literals can specify their bit-width using suffixes:
- `ui<N>` - unsigned integer with N bits (e.g., `0xAui4`, `255ui8`)
- `i<N>` - signed integer with N bits (e.g., `127i8`, `15i4`)
- No suffix - defaults to 8-bit shift for backward compatibility

Examples:
```c
int nibbles = 0xAui4 .. 0xBui4;     // Shifts by 4 bits → 0xAB
int bytes = 0xABui8 .. 0xCDui8;     // Shifts by 8 bits → 0xABCD
int mixed = 0x1ui8 .. 0xFFui8;      // Shifts by 8 bits → 0x1FF
int legacy = 0xA .. 0xB;            // Defaults to 8 bits → 0xA00 | 0xB
```

### Implementation Details
- **Lexer** (`src/compiler/Lexer.c`): Reads alphanumeric suffixes including digits
- **Parser** (`src/compiler/Parser.c`): Parses suffix to extract unsigned flag and bit-width
- **AST** (`include/compiler/MmixAst.h`): Stores `BitWidth` field in integer literal nodes
- **IR** (`include/compiler/MmixIr.h`): Propagates `BitWidth` through IR_OPERAND
- **CodeGen** (`src/compiler/CodeGen.c`): Uses `BitWidth` for bit concatenation shift amounts

### For Variable Operands
- Defaults to 8-bit shift for backward compatibility
- **Future**: Will use _BitInt type information when available

## _BitInt(N) Type Support (Future Enhancement)

### What is _BitInt?

C23 introduces `_BitInt(N)` for arbitrary-width integers:
```c
_BitInt(17) x;     // 17-bit signed integer
_BitInt(128) big;  // 128-bit signed integer
```

### Current _BitInt Status in Compiler

- **Token**: `TOK_BITINT` defined in `include/compiler/MmixToken.h:106`
- **AST Type**: `AST_TYPE_BITINT` in `include/compiler/MmixAst.h:121`
- **Width Field**: `AST_TYPE.BitInt.Width` stores the bit width
- **Implementation**: Not yet complete (see `src/compiler/Parser.c:1519`)

### Integration Plan

When `_BitInt` is implemented, enhance bit concatenation in `src/compiler/CodeGen.c` case `IR_BFCONCAT`:

```c
case IR_BFCONCAT:
  {
    UINT32 ShiftAmount = 8;  // Default

    if (Instr->Src2.Type == IR_OPERAND_CONST) {
      // Current dynamic detection - works great!
      ShiftAmount = ComputeBitWidth(Instr->Src2.ConstValue);
    }
    else if (Instr->Src2.DataType != NULL) {
      AST_TYPE *Type = Instr->Src2.DataType;

      // NEW: Check for _BitInt type
      if (Type->Kind == AST_TYPE_BITINT) {
        ShiftAmount = Type->BitInt.Width;
      }
      // NEW: Use sizeof for other types
      else if (Type->Kind == AST_TYPE_CHAR || Type->Kind == AST_TYPE_INT8) {
        ShiftAmount = 8;
      }
      else if (Type->Kind == AST_TYPE_SHORT || Type->Kind == AST_TYPE_INT16) {
        ShiftAmount = 16;
      }
      else if (Type->Kind == AST_TYPE_INT || Type->Kind == AST_TYPE_INT32) {
        ShiftAmount = 32;
      }
      else if (Type->Kind == AST_TYPE_LONG || Type->Kind == AST_TYPE_INT64) {
        ShiftAmount = 64;
      }
      else {
        ShiftAmount = 8;  // Fallback
      }

      // Overflow check for known widths
      if (Instr->Src1.DataType != NULL) {
        UINT32 LeftBits = GetTypeWidth(Instr->Src1.DataType);
        if (LeftBits + ShiftAmount > 64) {
          fprintf(stderr, "Warning: bit concatenation overflow (%u + %u bits)\n",
                  LeftBits, ShiftAmount);
        }
      }
    }

    // ... rest of code generation
  }
```

### Example Use Cases

```c
// Example 1: Explicit bit widths
_BitInt(4) nibble1 = 0xA;
_BitInt(4) nibble2 = 0xB;
int result = nibble1 .. nibble2;  // Shifts by 4 bits → 0xAB

// Example 2: Mixed widths
_BitInt(12) addr_hi = 0x123;
_BitInt(8) addr_lo = 0x45;
int address = addr_hi .. addr_lo;  // Shifts by 8 bits → 0x12345

// Example 3: Large integers
_BitInt(48) timestamp_hi = get_timestamp_hi();
_BitInt(16) timestamp_lo = get_timestamp_lo();
_BitInt(64) timestamp = timestamp_hi .. timestamp_lo;  // Shifts by 16 bits

// Example 4: Overflow detection
_BitInt(40) big1 = 0xFFFFFFFFFF;
_BitInt(32) big2 = 0xFFFFFFFF;
// WARNING: 40 + 32 = 72 bits exceeds 64-bit register
int result = big1 .. big2;
```

### Additional Enhancements

1. **Type Safety**
   - Verify result type can hold the concatenated value
   - Suggest `_BitInt(N)` for result if overflow detected

2. **Optimization Opportunities**
   - Constant fold `_BitInt` concatenations at compile time
   - Use MMIX's byte/wyde/tetra/octa instructions when appropriate

3. **Error Messages**
   ```c
   _BitInt(32) a = 0xFFFFFFFF;
   _BitInt(32) b = 0xFFFFFFFF;
   int result = a .. b;  // ERROR: result needs 64 bits, but 'int' is 32 bits
   // Suggestion: Use '_BitInt(64)' for result type
   ```

4. **Mixed-Width Expressions**
   ```c
   _BitInt(4) n1 = 0xA;
   int byte = 0x12;
   _BitInt(8) n2 = 0xB;

   // Complex: (4-bit .. 32-bit) .. 8-bit
   auto result = (n1 .. byte) .. n2;
   // Should infer: _BitInt(44) result
   ```

## Implementation Checklist

When implementing `_BitInt` integration:

- [ ] Complete `_BitInt(N)` type implementation in Parser
- [ ] Add `GetTypeWidth()` helper function in CodeGen
- [ ] Update `IR_BFCONCAT` to check for `AST_TYPE_BITINT`
- [ ] Add type-based overflow warnings for non-constant operands
- [ ] Implement result type inference for bit concatenation
- [ ] Add comprehensive tests in `tests/compiler/bitfield/bitint_concat.c`
- [ ] Update documentation in `docs/MMIX_COMPLETE_REFERENCE.md`

## References

- **C23 Standard**: ISO/IEC 9899:2023 Section 6.2.5 (Types) - `_BitInt(N)`
- **Bit Concatenation**: `docs/MMIX_COMPLETE_REFERENCE.md` - Bit-field operations
- **Current Implementation**: `src/compiler/CodeGen.c:578-638`
- **AST Types**: `include/compiler/MmixAst.h:121` (AST_TYPE_BITINT)
