// RUN: %mmix-cc %s -o %t.s
// RUN: %filecheck %s < %t.s

// Test function calls

// CHECK-LABEL: simple_function
int simple_function() {
    return 42;
}

// CHECK-LABEL: function_with_args
int function_with_args(int a, int b) {
    // CHECK: ADDU
    return a + b;
}

// CHECK-LABEL: function_with_multiple_args
int function_with_multiple_args(int a, int b, int c, int d) {
    // CHECK: ADDU
    // CHECK: ADDU
    // CHECK: ADDU
    return a + b + c + d;
}

// CHECK-LABEL: recursive_factorial
int recursive_factorial(int n) {
    // CHECK: CMPU
    // CHECK: BNZ
    if (n <= 1) {
        return 1;
    }
    // CHECK: PUSHJ
    return n * recursive_factorial(n - 1);
}

// CHECK-LABEL: recursive_fibonacci
int recursive_fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    // CHECK: PUSHJ
    // CHECK: PUSHJ
    return recursive_fibonacci(n - 1) + recursive_fibonacci(n - 2);
}

// CHECK-LABEL: nested_calls
int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int nested_calls(int a, int b, int c) {
    // CHECK: PUSHJ
    // CHECK: PUSHJ
    return add(multiply(a, b), c);
}

// CHECK-LABEL: main
int main() {
    // CHECK: PUSHJ {{.*}},simple_function
    int r1 = simple_function();

    // CHECK: PUSHJ {{.*}},function_with_args
    int r2 = function_with_args(10, 20);

    // CHECK: PUSHJ {{.*}},function_with_multiple_args
    int r3 = function_with_multiple_args(1, 2, 3, 4);

    // CHECK: PUSHJ {{.*}},recursive_factorial
    int r4 = recursive_factorial(5);

    // CHECK: PUSHJ {{.*}},recursive_fibonacci
    int r5 = recursive_fibonacci(7);

    // CHECK: PUSHJ {{.*}},nested_calls
    int r6 = nested_calls(3, 4, 5);

    return r1 + r2 + r3 + r4 + r5 + r6;
}
