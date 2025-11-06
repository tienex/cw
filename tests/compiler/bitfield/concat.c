// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test bit concatenation operations

// CHECK-LABEL: test_concat_bytes
int test_concat_bytes(int a, int b) {
    // CHECK: SLU {{.*}},{{.*}},8
    // CHECK: OR
    return a .. b;
}

// CHECK-LABEL: test_concat_nibbles
int test_concat_nibbles(int n1, int n2, int n3, int n4) {
    // Multiple concatenations
    // CHECK: SLU
    // CHECK: OR
    // CHECK: SLU
    // CHECK: OR
    return (n1 .. n2) .. (n3 .. n4);
}

// CHECK-LABEL: test_concat_chain
int test_concat_chain(int a, int b, int c) {
    // CHECK: SLU
    // CHECK: OR
    // CHECK: SLU
    // CHECK: OR
    return a .. b .. c;
}

// CHECK-LABEL: test_concat_with_literals
int test_concat_with_literals() {
    // CHECK: SLU
    // CHECK: OR
    return 0xAB .. 0xCD;
}

// CHECK-LABEL: test_concat_and_extract
int test_concat_and_extract(int a, int b) {
    // Concatenate two bytes, then extract middle nibble
    int combined = a .. b;
    return combined[4:4];
}

// CHECK-LABEL: main
int main() {
    int byte1 = 0x12;
    int byte2 = 0x34;
    int byte3 = 0x56;
    int byte4 = 0x78;

    int word1 = test_concat_bytes(byte1, byte2);
    int word2 = test_concat_nibbles(byte1, byte2, byte3, byte4);
    int word3 = test_concat_chain(byte1, byte2, byte3);
    int word4 = test_concat_with_literals();

    return word1 + word2 + word3 + word4;
}
