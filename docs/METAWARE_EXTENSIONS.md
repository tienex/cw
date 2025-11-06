# MetaWare High C Compiler Extensions

## Overview

MetaWare High C was a highly advanced C compiler for the FM Towns platform (1989) and other systems. It introduced many innovative language extensions, some of which predated their standardization by decades.

## Core Extensions

### 1. Numeric Literal Separators (1989)

Decades before C++14 added this feature, High C supported underscores in numeric literals for improved readability:

```c
// Binary literals with separators
int binary = 0b1111_0000_1010_0101;

// Decimal literals with separators
long big_number = 1_234_567_890;

// Hexadecimal literals with separators
unsigned int addr = 0xDEAD_BEEF;

// Floating-point literals with separators
double pi = 3.141_592_653_589_793;
```

### 2. Nested Functions with Up-Level References

High C supports Pascal-style nested function definitions with full access to parent function variables:

```c
int outer_function(int x) {
    int local_var = 100;

    // Nested function can access outer_function's variables
    int inner_function(int y) {
        return x + y + local_var;  // Up-level reference
    }

    // Can pass nested function as parameter
    return process_values(inner_function, x);
}
```

**Features:**
- Full lexical scoping
- Closure-like behavior
- Trampoline implementation for function pointers
- Can pass nested functions as arguments
- Access to all parent function variables

### 3. Non-Local Exits from Nested Functions

Nested functions can `goto` labels in their parent functions, enabling sophisticated control flow:

```c
int search_with_early_exit(int *array, int size) {
    found:
    return current_index;

    int search_helper(int start, int end) {
        for (int i = start; i < end; i++) {
            if (array[i] == target) {
                current_index = i;
                goto found;  // Jump back to parent function!
            }
        }
        return -1;
    }

    int current_index = 0;
    int target = 42;
    return search_helper(0, size);
}
```

### 4. Generator Coroutines (1989!)

Python-style generator functions with `yield`, implemented in 1989:

```c
// Generator function declaration
void fibonacci(void) -> (int value) {
    int a = 0, b = 1;

    while (1) {
        yield(a);  // Yield current value and suspend
        int temp = a + b;
        a = b;
        b = temp;
    }
}

// Usage
void test() {
    fibonacci gen;
    int count = 10;

    while (count--) {
        int value;
        if (gen(&value)) {  // Resume generator
            printf("%d\n", value);
        } else {
            break;  // Generator exhausted
        }
    }
}
```

**Generator Syntax:**
```c
return_type function_name(parameters) -> (yield_type yield_name)
```

**Features:**
- State preservation between calls
- Multiple yield points
- Automatic state management
- Resume from last yield point
- Return value indicates if generator is exhausted

### 5. Advanced String Literals

Extended string literal support beyond standard C:

```c
// Multi-line string literals (preserving newlines)
const char *text = "This is a \
multi-line string \
that continues";

// Raw string literals (no escape processing)
const char *path = r"C:\Users\Name\Documents\file.txt";

// String concatenation with auto-whitespace
const char *long_text = "First part"
                        "Second part"
                        "Third part";
```

### 6. Extended Inline Assembly

More flexible inline assembly than standard C:

```c
// Named inputs and outputs
int multiply(int a, int b) {
    int result;
    __asm {
        mov eax, a
        imul eax, b
        mov result, eax
    }
    return result;
}

// Register constraints
void atomic_inc(int *ptr) {
    __asm {
        lock inc dword ptr [ptr]
    }
}
```

### 7. Pragma Directives

Extensive pragma support for fine-grained control:

```c
// Function inlining control
#pragma inline(function_name)
#pragma noinline(function_name)

// Optimization control
#pragma optimize(speed)
#pragma optimize(size)
#pragma optimize(off)

// Alignment control
#pragma pack(1)
struct packed_struct {
    char a;
    int b;
};
#pragma pack()

// Calling convention
#pragma calling_convention(cdecl)
#pragma calling_convention(pascal)
#pragma calling_convention(fastcall)

// Memory model
#pragma memory_model(small)
#pragma memory_model(large)

// Code/data segment control
#pragma code_seg("CODE")
#pragma data_seg("DATA")

// Warning control
#pragma warning(disable: 123)
#pragma warning(enable: 456)
```

### 8. Memory Models (8086/80286/80386)

Full support for x86 memory models:

```c
// Function attributes for memory models
void __near near_function(void);    // Near pointer (16-bit)
void __far far_function(void);      // Far pointer (32-bit)
void __huge huge_function(void);    // Huge pointer (normalized)

// Pointer types
char __near *near_ptr;              // Near data pointer
char __far *far_ptr;                // Far data pointer
char __huge *huge_ptr;              // Huge data pointer

// Memory model specific code
#ifdef __SMALL__
    // Small model code
#endif

#ifdef __LARGE__
    // Large model code
#endif
```

