# MMIX C23 Compiler Design

## Overview

This document describes the design of the MMIX C23 compiler (`mmix-cc`) with comprehensive support for GNU, MSVC, and Clang language extensions.

## Architecture

```
Source Code (.c)
    ↓
[Preprocessor] → Macros, includes, conditional compilation
    ↓
[Lexer] → Tokens
    ↓
[Parser] → Abstract Syntax Tree (AST)
    ↓
[Semantic Analyzer] → Type checking, symbol resolution
    ↓
[IR Generator] → Intermediate Representation
    ↓
[Optimizer] → SSA form, constant folding, dead code elimination
    ↓
[Code Generator] → MMIX assembly
    ↓
[Assembler] → Object file with debug info
```

## C23 Standard Features

### Core Language
- `_Generic` selections
- `_Static_assert`
- `_Alignas` and `_Alignof`
- `_Noreturn`
- `_Thread_local`
- `typeof` and `typeof_unqual`
- `constexpr`
- `nullptr`
- `_BitInt(N)` arbitrary-width integers
- `[[attributes]]` standard attributes
- Binary literals (`0b` prefix)
- Digit separators (`1'000'000`)
- `auto` type inference (C23)
- Labels at end of compound statements
- Empty initializer lists

### Preprocessor
- `__VA_OPT__` for variadic macros
- `#elifdef` and `#elifndef`
- `#embed` for binary resources
- `#warning` directive
- `__has_include` and `__has_c_attribute`

### Library Features
- `<stdckdint.h>` - Checked integer arithmetic
- `<stdbit.h>` - Bit manipulation
- `<stdatomic.h>` enhancements
- `<threads.h>` improvements

## GNU Extensions (GCC Compatibility)

### Statement Expressions
```c
int x = ({ int y = foo(); y + 1; });
```

### typeof Operator
```c
typeof(x) y = x;
__typeof__(expr) var;
```

### Case Ranges
```c
case 'A' ... 'Z':
```

### Labels as Values
```c
void *ptr = &&label;
goto *ptr;
```

### Nested Functions
```c
void outer() {
    void inner() { }
    inner();
}
```

### Flexible Array Members
```c
struct flex {
    int n;
    int data[];
};
```

### Zero-Length Arrays
```c
int array[0];
```

### Variable-Length Arrays in Structs
```c
struct vla {
    int n;
    int data[n];
};
```

### Attributes
- `__attribute__((aligned(N)))`
- `__attribute__((packed))`
- `__attribute__((section("name")))`
- `__attribute__((weak))`
- `__attribute__((alias("target")))`
- `__attribute__((constructor))` / `__attribute__((destructor))`
- `__attribute__((format(printf, 1, 2)))`
- `__attribute__((noreturn))`
- `__attribute__((always_inline))`
- `__attribute__((noinline))`
- `__attribute__((pure))` / `__attribute__((const))`
- `__attribute__((hot))` / `__attribute__((cold))`
- `__attribute__((visibility("default|hidden|internal|protected")))`
- `__attribute__((deprecated))`
- `__attribute__((warn_unused_result))`
- `__attribute__((malloc))`
- `__attribute__((nonnull))`
- `__attribute__((returns_nonnull))`

### Built-in Functions
- `__builtin_expect(expr, expected)`
- `__builtin_unreachable()`
- `__builtin_prefetch(addr, rw, locality)`
- `__builtin_constant_p(expr)`
- `__builtin_types_compatible_p(type1, type2)`
- `__builtin_choose_expr(const, expr1, expr2)`
- `__builtin_offsetof(type, member)`
- `__builtin_va_arg_pack()`
- `__builtin_clz(x)` - Count leading zeros
- `__builtin_ctz(x)` - Count trailing zeros
- `__builtin_popcount(x)` - Population count
- `__builtin_bswap32/64(x)` - Byte swap
- `__builtin_frame_address(level)`
- `__builtin_return_address(level)`
- `__builtin_alloca(size)`
- `__builtin_trap()`
- `__builtin_abort()`

### Inline Assembly
```c
asm volatile ("instruction" : outputs : inputs : clobbers);
__asm__ __volatile__ ("code");
```

### Compound Literals
```c
(struct point){.x = 1, .y = 2}
```

### Designated Initializers
```c
struct s var = {.field = value};
int arr[10] = {[5] = 42};
```

### Binary Constants
```c
0b101010
0B11110000
```

### 128-bit Integers
```c
__int128
unsigned __int128
```

