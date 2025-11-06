// RUN: mmix-cc %s -o %t.s
// RUN: mmix-filecheck %s < %t.s

// Test rotation operators (MMIX extensions)

int test_rotate_left() {
    int x = 0x12345678;
    int result = x <<> 8;
    return result;
}

// CHECK-LABEL: test_rotate_left:
// CHECK: SLU {{.*}},{{.*}},8
// CHECK: SUBU {{.*}},64,8
// CHECK: SRU
// CHECK: OR

int test_rotate_right() {
    int x = 0xABCDEF00;
    int result = x >>< 4;
    return result;
}

// CHECK-LABEL: test_rotate_right:
// CHECK: SRU {{.*}},{{.*}},4
// CHECK: SUBU {{.*}},64,4
// CHECK: SLU
// CHECK: OR

int test_rotate_variable() {
    int value = 0xFF00FF00;
    int shift = 16;
    int left = value <<> shift;
    int right = value >>< shift;
    return left + right;
}

// CHECK-LABEL: test_rotate_variable:
// CHECK: SLU
// CHECK: SUBU
// CHECK: SRU
// CHECK: OR
// CHECK: SRU
// CHECK: SUBU
// CHECK: SLU
// CHECK: OR

int test_rotate_expression() {
    int x = 100;
    int y = 50;
    int result = (x + y) <<> 3;
    return result;
}

// CHECK-LABEL: test_rotate_expression:
// CHECK: ADDU
// CHECK: SLU
// CHECK: SUBU
// CHECK: SRU
// CHECK: OR

int main() {
    return test_rotate_left() + test_rotate_right() +
           test_rotate_variable() + test_rotate_expression();
}

// CHECK-LABEL: main:
// CHECK: PUSHJ
