# Simple MMIX Test Program
#
# This program demonstrates basic MMIX instructions:
# - Integer arithmetic
# - Memory load/store
# - Conditional branches
# - Floating-point operations
#
# To assemble (theoretical - requires MMIX assembler):
#   mmixal simple_test.asm -o simple_test.mmo
#
# To run:
#   mmix-emulator simple_test.bin

        .text
        .global _start

_start:
        # Initialize some values
        SETH    $1,#1234        # Load high immediate
        ORMH    $1,#5678        # OR middle-high
        ORML    $1,#9ABC        # OR middle-low
        ORL     $1,#DEF0        # OR low

        # Basic arithmetic
        ADDU    $2,$0,#100      # $2 = 100
        ADDU    $3,$0,#50       # $3 = 50
        ADD     $4,$2,$3        # $4 = 150
        SUB     $5,$2,$3        # $5 = 50
        MUL     $6,$2,$3        # $6 = 5000

        # Memory operations
        STO     $4,memory,0     # Store 150 to memory
        LDO     $7,memory,0     # Load from memory into $7

        # Comparison and branching
        CMP     $8,$2,$3        # Compare $2 and $3
        BP      $8,positive     # Branch if positive
        JMP     end

positive:
        ADDU    $9,$0,#1        # Set flag

        # Floating-point operations
        FLOT    $10,$2          # Convert 100 to float
        FLOT    $11,$3          # Convert 50 to float
        FADD    $12,$10,$11     # FP add
        FMUL    $13,$10,$11     # FP multiply
        FSQRT   $14,$10         # FP square root

end:
        # Halt (theoretical - would trap or exit)
        TRAP    0,0,0

        .data
memory:
        OCTA    0               # Storage location

# Expected results:
# $2 = 100
# $3 = 50
# $4 = 150
# $5 = 50
# $6 = 5000
# $7 = 150
# $9 = 1
# $12 = 150.0 (in FP format)
# $13 = 5000.0 (in FP format)
# $14 = 10.0 (in FP format)
