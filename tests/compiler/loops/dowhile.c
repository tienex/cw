// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test do-while loops

// CHECK-LABEL: test_simple_dowhile
int test_simple_dowhile(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: ADDU
    do {
        sum = sum + i;
        i = i + 1;
        // CHECK: CMPU
        // CHECK: BNZ
    } while (i < n);
    return sum;
}

// CHECK-LABEL: test_dowhile_executes_once
int test_dowhile_executes_once() {
    int count = 0;
    // CHECK: ADDU
    do {
        count = count + 1;
    } while (0);
    return count;  // Should return 1
}

// CHECK-LABEL: test_dowhile_with_break
int test_dowhile_with_break(int n) {
    int i = 0;
    // CHECK: CMPU
    do {
        if (i == 5) {
            // CHECK: JMP
            break;
        }
        i = i + 1;
    } while (i < n);
    return i;
}

// CHECK-LABEL: test_dowhile_with_continue
int test_dowhile_with_continue(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: ADDU
    do {
        i = i + 1;
        if (i == 5) {
            // CHECK: JMP
            continue;
        }
        sum = sum + i;
    } while (i < n);
    return sum;
}

// CHECK-LABEL: test_nested_dowhile
int test_nested_dowhile(int n) {
    int sum = 0;
    int i = 0;
    // CHECK: ADDU
    do {
        int j = 0;
        // CHECK: ADDU
        do {
            sum = sum + 1;
            j = j + 1;
        } while (j < i);
        i = i + 1;
    } while (i < n);
    return sum;
}

// CHECK-LABEL: main
int main() {
    int r1 = test_simple_dowhile(10);
    int r2 = test_dowhile_executes_once();
    int r3 = test_dowhile_with_break(10);
    int r4 = test_dowhile_with_continue(10);
    int r5 = test_nested_dowhile(5);
    return r1 + r2 + r3 + r4 + r5;
}
