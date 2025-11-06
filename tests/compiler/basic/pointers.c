// RUN: mmix-cc %s -o %t.s
// RUN: mmix-filecheck %s < %t.s

// Test basic pointer operations

int test_address_of() {
    int x;
    int *p;

    x = 42;
    p = &x;
    return *p;
}

// CHECK-LABEL: test_address_of:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: LDO

int test_pointer_modify() {
    int a;
    int *ptr;

    a = 10;
    ptr = &a;
    *ptr = 20;
    return a;
}

// CHECK-LABEL: test_pointer_modify:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: STO
// CHECK: LDO

int test_pointer_increment() {
    int value;
    int *p;

    value = 100;
    p = &value;
    *p = *p + 1;
    return value;
}

// CHECK-LABEL: test_pointer_increment:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: LDO
// CHECK: ADDU
// CHECK: STO

int test_multiple_pointers() {
    int x;
    int *p1;
    int *p2;

    x = 5;
    p1 = &x;
    p2 = &x;
    *p1 = 10;
    *p2 = *p2 + 5;
    return x;
}

// CHECK-LABEL: test_multiple_pointers:
// CHECK: SET {{.*}},{{.*}}254
// CHECK: SUBU {{.*}}254,{{.*}}254
// CHECK: STO
// CHECK: STO
// CHECK: LDO
// CHECK: ADDU
// CHECK: STO

int main() {
    return test_address_of() + test_pointer_modify() +
           test_pointer_increment() + test_multiple_pointers();
}

// CHECK-LABEL: main:
// CHECK: PUSHJ
// CHECK: PUSHJ
// CHECK: PUSHJ
// CHECK: PUSHJ
