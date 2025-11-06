// RUN: mmix-cc %s -o %t.s
// RUN: mmix-filecheck %s < %t.s

// Test pointer to array elements (arrays are stack-allocated)

int test_array_pointer() {
    int arr[3];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;

    // Array name is already an address
    return arr[1];
}

// CHECK-LABEL: test_array_pointer:
// CHECK: ALLOCA
// CHECK: STO
// CHECK: LDO

int main() {
    return test_array_pointer();
}

// CHECK-LABEL: main:
// CHECK: PUSHJ