## MSVC Extensions

### Calling Conventions
- `__cdecl` - C calling convention
- `__stdcall` - Standard call
- `__fastcall` - Fast call
- `__vectorcall` - Vector calling convention
- `__thiscall` - C++ this call

### Storage Class Modifiers
- `__declspec(align(N))` - Alignment
- `__declspec(dllimport)` / `__declspec(dllexport)` - DLL linkage
- `__declspec(naked)` - Naked function
- `__declspec(noinline)` - No inlining
- `__declspec(noreturn)` - No return
- `__declspec(restrict)` - Pointer restriction
- `__declspec(thread)` - Thread-local storage
- `__declspec(uuid("..."))` - UUID
- `__declspec(selectany)` - Select any
- `__declspec(deprecated)` - Deprecation

### Intrinsics
- `__assume(expr)` - Optimization hint
- `__noop` - No operation
- `__debugbreak()` - Debug breakpoint
- `_ReturnAddress()` - Return address
- `_AddressOfReturnAddress()` - Stack location
- `_byteswap_ushort/ulong/uint64` - Byte swap
- `_rotl/_rotr` - Rotate left/right
- `_lrotl/_lrotr` - Long rotate
- `_BitScanForward/_BitScanReverse` - Bit scan
- `__ll_lshift/__ll_rshift` - 64-bit shifts
- `__umulh/__mulh` - High multiplication
- `__readmsr/__writemsr` - MSR access
- `__cpuid` - CPU ID
- `__rdtsc` - Read time-stamp counter

### Pragmas
- `#pragma pack(N)` - Structure packing
- `#pragma warning(disable: N)` - Warning control
- `#pragma comment(lib, "name")` - Library linking
- `#pragma section("name", flags)` - Section creation
- `#pragma code_seg("name")` - Code segment
- `#pragma data_seg("name")` - Data segment
- `#pragma once` - Include guard
- `#pragma optimize("", on|off)` - Optimization control

### Microsoft-Specific Keywords
- `__int8`, `__int16`, `__int32`, `__int64` - Fixed-width integers
- `__ptr32`, `__ptr64` - Pointer sizing
- `__unaligned` - Unaligned access
- `__forceinline` - Force inlining
- `__restrict` - Pointer restriction
- `__sptr`, `__uptr` - Signed/unsigned pointers
- `__w64` - 64-bit portability check
- `__interface` - Interface declaration

### Structured Exception Handling (SEH)
```c
__try {
    // Protected code
}
__except (filter_expression) {
    // Exception handler
}
__finally {
    // Cleanup code
}
```

## Clang Extensions

### Blocks (Apple Extension)
```c
int (^block)(int) = ^(int x) { return x * 2; };
```

### Attributes
- `__attribute__((availability(...)))` - Platform availability
- `__attribute__((objc_arc_weak_reference_unavailable))` - ARC weak
- `__attribute__((swift_name("...")))` - Swift name
- `__attribute__((enable_if(...)))` - Conditional availability
- `__attribute__((diagnose_if(...)))` - Diagnostic condition
- `__attribute__((disable_tail_calls))` - Disable tail calls
- `__attribute__((no_sanitize("...")))` - Sanitizer control
- `__attribute__((require_constant_initialization))` - Const init

### Built-ins
- `__builtin_assume(expr)` - Optimization hint
- `__builtin_unpredictable(expr)` - Branch hint
- `__builtin_readcyclecounter()` - Cycle counter
- `__builtin_shufflevector()` - Vector shuffle
- `__builtin_convertvector()` - Vector conversion
- `__builtin_add_overflow(a, b, &result)` - Overflow checking
- `__builtin_sub_overflow(a, b, &result)`
- `__builtin_mul_overflow(a, b, &result)`
- `__has_feature(x)` - Feature test
- `__has_extension(x)` - Extension test
- `__has_builtin(__builtin_xxx)` - Built-in test
- `__has_attribute(x)` - Attribute test

### Type Traits
- `__is_pod(type)`
- `__is_trivial(type)`
- `__is_trivially_copyable(type)`
- `__is_standard_layout(type)`
- `__is_literal_type(type)`
- `__has_trivial_constructor(type)`
- `__has_trivial_destructor(type)`

### Pragmas
- `#pragma clang diagnostic push/pop`
- `#pragma clang diagnostic ignored "-W..."`
- `#pragma clang optimize on/off`
- `#pragma clang loop vectorize(enable)`
- `#pragma clang assume_nonnull begin/end`

