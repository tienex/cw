#!/usr/bin/env python3
"""
MMIX Test Binary Generator

This script generates simple MMIX test binaries for testing the emulator.
"""

import struct
import sys

def mmix_inst(opcode, x, y, z):
    """Create a 32-bit MMIX instruction in big-endian format"""
    return struct.pack('>I', (opcode << 24) | (x << 16) | (y << 8) | z)

def mmix_inst_imm16(opcode, x, yz):
    """Create a 32-bit MMIX instruction with 16-bit immediate"""
    return struct.pack('>I', (opcode << 24) | (x << 16) | (yz & 0xFFFF))

def create_arithmetic_test():
    """Create a test binary for arithmetic operations"""
    with open('arithmetic_test.bin', 'wb') as f:
        # Initialize registers
        f.write(mmix_inst(0x21, 1, 0, 100))   # ADDI $1,$0,100
        f.write(mmix_inst(0x21, 2, 0, 50))    # ADDI $2,$0,50

        # Arithmetic operations
        f.write(mmix_inst(0x20, 3, 1, 2))     # ADD $3,$1,$2  = 150
        f.write(mmix_inst(0x24, 4, 1, 2))     # SUB $4,$1,$2  = 50
        f.write(mmix_inst(0x28, 5, 1, 2))     # MUL $5,$1,$2  = 5000
        f.write(mmix_inst(0x2C, 6, 1, 2))     # DIV $6,$1,$2  = 2

        # Logical operations
        f.write(mmix_inst(0x40, 7, 1, 2))     # AND $7,$1,$2
        f.write(mmix_inst(0x42, 8, 1, 2))     # OR $8,$1,$2
        f.write(mmix_inst(0x44, 9, 1, 2))     # XOR $9,$1,$2

        # Halt (invalid instruction to stop)
        f.write(mmix_inst(0xFF, 0, 0, 0))

    print("Created: arithmetic_test.bin")

def create_memory_test():
    """Create a test binary for memory operations"""
    with open('memory_test.bin', 'wb') as f:
        # Initialize values
        f.write(mmix_inst(0x21, 1, 0, 42))    # ADDI $1,$0,42
        f.write(mmix_inst(0x21, 2, 0, 100))   # ADDI $2,$0,100 (address)

        # Store operations
        f.write(mmix_inst(0x16, 1, 2, 0))     # STO $1,$2,0
        f.write(mmix_inst(0x14, 1, 2, 8))     # STT $1,$2,8
        f.write(mmix_inst(0x12, 1, 2, 12))    # STW $1,$2,12
        f.write(mmix_inst(0x10, 1, 2, 14))    # STB $1,$2,14

        # Load operations
        f.write(mmix_inst(0x06, 3, 2, 0))     # LDO $3,$2,0
        f.write(mmix_inst(0x04, 4, 2, 8))     # LDT $4,$2,8
        f.write(mmix_inst(0x02, 5, 2, 12))    # LDW $5,$2,12
        f.write(mmix_inst(0x00, 6, 2, 14))    # LDB $6,$2,14

        # Halt
        f.write(mmix_inst(0xFF, 0, 0, 0))

    print("Created: memory_test.bin")

def create_branch_test():
    """Create a test binary for branch operations"""
    with open('branch_test.bin', 'wb') as f:
        # Initialize
        f.write(mmix_inst(0x21, 1, 0, 10))    # ADDI $1,$0,10
        f.write(mmix_inst(0x21, 2, 0, 0))     # ADDI $2,$0,0 (counter)

        # Loop start (address 8)
        f.write(mmix_inst(0x21, 2, 2, 1))     # ADDI $2,$2,1 (increment)
        f.write(mmix_inst(0x3C, 3, 2, 1))     # CMP $3,$2,$1 (compare with 10)
        f.write(mmix_inst(0x5C, 3, 0, 2))     # BNP $3,2 (branch back if <= 0)

        # After loop
        f.write(mmix_inst(0x21, 4, 0, 99))    # ADDI $4,$0,99 (done marker)

        # Halt
        f.write(mmix_inst(0xFF, 0, 0, 0))

    print("Created: branch_test.bin")

def create_fp_test():
    """Create a test binary for floating-point operations"""
    with open('fp_test.bin', 'wb') as f:
        # Initialize integer values
        f.write(mmix_inst(0x21, 1, 0, 100))   # ADDI $1,$0,100
        f.write(mmix_inst(0x21, 2, 0, 50))    # ADDI $2,$0,50

        # Convert to FP
        f.write(mmix_inst(0x69, 3, 0, 1))     # FLOT $3,$1
        f.write(mmix_inst(0x69, 4, 0, 2))     # FLOT $4,$2

        # FP operations
        f.write(mmix_inst(0x60, 5, 3, 4))     # FADD $5,$3,$4
        f.write(mmix_inst(0x61, 6, 3, 4))     # FSUB $6,$3,$4
        f.write(mmix_inst(0x62, 7, 3, 4))     # FMUL $7,$3,$4
        f.write(mmix_inst(0x63, 8, 3, 4))     # FDIV $8,$3,$4
        f.write(mmix_inst(0x65, 9, 0, 3))     # FSQRT $9,$3

        # Convert back to integer
        f.write(mmix_inst(0x67, 10, 0, 5))    # FIX $10,$5

        # Halt
        f.write(mmix_inst(0xFF, 0, 0, 0))

    print("Created: fp_test.bin")

def create_all_tests():
    """Create all test binaries"""
    create_arithmetic_test()
    create_memory_test()
    create_branch_test()
    create_fp_test()
    print("\nAll test binaries created successfully!")
    print("Run with: ./bin/mmix-emulator -dump <test_file>.bin")

if __name__ == '__main__':
    if len(sys.argv) > 1:
        test_type = sys.argv[1]
        if test_type == 'arithmetic':
            create_arithmetic_test()
        elif test_type == 'memory':
            create_memory_test()
        elif test_type == 'branch':
            create_branch_test()
        elif test_type == 'fp':
            create_fp_test()
        else:
            print(f"Unknown test type: {test_type}")
            print("Available types: arithmetic, memory, branch, fp")
            sys.exit(1)
    else:
        create_all_tests()
