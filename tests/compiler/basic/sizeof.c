// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test sizeof operator

// CHECK-LABEL: test_basic_types
int test_basic_types() {
    // CHECK: ADDU {{.*}},{{.*}},1
    // CHECK: ADDU {{.*}},{{.*}},2
    // CHECK: ADDU {{.*}},{{.*}},4
    // CHECK: ADDU {{.*}},{{.*}},8
    int result = 0;
    result += sizeof(char);       // 1
    result += sizeof(short);      // 2
    result += sizeof(int);        // 4
    result += sizeof(long);       // 8
    return result;  // Expected: 15
}

// CHECK-LABEL: test_pointer_sizeof
int test_pointer_sizeof() {
    int *ptr;
    char *cptr;
    // All pointers are 8 bytes on 64-bit MMIX
    // CHECK: ADDU {{.*}},8,8
    return sizeof(ptr) + sizeof(cptr);  // Expected: 16
}

// CHECK-LABEL: test_array_sizeof
int test_array_sizeof() {
    int arr[10];
    char carr[5];
    // Arrays: element_size * count
    // CHECK: ADDU {{.*}},40,5
    return sizeof(arr) + sizeof(carr);  // Expected: 45
}

// CHECK-LABEL: test_sizeof_expr
int test_sizeof_expr() {
    int x = 42;
    int y = 100;
    // sizeof on expressions uses type of expression
    // CHECK: ADDU {{.*}},4,4
    // CHECK: ADDU {{.*}},{{.*}},4
    return sizeof(x) + sizeof(x + y) + sizeof(10);  // Expected: 12
}

// CHECK-LABEL: test_sizeof_types
int test_sizeof_types() {
    // Test sizeof with explicit types
    // CHECK: ADDU {{.*}},{{.*}},1
    // CHECK: ADDU {{.*}},{{.*}},4
    // CHECK: ADDU {{.*}},{{.*}},8
    int result = 0;
    result += sizeof(char);
    result += sizeof(int);
    result += sizeof(long);
    return result;  // Expected: 13
}

// CHECK-LABEL: main
int main() {
    int result = 0;
    result += test_basic_types();
    result += test_pointer_sizeof();
    result += test_array_sizeof();
    result += test_sizeof_expr();
    result += test_sizeof_types();
    return result;
}