### Sanitizers Support
- AddressSanitizer annotations
- MemorySanitizer annotations
- ThreadSanitizer annotations
- UndefinedBehaviorSanitizer annotations

## Optimization Features

### Optimization Levels
- `-O0` - No optimization (default)
- `-O1` - Basic optimization
- `-O2` - Full optimization
- `-O3` - Aggressive optimization
- `-Os` - Size optimization
- `-Oz` - Aggressive size optimization
- `-Ofast` - Unsafe optimizations
- `-Og` - Debug-friendly optimization

### Optimization Passes
1. **SSA Construction** - Convert to Static Single Assignment
2. **Constant Folding** - Evaluate constants at compile time
3. **Dead Code Elimination** - Remove unreachable code
4. **Common Subexpression Elimination** - Eliminate redundant computations
5. **Loop Invariant Code Motion** - Move loop-invariant code outside loops
6. **Inline Expansion** - Inline small functions
7. **Tail Call Optimization** - Convert tail calls to jumps
8. **Register Allocation** - Graph coloring algorithm
9. **Instruction Scheduling** - Reorder for better pipeline usage
10. **Peephole Optimization** - Local instruction optimization

### MMIX-Specific Optimizations
- Use of 256 registers
- Compressed instruction selection
- Vector instruction utilization
- Matrix operation fusion
- Branch prediction hints
- Cache prefetch generation

## Code Generation

### Register Allocation
- 256 general-purpose registers
- Local/global register distinction
- Register windows for function calls
- Spilling to memory when needed

### Calling Convention
- Arguments in $0-$7
- Return value in $0
- Callee-saved registers
- Stack frame layout
- Structure return

### Exception Handling
- C++ exception support
- SEH support (Windows)
- Dwarf unwinding (Unix)
- CodeView debug info

## Debug Information

### CodeView (MSVC Compatible)
- Symbol information
- Type information
- Line number information
- Source file information
- Local variable information

### DWARF (GCC/Clang Compatible)
- `.debug_info` - Debug information
- `.debug_line` - Line number information
- `.debug_frame` - Stack frame information
- `.debug_abbrev` - Abbreviation tables
- `.debug_str` - String table

## Implementation Phases

### Phase 1: Core C23 Compiler
- Lexer and preprocessor
- Parser for C23 syntax
- AST generation
- Basic type checking
- Simple code generation

### Phase 2: Optimization Infrastructure
- IR design and implementation
- SSA construction
- Basic optimization passes
- Register allocation

### Phase 3: Extension Support
- GNU extensions
- MSVC extensions
- Clang extensions
- Attribute processing

### Phase 4: Debug Information
- CodeView generation
- DWARF generation
- Source-level debugging support

### Phase 5: Advanced Features
- Link-time optimization (LTO)
- Profile-guided optimization (PGO)
- Sanitizer instrumentation
- Coverage instrumentation

## Testing Strategy

- Comprehensive test suite
- Compatibility with GCC, MSVC, Clang test suites
- Regression testing
- Performance benchmarks
- Self-hosting validation

## File Structure

```
compiler/
├── frontend/
│   ├── lexer.c          - Tokenization
│   ├── preprocessor.c   - Macro expansion
│   ├── parser.c         - Syntax analysis
│   └── sema.c           - Semantic analysis
├── ir/
│   ├── builder.c        - IR construction
│   ├── types.c          - Type system
│   └── optimize.c       - Optimization passes
├── backend/
│   ├── codegen.c        - MMIX code generation
│   ├── regalloc.c       - Register allocation
│   └── emit.c           - Assembly output
├── debug/
│   ├── codeview.c       - CodeView generation
│   └── dwarf.c          - DWARF generation
└── driver/
    └── main.c           - Compiler driver
```

## Command-Line Interface

```bash
mmix-cc [options] file...

Options:
  -c              Compile only, don't link
  -o <file>       Output file name
  -O[0-3sz]       Optimization level
  -g              Generate debug information
  -std=c23        Language standard
  -Wall           Enable all warnings
  -Werror         Treat warnings as errors
  -D<macro>       Define macro
  -I<dir>         Include directory
  -L<dir>         Library directory
  -l<lib>         Link library
  -fPIC           Position-independent code
  -fno-<feature>  Disable feature
```

## Compatibility

- Full C23 standard compliance
- GNU extension compatibility for portability
- MSVC extension support for Windows code
- Clang extension support for modern features
- Self-hosting capability