### 9. ROM-able Code Support

Support for embedded systems and ROM execution:

```c
// Const data in ROM
#pragma rom_data
const int lookup_table[] = {1, 2, 3, 4, 5};
#pragma ram_data

// Position-independent code
#pragma pic_code
void embedded_function(void) {
    // Can execute from ROM
}
#pragma normal_code

// Startup code attributes
void __startup init_function(void) {
    // Executed at system initialization
}

void __exit cleanup_function(void) {
    // Executed at system shutdown
}
```

### 10. Extended Type Qualifiers

Additional type qualifiers beyond standard C:

```c
// Interrupt function
void __interrupt timer_handler(void) {
    // Saves all registers
    // Can handle hardware interrupts
}

// Reentrant functions
void __reentrant thread_safe_function(void) {
    // Guaranteed thread-safe
}

// Task functions (for RTOS)
void __task background_task(void) {
    while (1) {
        // Task code
    }
}

// I/O port access
__port unsigned char port_value;
port_value = 0x80;  // Direct port I/O
```

### 11. Built-in Functions

MetaWare-specific built-ins:

```c
// Absolute value (compile-time optimized)
int __abs(int x);

// Min/max (type-generic)
typeof(a) __min(a, b);
typeof(a) __max(a, b);

// Bit manipulation
int __bit_count(unsigned int x);     // Population count
int __leading_zeros(unsigned int x);  // Count leading zeros
int __trailing_zeros(unsigned int x); // Count trailing zeros

// Rotate operations
unsigned int __rotate_left(unsigned int x, int count);
unsigned int __rotate_right(unsigned int x, int count);

// Atomic operations (for multithreading)
int __atomic_add(int *ptr, int value);
int __atomic_swap(int *ptr, int value);
int __compare_and_swap(int *ptr, int old_val, int new_val);

// Compiler information
const char *__COMPILER_VERSION__;
const char *__COMPILE_DATE__;
const char *__COMPILE_TIME__;
```

### 12. Quality Assurance Features

Built-in lint-like checking:

```c
// Unreachable code detection
#pragma warn_unreachable

// Unused variable warnings
#pragma warn_unused

// Type safety checks
#pragma strict_types

// Bounds checking
#pragma array_bounds_check

// Stack overflow detection
#pragma stack_check
```

### 13. Advanced Calling Conventions

Multiple calling conventions with fine control:

```c
// Standard C calling convention
int __cdecl standard_func(int a, int b);

// Pascal calling convention (reverse order)
int __pascal pascal_func(int a, int b);

// Fast calling convention (registers)
int __fastcall fast_func(int a, int b);

// System calling convention (OS-specific)
int __syscall system_func(int a, int b);

// Fortran calling convention
int __fortran fortran_func(int a, int b);
```

### 14. Segment Control

Direct segment management for systems programming:

```c
// Segment pragmas
#pragma segment(name, class, alignment)

// Example usage
#pragma segment("VIDEO", "DATA", 16)
char __based(__segname("VIDEO")) *video_memory;

// Segment operators
unsigned int seg = __segment_of(variable);
unsigned int off = __offset_of(variable);
void __far *ptr = __make_pointer(seg, off);
```

## Implementation Notes

### For MMIX Compiler

When implementing these extensions for MMIX:

1. **Nested Functions**: Use trampoline code on stack or heap
2. **Generators**: Implement as state machines with context switching
3. **Numeric Separators**: Handle in lexer, strip before parsing
4. **Non-Local Exits**: Use setjmp/longjmp mechanism or custom unwinding
5. **Memory Models**: Adapt to MMIX's 64-bit flat address space
6. **Inline Assembly**: Support both MMIX assembly and compatibility layer

### Compatibility Mode

Provide compatibility flags:

```bash
mmix-cc -fmetaware-extensions      # Enable all MetaWare extensions
mmix-cc -fgenerators               # Enable generator coroutines
mmix-cc -fnested-functions         # Enable nested functions with up-level refs
mmix-cc -fnumeric-separators       # Enable underscore in numbers
mmix-cc -fnonlocal-goto            # Enable goto from nested functions
```

## Historical Significance

These extensions were groundbreaking in 1989:

- **Generators** predated Python generators by 12 years
- **Numeric separators** predated C++14 by 25 years
- **Nested functions** still not in standard C/C++
- **Inline assembly** influenced modern compilers

Many of these features demonstrate that advanced PL research concepts (closures, coroutines, etc.) were being productively used in commercial systems programming long before they became mainstream.

## References

- MetaWare High C Programmer's Guide (1985-1992)
- FM Towns Developer Documentation
- "The lost language extensions of MetaWare's High C Compiler" (2023)
