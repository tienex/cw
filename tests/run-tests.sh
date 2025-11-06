#!/bin/bash

# MMIX Compiler Test Runner
# Runs all test files and reports results

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test configuration
MMIX_CC="../bin/mmix-cc"
FILECHECK="../bin/mmix-filecheck"
TEST_TEMP_DIR="./temp"

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Check if compiler exists
if [ ! -f "$MMIX_CC" ]; then
    echo -e "${RED}Error: Compiler not found at $MMIX_CC${NC}"
    echo "Please run 'make' to build the compiler first."
    exit 1
fi

# Check if filecheck exists
if [ ! -f "$FILECHECK" ]; then
    echo -e "${RED}Error: FileCheck not found at $FILECHECK${NC}"
    echo "Please run 'make' to build FileCheck first."
    exit 1
fi

# Create temp directory
mkdir -p "$TEST_TEMP_DIR"

echo "MMIX Compiler Test Suite"
echo "========================"
echo ""

# Function to run a single test
run_test() {
    local test_file=$1
    local test_name=$(basename "$test_file" .c)
    local test_dir=$(dirname "$test_file")
    local temp_output="$TEST_TEMP_DIR/${test_name}.s"

    TOTAL_TESTS=$((TOTAL_TESTS + 1))

    echo -n "Testing $test_file ... "

    # Compile the test
    if $MMIX_CC "$test_file" -o "$temp_output" 2>/dev/null; then
        # Run FileCheck
        if $FILECHECK "$test_file" -input "$temp_output" > /dev/null 2>&1; then
            echo -e "${GREEN}PASS${NC}"
            PASSED_TESTS=$((PASSED_TESTS + 1))
            return 0
        else
            echo -e "${RED}FAIL${NC} (FileCheck failed)"
            FAILED_TESTS=$((FAILED_TESTS + 1))
            return 1
        fi
    else
        echo -e "${RED}FAIL${NC} (Compilation failed)"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return 1
    fi
}

# Function to run tests in a directory
run_tests_in_dir() {
    local dir=$1
    local category=$(basename "$dir")

    echo -e "${YELLOW}Running $category tests...${NC}"

    for test_file in "$dir"/*.c; do
        if [ -f "$test_file" ]; then
            run_test "$test_file"
        fi
    done

    echo ""
}

# Run all tests
for test_category in compiler/basic compiler/bitfield compiler/loops compiler/functions; do
    if [ -d "$test_category" ]; then
        run_tests_in_dir "$test_category"
    fi
done

# Clean up temp directory
rm -rf "$TEST_TEMP_DIR"

# Print summary
echo "========================"
echo "Test Summary"
echo "========================"
echo "Total tests:  $TOTAL_TESTS"
echo -e "Passed:       ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed:       ${RED}$FAILED_TESTS${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
