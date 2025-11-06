# MMIX Compiler and Toolchain Test Suite

This directory contains the test suite for the MMIX C23 compiler and toolchain.

## Directory Structure

```
tests/
├── compiler/           # C compiler tests
│   ├── basic/         # Basic language features
│   ├── bitfield/      # Bit field operations (MMIX extensions)
│   ├── loops/         # Loop constructs
│   └── functions/     # Function calls and recursion
├── assembler/         # MMIX assembler tests
├── linker/            # MMIX linker tests
├── objdump/           # MMIX objdump tests
├── integrated/        # Full toolchain integration tests
├── run-tests.sh       # Main test runner
└── README.md          # This file
```

## Running Tests

### All Tests
```bash
cd tests
./run-tests.sh
```

### Specific Category
```bash
# Compiler tests only
./run-tests.sh compiler/basic

# Bit field tests only
./run-tests.sh compiler/bitfield
```

### Individual Test
```bash
# Compile test
../bin/mmix-cc compiler/basic/arithmetic.c -o /tmp/test.s

# Verify output
../bin/mmix-filecheck compiler/basic/arithmetic.c -input /tmp/test.s
```

## Test Format

Tests use LLVM-style RUN and CHECK directives:

```c
// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// CHECK-LABEL: function_name
int function_name(int x) {
    // CHECK: ADDU
    return x + 1;
}
```

### CHECK Directives

- `// CHECK: pattern` - Match pattern anywhere after previous match
- `// CHECK-LABEL: name` - Match label and reset search position
- `// CHECK: {{.*}}` - Regex pattern using {{...}} syntax

## Test Coverage

### Compiler Tests (11 tests)

**Basic Operations (3 tests)**
- arithmetic.c: Addition, subtraction, multiplication, division, modulo
- bitwise.c: AND, OR, XOR, NOT, shifts
- variables.c: Declarations, assignments, compound assignments, scoping

**Bit Field Operations (4 tests)**
- extract.c: Bit field extraction `x[index:count]`
- concat.c: Bit concatenation `a .. b`
- test_bitfield.c: Nibble and byte extraction
- test_bitconcat.c: Byte and word concatenation

**Loop Constructs (3 tests)**
- while.c: While loops with break/continue
- for.c: For loops (classic, infinite, nested)
- dowhile.c: Do-while loops

**Function Calls (1 test)**
- call.c: Simple, recursive, and nested function calls

### Assembler Tests (3 tests)

- basic.mms: Basic MMIX instructions
- labels.mms: Labels and jumps
- directives.mms: Assembler directives (.text, .data, .globl)

### Integrated Tests (1 test)

- compile-and-assemble.sh: Full C-to-binary pipeline

## Tools

### mmix-cc
MMIX C23 compiler with MMIX-specific extensions:
- Bit field indexing: `intvar[index:count]`
- Bit concatenation: `a .. b`

### mmix-filecheck
Pattern matching verification tool (similar to LLVM FileCheck):
- Supports CHECK and CHECK-LABEL directives
- Regex patterns with {{...}} syntax
- Verbose mode: `-v`
- Input from file: `-input file.s`

### mmix-as
MMIX assembler

### mmix-ld
MMIX linker

### mmix-objdump
MMIX disassembler and object file analyzer

## Test Results

Current status: **All 11 tests passing**

```
Test Summary
========================
Total tests:  11
Passed:       11
Failed:       0
```

## Adding New Tests

1. Create test file in appropriate directory
2. Add RUN and CHECK directives
3. Run test suite to verify
4. Commit with descriptive message

Example:
```bash
# Create new test
cat > tests/compiler/basic/newtest.c << 'EOF'
// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// CHECK-LABEL: test_function
int test_function() {
    // CHECK: SET
    return 42;
}
EOF

# Run test
cd tests && ./run-tests.sh
```

## Contributing

When adding new compiler features:
1. Add tests for the feature
2. Verify all existing tests still pass
3. Document CHECK patterns for new instructions
4. Update this README if adding new test categories
