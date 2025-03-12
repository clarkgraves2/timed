#!/bin/bash

# Text colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================================${NC}"
echo -e "${BLUE}   TESTING TCP FUNCTIONALITY${NC}"
echo -e "${BLUE}========================================================${NC}"
./run_all_tests.sh

echo -e "\n\n"
echo -e "${BLUE}========================================================${NC}"
echo -e "${BLUE}   TESTING UDP FUNCTIONALITY${NC}"
echo -e "${BLUE}========================================================${NC}"
./run_udp_tests.sh

echo -e "\n\n"
echo -e "${BLUE}========================================================${NC}"
echo -e "${BLUE}   ALL TESTS COMPLETED${NC}"
echo -e "${BLUE}========================================================${NC}"
