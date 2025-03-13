#!/bin/bash

# Master test script for timed server
# This script runs all test categories and reports summary results

# Text colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Global counters
TOTAL_TESTS=0
TOTAL_PASSED=0
TOTAL_FAILED=0

# Start time
START_TIME=$(date +%s)

# Function to print section header
print_header() {
    echo -e "\n${BLUE}========================================================${NC}"
    echo -e "${BLUE}   $1${NC}"
    echo -e "${BLUE}========================================================${NC}"
}

# Function to run a test script and update counters
run_test_script() {
    local script=$1
    local description=$2
    local summary_file=$3
    
    print_header "$description"
    
    # Run the test script
    ./$script
    
    # Create summary files directory if needed
    if [ ! -d "$(dirname "$summary_file")" ]; then
        mkdir -p "$(dirname "$summary_file")"
    fi
    
    # Wait briefly to ensure the summary file exists
    sleep 1
    
    # Check if summary file exists
    if [ -f "$summary_file" ]; then
        # Get the result counters from the script output
        local script_total=$(grep -oP "TESTS_RUN=\K\d+" "$summary_file")
        local script_passed=$(grep -oP "TESTS_PASSED=\K\d+" "$summary_file")
        local script_failed=$(grep -oP "TESTS_FAILED=\K\d+" "$summary_file")
        
        # Update the global counters
        TOTAL_TESTS=$((TOTAL_TESTS + script_total))
        TOTAL_PASSED=$((TOTAL_PASSED + script_passed))
        TOTAL_FAILED=$((TOTAL_FAILED + script_failed))
    else
        echo -e "${RED}Warning: Summary file $summary_file not found.${NC}"
        echo -e "${RED}Test statistics may not be accurate.${NC}"
    fi
}

# Make sure we're in the test scripts directory
cd "$(dirname "$0")" || { echo "Failed to change to script directory"; exit 1; }

# Check if timed server is running (commenting out since we start it through make)
# if ! pgrep -f "timed" > /dev/null; then
#     echo -e "${RED}ERROR: timed server does not appear to be running${NC}"
#     echo "Please start the server before running tests."
#     exit 1
# fi

# Run each test category
run_test_script "run_tcp_tests.sh" "TCP PROTOCOL TESTS" "tcp_summary.txt"
run_test_script "run_udp_tests.sh" "UDP PROTOCOL TESTS (COMMON FORMATS)" "udp_summary.txt"
run_test_script "run_udp_specific_tests.sh" "UDP-SPECIFIC TESTS" "udp_specific_summary.txt"

# Calculate elapsed time
END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

# Print master summary
print_header "TEST SUMMARY"
echo -e "${CYAN}Test Categories Run:     ${NC}3"
echo -e "${CYAN}Total Tests Run:         ${NC}$TOTAL_TESTS"
echo -e "${CYAN}Total Tests Passed:      ${NC}${GREEN}$TOTAL_PASSED${NC}"
echo -e "${CYAN}Total Tests Failed:      ${NC}${RED}$TOTAL_FAILED${NC}"
echo -e "${CYAN}Total Time Elapsed:      ${NC}${ELAPSED} seconds"
echo

# Determine overall success/failure
if [ $TOTAL_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed successfully!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Please review the test output.${NC}"
    exit 1
fi