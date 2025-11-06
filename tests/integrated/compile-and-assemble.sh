#!/bin/bash
# Test integrated toolchain: compile C to assembly and assemble to binary

set -e

MMIX_CC="../../bin/mmix-cc"
MMIX_AS="../../bin/mmix-as"
TEST_SRC="test_simple.c"
TEST_ASM="test_simple.s"
TEST_BIN="test_simple.bin"

# Create test C file
cat > "$TEST_SRC" << 'EOF'
int add(int a, int b) {
    return a + b;
}

int main() {
    int result = add(5, 10);
    return result;
}
EOF

echo "Step 1: Compiling C to MMIX assembly..."
$MMIX_CC "$TEST_SRC" -o "$TEST_ASM"

if [ ! -f "$TEST_ASM" ]; then
    echo "FAIL: Compilation failed"
    exit 1
fi

echo "Step 2: Assembling MMIX assembly to binary..."
$MMIX_AS "$TEST_ASM" -o "$TEST_BIN"

if [ ! -f "$TEST_BIN" ]; then
    echo "FAIL: Assembly failed"
    exit 1
fi

echo "Step 3: Verifying output..."
if [ -s "$TEST_BIN" ]; then
    echo "PASS: Binary generated successfully"
    ls -lh "$TEST_BIN"
else
    echo "FAIL: Binary is empty"
    exit 1
fi

# Cleanup
rm -f "$TEST_SRC" "$TEST_ASM" "$TEST_BIN"

echo ""
echo "Integrated toolchain test PASSED"
