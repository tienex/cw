// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test variable declarations and assignments

// CHECK-LABEL: test_local_variables
int test_local_variables() {
    // CHECK: SET
    int a = 10;
    // CHECK: SET
    int b = 20;
    // CHECK: SET
    int c = 30;

    // CHECK: ADDU
    int sum = a + b + c;
    return sum;
}

// CHECK-LABEL: test_uninitialized_variables
int test_uninitialized_variables() {
    int x;
    // CHECK: SET
    x = 42;
    return x;
}

// CHECK-LABEL: test_multiple_declarations
int test_multiple_declarations() {
    // CHECK: SET
    int a = 1;
    // CHECK: SET
    int b = 2;
    // CHECK: SET
    int c = 3;
    return a + b + c;
}

// CHECK-LABEL: test_assignment_operators
int test_assignment_operators() {
    // CHECK: SET
    int x = 10;

    // CHECK: ADDU
    x = x + 5;  // x += 5 equivalent

    // CHECK: SUBU
    x = x - 3;  // x -= 3 equivalent

    // CHECK: MULU
    x = x * 2;  // x *= 2 equivalent

    return x;
}

// CHECK-LABEL: test_compound_assignments
int test_compound_assignments() {
    // CHECK: SET
    int x = 100;

    // CHECK: ADDU
    x += 10;

    // CHECK: SUBU
    x -= 5;

    // CHECK: MULU
    x *= 2;

    // CHECK: DIVU
    x /= 3;

    return x;
}

// CHECK-LABEL: test_nested_scopes
int test_nested_scopes() {
    // CHECK: SET
    int x = 10;
    int sum = x;

    {
        // CHECK: SET
        int y = 20;
        sum = sum + y;

        {
            // CHECK: SET
            int z = 30;
            sum = sum + z;
        }
    }

    return sum;
}

// CHECK-LABEL: main
int main() {
    int r1 = test_local_variables();
    int r2 = test_uninitialized_variables();
    int r3 = test_multiple_declarations();
    int r4 = test_assignment_operators();
    int r5 = test_compound_assignments();
    int r6 = test_nested_scopes();
    return r1 + r2 + r3 + r4 + r5 + r6;
}
