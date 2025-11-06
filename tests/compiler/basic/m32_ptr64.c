// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s
// RUN: %mmix-cc -m32 %s -o %t_m32.s

// Test -m32 mode and __ptr64/__ptr32 qualifiers

// CHECK-LABEL: test_normal_pointers
int test_normal_pointers() {
    int *ptr;
    char *cptr;
    // In normal 64-bit mode, pointers are 8 bytes
    // CHECK: ADDU {{.*}},8,8
    return sizeof(ptr) + sizeof(cptr);  // Expected: 16
}

// CHECK-LABEL: test_ptr32_in_64bit
int test_ptr32_in_64bit() {
    int * __ptr32 ptr32;
    int *ptr_normal;
    // __ptr32 forces 32-bit pointer even in 64-bit mode
    // CHECK: ADDU {{.*}},4,8
    return sizeof(ptr32) + sizeof(ptr_normal);  // Expected: 12
}

// CHECK-LABEL: test_ptr64_always_64
int test_ptr64_always_64() {
    int * __ptr64 ptr64;
    // __ptr64 always forces 64-bit pointer
    // CHECK: SET {{.*}},8
    return sizeof(ptr64);  // Expected: 8
}

// CHECK-LABEL: main
int main() {
    int result = 0;
    result += test_normal_pointers();
    result += test_ptr32_in_64bit();
    result += test_ptr64_always_64();
    return result;
}

/*
 * Expected behavior:
 *
 * Normal mode (64-bit):
 *   - int *ptr          → 8 bytes
 *   - int * __ptr32 ptr → 4 bytes
 *   - int * __ptr64 ptr → 8 bytes
 *
 * -m32 mode (32-bit):
 *   - int *ptr          → 4 bytes
 *   - int * __ptr32 ptr → 4 bytes (redundant)
 *   - int * __ptr64 ptr → 8 bytes (upgrade)
 */
