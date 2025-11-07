#!/bin/bash

set -e

# Compile the program
echo "Compiling producer_consumer_demo..."
make producer_consumer_demo
echo ""

# Function to run a test with both allocators
run_test() {
    local test_name="$1"
    shift
    local args="$@"

    echo "=== $test_name ==="
    echo ""

    echo "glibc malloc:"
    ./producer_consumer_demo $args | grep -E "(Mode:|Time elapsed:|Operations/sec:)"
    echo ""

    echo "tcmalloc:"
    LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 ./producer_consumer_demo $args | grep -E "(Mode:|Time elapsed:|Operations/sec:)"
    echo ""
    echo "-----------------------------------------------------------"
    echo ""
}

echo "============================================"
echo "    Quick Allocator Performance Tests      "
echo "============================================"
echo ""

# Quick subset of key experiments
run_test "Experiment 1: Same-thread baseline" "--same-thread --pairs 1"
run_test "Experiment 1: Cross-thread (1 pair)" "--pairs 1"
run_test "Experiment 2: Cross-thread (4 pairs)" "--pairs 4"
run_test "Experiment 3: 16 bytes" "--alloc-size 16"
run_test "Experiment 3: 1 KB" "--alloc-size 1024"
run_test "Experiment 4: Batch 100" "--batch-size 100 --num-batches 10000"
run_test "Experiment 4: Batch 10000" "--batch-size 10000 --num-batches 1000"

echo "All quick tests completed!"
