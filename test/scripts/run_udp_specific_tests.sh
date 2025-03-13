#!/bin/bash

# UDP-Specific Test Runner for timed server
# This script runs UDP-specific tests that check datagram handling

# Text colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Initialize counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Start time
START_TIME=$(date +%s)

echo -e "${YELLOW}Starting UDP-specific tests for timed server${NC}"
echo

# Change to the test directory (go up one level from scripts)
cd "$(dirname "$0")/.." || { echo "Failed to change to test directory"; exit 1; }

# Make sure the test client is compiled
if [ ! -x "bin/test_client_udp" ]; then
    echo -e "${RED}Error: test_client_udp binary not found or not executable${NC}"
    echo "Make sure to compile the UDP test client first."
    exit 1
fi

# Test each file in the udp_specific directory
for test_file in udp_specific/*.txt; do
    # Skip if the pattern didn't match any files
    if [ ! -f "$test_file" ]; then
        echo -e "${RED}No test files found in udp_specific/ directory${NC}"
        break
    fi
    
    # Extract test name from file path
    test_name=$(basename "$test_file" .txt)
    
    # Increment test counter
    TESTS_RUN=$((TESTS_RUN + 1))
    
    echo -e "${YELLOW}Running UDP-specific test: $test_name${NC}"
    
    # Show test description
    case "$test_name" in
        "mtu_limit")
            echo -e "${PURPLE}Test Description: Testing large format string approaching MTU size${NC}"
            ;;
        "zero_length")
            echo -e "${PURPLE}Test Description: Testing empty datagram handling${NC}"
            ;;
        "malformed")
            echo -e "${PURPLE}Test Description: Testing server response to malformed format string${NC}"
            ;;
        *)
            echo -e "${PURPLE}Test Description: $test_name${NC}"
            ;;
    esac
    
    # Show format string
    if [ -s "$test_file" ]; then
        # For large format strings, just show beginning and end
        if [ $(wc -c < "$test_file") -gt 100 ]; then
            format_start=$(head -c 50 "$test_file")
            format_end=$(tail -c 50 "$test_file")
            echo -e "Format string: ${CYAN}'${format_start}...${format_end}' (truncated)${NC}"
            echo -e "Format string length: $(wc -c < "$test_file") bytes"
        else
            format_content=$(cat "$test_file")
            echo -e "Format string: ${CYAN}'$format_content'${NC}"
        fi
    else
        echo -e "Format string: ${CYAN}(empty)${NC}"
    fi
    
    # Run the test
    echo "Response:"
    bin/test_client_udp < "$test_file"
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

# Special test: Multiple rapid UDP requests
TESTS_RUN=$((TESTS_RUN + 1))
echo -e "${YELLOW}Running UDP-specific test: multiple_rapid_requests${NC}"
echo -e "${PURPLE}Test Description: Send 5 rapid UDP requests and check responses${NC}"

# Create a temporary script for multiple requests
TEST_DIR=$(pwd)
cat > ./multiple_udp_requests.sh << EOF
#!/bin/bash
SUCCESS=0
TOTAL=5

cd "${TEST_DIR}"
for i in \$(seq 1 \$TOTAL); do
    echo "Request \$i of \$TOTAL"
    echo "%H:%M:%S" | ./bin/test_client_udp > ./udp_response_\$i
    if [ \$? -eq 0 ]; then
        echo "✓ Request \$i successful"
        SUCCESS=\$((SUCCESS + 1))
    else
        echo "✗ Request \$i failed"
    fi
    # Small delay between requests
    sleep 0.1
done

echo "\$SUCCESS of \$TOTAL requests successful"
if [ \$SUCCESS -eq \$TOTAL ]; then
    exit 0
else
    exit 1
fi
EOF

chmod +x ./multiple_udp_requests.sh
./multiple_udp_requests.sh
test_result=$?

# Clean up temporary files
rm -f ./multiple_udp_requests.sh
rm -f ./udp_response_*

# Check status
if [ $test_result -eq 0 ]; then
    echo -e "${GREEN}✓ Multiple requests test passed${NC}"
    TESTS_PASSED=$((TESTS_PASSED + 1))
else
    echo -e "${RED}✗ Multiple requests test failed${NC}"
    TESTS_FAILED=$((TESTS_FAILED + 1))
fi

echo -e "-------------------------------------------\n"

# Calculate elapsed time
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

# Print summary
echo -e "${YELLOW}UDP-Specific Tests Summary:${NC}"
echo -e "Tests run:    $TESTS_RUN"
echo -e "Tests passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests failed: ${RED}$TESTS_FAILED${NC}"
echo -e "Time elapsed: ${ELAPSED} seconds"

# Save summary to temp file for master script - use current directory
echo "TESTS_RUN=$TESTS_RUN" > scripts/udp_specific_summary.txt
echo "TESTS_PASSED=$TESTS_PASSED" >> scripts/udp_specific_summary.txt
echo "TESTS_FAILED=$TESTS_FAILED" >> scripts/udp_specific_summary.txt

# Set exit code based on test results
if [ $TESTS_FAILED -eq 0 ]; then
    exit 0
else
    exit 1
fi