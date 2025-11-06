// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test bit field extraction operations

// CHECK-LABEL: test_extract_low_nibble
int test_extract_low_nibble(int x) {
    // CHECK: SRU
    // CHECK: SLU {{.*}},1,4
    // CHECK: SUBU
    // CHECK: AND
    return x[0:4];
}

// CHECK-LABEL: test_extract_high_nibble
int test_extract_high_nibble(int x) {
    // CHECK: SRU {{.*}},{{.*}},4
    // CHECK: SLU {{.*}},1,4
    // CHECK: SUBU
    // CHECK: AND
    return x[4:4];
}

// CHECK-LABEL: test_extract_byte
int test_extract_byte(int x) {
    // CHECK: SRU
    // CHECK: SLU {{.*}},1,8
    // CHECK: SUBU
    // CHECK: AND
    return x[0:8];
}

// CHECK-LABEL: test_extract_word
int test_extract_word(int x) {
    // CHECK: SRU {{.*}},{{.*}},16
    // CHECK: SLU {{.*}},1,16
    // CHECK: SUBU
    // CHECK: AND
    return x[16:16];
}

// CHECK-LABEL: test_extract_single_bit
int test_extract_single_bit(int x) {
    // CHECK: SRU {{.*}},{{.*}},7
    // CHECK: SLU {{.*}},1,1
    // CHECK: SUBU
    // CHECK: AND
    return x[7:1];
}

// CHECK-LABEL: test_nested_extract
int test_nested_extract(int x) {
    // Extract high byte, then extract low nibble of that
    int high_byte = x[8:8];
    return high_byte[0:4];
}

// CHECK-LABEL: main
int main() {
    int test_val = 0xABCD1234;

    int low_nibble = test_extract_low_nibble(test_val);
    int high_nibble = test_extract_high_nibble(test_val);
    int byte_val = test_extract_byte(test_val);
    int word_val = test_extract_word(test_val);
    int bit_val = test_extract_single_bit(test_val);

    return low_nibble + high_nibble + byte_val + word_val + bit_val;
}
