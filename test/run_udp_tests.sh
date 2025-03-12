#!/bin/bash

# Text colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting UDP-specific tests for timed server${NC}"
echo 

# Test each file
for test_file in udp_tests/*.txt; do
    test_name=$(basename "$test_file" .txt)
    echo -e "${YELLOW}Running UDP test: $test_name${NC}"
    
    # Show format string
    if [ -s "$test_file" ]; then
        echo -n "Format string: "
        cat "$test_file"
    else
        echo "Format string: (empty)"
    fi
    
    # Run the test
    echo "Response:"
    ./test_client_udp < "$test_file"
    
    # Check status
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ Test passed${NC}"
    else
        echo -e "${RED}✗ Test failed${NC}"
    fi
    
    echo -e "-------------------------------------------\n"
done

echo -e "${YELLOW}All UDP-specific tests completed${NC}"
