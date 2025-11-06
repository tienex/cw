// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test basic arithmetic operations

// CHECK-LABEL: test_add
int test_add(int a, int b) {
    // CHECK: ADDU
    return a + b;
}

// CHECK-LABEL: test_sub
int test_sub(int a, int b) {
    // CHECK: SUBU
    return a - b;
}

// CHECK-LABEL: test_mul
int test_mul(int a, int b) {
    // CHECK: MULU
    return a * b;
}

// CHECK-LABEL: test_div
int test_div(int a, int b) {
    // CHECK: DIVU
    return a / b;
}

// CHECK-LABEL: test_mod
int test_mod(int a, int b) {
    // CHECK: DIVU
    return a % b;
}

// CHECK-LABEL: test_compound
int test_compound(int a, int b, int c) {
    // CHECK: MULU
    // CHECK: ADDU
    return a * b + c;
}

// CHECK-LABEL: main
int main() {
    int x = 10;
    int y = 3;
    int z = 5;

    int sum = test_add(x, y);
    int diff = test_sub(x, y);
    int prod = test_mul(x, y);
    int quot = test_div(x, y);
    int rem = test_mod(x, y);
    int comp = test_compound(x, y, z);

    return sum + diff + prod + quot + rem + comp;
}
