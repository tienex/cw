// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test while loops

// CHECK-LABEL: test_simple_while
int test_simple_while(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: CMPU
    // CHECK: BNZ
    while (i < n) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

// CHECK-LABEL: test_while_with_break
int test_while_with_break(int n) {
    int i = 0;
    // CHECK: CMPU
    while (i < n) {
        if (i == 5) {
            // CHECK: JMP
            break;
        }
        i = i + 1;
    }
    return i;
}

// CHECK-LABEL: test_while_with_continue
int test_while_with_continue(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: CMPU
    while (i < n) {
        i = i + 1;
        if (i == 5) {
            // CHECK: JMP
            continue;
        }
        sum = sum + i;
    }
    return sum;
}

// CHECK-LABEL: test_nested_while
int test_nested_while(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: CMPU
    while (i < n) {
        int j = 0;
        // CHECK: CMPU
        while (j < i) {
            sum = sum + 1;
            j = j + 1;
        }
        i = i + 1;
    }
    return sum;
}

// CHECK-LABEL: main
int main() {
    int result1 = test_simple_while(10);
    int result2 = test_while_with_break(10);
    int result3 = test_while_with_continue(10);
    int result4 = test_nested_while(5);
    return result1 + result2 + result3 + result4;
}
