// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test bitwise operations

// CHECK-LABEL: test_and
int test_and(int a, int b) {
    // CHECK: AND
    return a & b;
}

// CHECK-LABEL: test_or
int test_or(int a, int b) {
    // CHECK: OR
    return a | b;
}

// CHECK-LABEL: test_xor
int test_xor(int a, int b) {
    // CHECK: XOR
    return a ^ b;
}

// CHECK-LABEL: test_not
int test_not(int a) {
    // CHECK: XOR {{.*}},{{.*}},0xFFFFFFFFFFFFFFFF
    return ~a;
}

// CHECK-LABEL: test_shl
int test_shl(int a, int b) {
    // CHECK: SLU
    return a << b;
}

// CHECK-LABEL: test_shr
int test_shr(int a, int b) {
    // CHECK: SRU
    return a >> b;
}

// CHECK-LABEL: test_mask
int test_mask(int x) {
    // CHECK: AND
    return x & 0xFF;
}

// CHECK-LABEL: main
int main() {
    int a = 0xAAAA;
    int b = 0x5555;

    int and_result = test_and(a, b);
    int or_result = test_or(a, b);
    int xor_result = test_xor(a, b);
    int not_result = test_not(a);
    int shl_result = test_shl(a, 4);
    int shr_result = test_shr(a, 4);
    int mask_result = test_mask(a);

    return and_result + or_result + xor_result + not_result +
           shl_result + shr_result + mask_result;
}
