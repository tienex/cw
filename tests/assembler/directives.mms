// RUN: %mmix-as %s -o %t.bin

// Test assembler directives

    .text
    .globl main

main:
    SET     $0,value1
    SET     $1,value2
    ADDU    $2,$0,$1
    TRAP    0,Halt,0

    .data
value1:
    OCTA    42
value2:
    OCTA    100

    .bss
buffer:
    .skip   1024            // Reserve 1024 bytes
