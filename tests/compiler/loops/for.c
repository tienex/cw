// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test for loops

// CHECK-LABEL: test_classic_for
int test_classic_for(int n) {
    int sum = 0;
    // CHECK: CMPU
    // CHECK: BNZ
    for (int i = 0; i < n; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}

// CHECK-LABEL: test_for_without_init
int test_for_without_init(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: CMPU
    for (; i < n; i = i + 1) {
        sum = sum + i;
    }
    return sum;
}

// CHECK-LABEL: test_for_without_increment
int test_for_without_increment(int n) {
    int sum = 0;
    // CHECK: CMPU
    for (int i = 0; i < n;) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

// CHECK-LABEL: test_infinite_for_with_break
int test_infinite_for_with_break() {
    int i = 0;
    // CHECK: CMPU
    for (;;) {
        if (i >= 10) {
            // CHECK: JMP
            break;
        }
        i = i + 1;
    }
    return i;
}

// CHECK-LABEL: test_for_with_continue
int test_for_with_continue(int n) {
    int sum = 0;
    // CHECK: CMPU
    for (int i = 0; i < n; i = i + 1) {
        if (i == 5) {
            // CHECK: JMP
            continue;
        }
        sum = sum + i;
    }
    return sum;
}

// CHECK-LABEL: test_nested_for
int test_nested_for(int n) {
    int sum = 0;
    // CHECK: CMPU
    for (int i = 0; i < n; i = i + 1) {
        // CHECK: CMPU
        for (int j = 0; j < i; j = j + 1) {
            sum = sum + 1;
        }
    }
    return sum;
}

// CHECK-LABEL: main
int main() {
    int r1 = test_classic_for(10);
    int r2 = test_for_without_init(10);
    int r3 = test_for_without_increment(10);
    int r4 = test_infinite_for_with_break();
    int r5 = test_for_with_continue(10);
    int r6 = test_nested_for(5);
    return r1 + r2 + r3 + r4 + r5 + r6;
}
