// RUN: %mmix-as %s -o %t.bin

// Test labels and jumps

main:
    SET     $0,0
    JMP     label1

label1:
    ADDU    $0,$0,1
    JMP     label2

label2:
    ADDU    $0,$0,2
    JMP     label3

label3:
    ADDU    $0,$0,3
    CMPU    $1,$0,6         // Should be 6
    BZ      $1,success
    TRAP    0,Halt,1        // Fail

success:
    TRAP    0,Halt,0        // Success
