#!/bin/bash

# TCP Test Runner for timed server
# This script runs TCP tests with various format strings

# Text colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Initialize counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Start time
START_TIME=$(date +%s)

echo -e "${YELLOW}Starting TCP tests for timed server${NC}"
echo

# Change to the test directory (go up one level from scripts)
cd "$(dirname "$0")/.." || { echo "Failed to change to test directory"; exit 1; }

# Make sure the test client is compiled
if [ ! -x "bin/test_client" ]; then
    echo -e "${RED}Error: test_client binary not found or not executable${NC}"
    echo "Make sure to compile the test client first."
    exit 1
fi

# Test each file in the tcp directory
for test_file in tcp/*.txt; do
    # Skip if the pattern didn't match any files
    if [ ! -f "$test_file" ]; then
        echo -e "${RED}No test files found in tcp/ directory${NC}"
        break
    fi
    
    # Extract test name from file path
    test_name=$(basename "$test_file" .txt)
    
    # Increment test counter
    TESTS_RUN=$((TESTS_RUN + 1))
    
    echo -e "${YELLOW}Running TCP test: $test_name${NC}"
    
    # Show format string
    if [ -s "$test_file" ]; then
        format_content=$(cat "$test_file")
        echo -e "Format string: ${CYAN}'$format_content'${NC}"
    else
        echo -e "Format string: ${CYAN}(empty - using server default)${NC}"
    fi
    
    # Run the test
    echo "Response:"
    bin/test_client < "$test_file"
    test_result=$?
    
    # Check status
    if [ $test_result -eq 0 ]; then
        echo -e "${GREEN}✓ Test passed${NC}"
        TESTS_PASSED=$((TESTS_PASSED + 1))
    else
        echo -e "${RED}✗ Test failed (exit code: $test_result)${NC}"
        TESTS_FAILED=$((TESTS_FAILED + 1))
    fi
    
    echo -e "-------------------------------------------\n"
done

# Calculate elapsed time
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

# Print summary
echo -e "${YELLOW}TCP Tests Summary:${NC}"
echo -e "Tests run:    $TESTS_RUN"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"
echo -e "Time elapsed: ${ELAPSED} seconds"

# Save summary to temp file for master script - use current directory
echo "TESTS_RUN=$TESTS_RUN" > scripts/tcp_summary.txt
echo "TESTS_PASSED=$TESTS_PASSED" >> scripts/tcp_summary.txt
echo "TESTS_FAILED=$TESTS_FAILED" >> scripts/tcp_summary.txt

# Set exit code based on test results
if [ $TESTS_FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi