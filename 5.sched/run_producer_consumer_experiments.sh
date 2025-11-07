#!/bin/bash

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Compile the program
echo -e "${BLUE}Compiling producer_consumer_demo...${NC}"
make producer_consumer_demo
echo ""

# Function to run a test with both allocators
run_test() {
    local test_name="$1"
    shift
    local args="$@"

    echo -e "${YELLOW}=== $test_name ===${NC}"
    echo ""

    echo -e "${GREEN}With glibc malloc:${NC}"
    ./producer_consumer_demo $args
    echo ""

    echo -e "${GREEN}With tcmalloc:${NC}"
    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 ./producer_consumer_demo $args
    echo ""
    echo "-----------------------------------------------------------"
    echo ""
}

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}    Allocator Performance Experiments      ${NC}"
echo -e "${BLUE}============================================${NC}"
echo ""

# Experiment 1: Same-thread baseline
echo -e "${RED}EXPERIMENT 1: Same-thread baseline${NC}"
echo -e "${RED}Hypothesis: Performance gap should shrink when no cross-thread deallocation${NC}"
echo ""
run_test "Same-thread (1 thread)" "--same-thread --pairs 1"

# Experiment 2: Multiple producer-consumer pairs
echo -e "${RED}EXPERIMENT 2: Multiple producer-consumer pairs${NC}"
echo -e "${RED}Hypothesis: glibc performance degrades more with increased contention${NC}"
echo ""
run_test "Cross-thread (1 pair)" "--pairs 1"
run_test "Cross-thread (2 pairs)" "--pairs 2"
run_test "Cross-thread (4 pairs)" "--pairs 4"
run_test "Cross-thread (8 pairs)" "--pairs 8"

# Experiment 3: Allocation size sweep
echo -e "${RED}EXPERIMENT 3: Allocation size sweep${NC}"
echo -e "${RED}Hypothesis: Performance gap varies by size class${NC}"
echo ""
run_test "Alloc size: 16 bytes" "--alloc-size 16"
run_test "Alloc size: 64 bytes" "--alloc-size 64"
run_test "Alloc size: 256 bytes" "--alloc-size 256"
run_test "Alloc size: 1 KB" "--alloc-size 1024"
run_test "Alloc size: 16 KB" "--alloc-size 16384"
run_test "Alloc size: 64 KB" "--alloc-size 65536"

# Experiment 4: Batch size variation
echo -e "${RED}EXPERIMENT 4: Batch size variation${NC}"
echo -e "${RED}Hypothesis: Different queue depths affect allocator strategies${NC}"
echo ""
run_test "Batch size: 10" "--batch-size 10 --num-batches 100000"
run_test "Batch size: 100" "--batch-size 100 --num-batches 10000"
run_test "Batch size: 1000" "--batch-size 1000 --num-batches 10000"
run_test "Batch size: 10000" "--batch-size 10000 --num-batches 1000"

echo -e "${BLUE}============================================${NC}"
echo -e "${BLUE}    All experiments completed!             ${NC}"
echo -e "${BLUE}============================================${NC}"
