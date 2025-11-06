// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test explicit bit-width suffixes for integer literals

// CHECK-LABEL: test_nibble_concat
int test_nibble_concat() {
    // Explicit 4-bit values
    // CHECK: SLU {{.*}},10,4
    // CHECK: OR
    return 0xAui4 .. 0xBui4;  // Expected: 0xAB
}

// CHECK-LABEL: test_byte_concat
int test_byte_concat() {
    // Explicit 8-bit values
    // CHECK: SLU {{.*}},171,8
    // CHECK: OR
    return 0xABui8 .. 0xCDui8;  // Expected: 0xABCD
}

// CHECK-LABEL: test_mixed_widths
int test_mixed_widths() {
    // Mix of different bit widths
    // CHECK: SLU {{.*}},1,8
    // CHECK: OR
    // CHECK: SLU {{.*}},{{.*}},4
    // CHECK: OR
    return (0x1ui8 .. 0xFFui8) .. 0xFui4;  // 1 + FF (16 bits) then concat with F (4 bits)
}

// CHECK-LABEL: test_signed_suffix
int test_signed_suffix() {
    // Signed bit-width suffix (i8 without u)
    // CHECK: SLU {{.*}},127,8
    // CHECK: OR
    return 0x7Fi8 .. 0xABui8;  // 7F is max positive for signed 8-bit
}

// CHECK-LABEL: test_no_suffix_default
int test_no_suffix_default() {
    // No suffix should default to 8-bit shift (backward compatibility)
    // CHECK: SLU {{.*}},10,8
    // CHECK: OR
    return 0xA .. 0xB;
}

// CHECK-LABEL: main
int main() {
    int result = 0;
    result += test_nibble_concat();
    result += test_byte_concat();
    result += test_mixed_widths();
    result += test_signed_suffix();
    result += test_no_suffix_default();
    return result;
}
