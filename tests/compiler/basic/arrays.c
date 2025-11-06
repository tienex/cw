// RUN: mmix-cc %s -o %t.s
// RUN: mmix-filecheck %s < %t.s

// Test basic array allocation and assignment

int test_array_alloc() {
    int arr[5];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    return arr[1];
}

// CHECK-LABEL: test_array_alloc:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: STO
// CHECK: STO

int test_array_sum() {
    int nums[3];
    nums[0] = 5;
    nums[1] = 10;
    nums[2] = 15;

    int sum = 0;
    sum = sum + nums[0];
    sum = sum + nums[1];
    sum = sum + nums[2];
    return sum;
}

// CHECK-LABEL: test_array_sum:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: LDO
// CHECK: ADDU

int test_array_loop() {
    int arr[4];
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    arr[3] = 4;

    int total = 0;
    for (int i = 0; i < 4; i = i + 1) {
        total = total + arr[i];
    }
    return total;
}

// CHECK-LABEL: test_array_loop:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: ADDU
// CHECK: LDO

int main() {
    return test_array_alloc() + test_array_sum() + test_array_loop();
}

// CHECK-LABEL: main:
// CHECK: PUSHJ
