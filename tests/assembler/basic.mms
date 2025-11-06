// RUN: %mmix-as %s -o %t.bin
// RUN: %filecheck %s < %t.bin

// Test basic MMIX assembly

// Simple arithmetic
main:
    SET     $0,10           // Load immediate
    SET     $1,20           // Load immediate
    ADDU    $2,$0,$1        // Add
    SUBU    $3,$1,$0        // Subtract
    MULU    $4,$0,$1        // Multiply
    DIVU    $5,$1,$0        // Divide

// Bitwise operations
    AND     $10,$0,$1       // Bitwise AND
    OR      $11,$0,$1       // Bitwise OR
    XOR     $12,$0,$1       // Bitwise XOR
    SLU     $13,$0,2        // Shift left
    SRU     $14,$1,2        // Shift right

// Memory operations
    STB     $0,mem          // Store byte
    LDB     $15,mem         // Load byte
    STW     $1,mem          // Store wyde
    LDW     $16,mem         // Load wyde
    STT     $2,mem          // Store tetra
    LDT     $17,mem         // Load tetra
    STO     $3,mem          // Store octa
    LDO     $18,mem         // Load octa

// Control flow
    JMP     skip            // Unconditional jump
    TRAP    0,Halt,0        // Should not execute

skip:
    CMPU    $20,$0,$1       // Compare unsigned
    BNZ     $20,end         // Branch if non-zero
    TRAP    0,Halt,0        // Should not execute

end:
    TRAP    0,Halt,0        // Halt

mem:
    OCTA    0
