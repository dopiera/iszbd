#!/bin/bash

set -e

# Compile if needed
make producer_consumer_demo

echo "============================================"
echo "   Testing glibc tcache tuning impact     "
echo "============================================"
echo ""

# Function to run test with specific glibc tunables
run_with_tunables() {
    local desc="$1"
    local tunables="$2"
    shift 2
    local args="$@"

    echo "=== $desc ==="
    if [ -n "$tunables" ]; then
        echo "GLIBC_TUNABLES=$tunables"
    else
        echo "GLIBC_TUNABLES=(default)"
    fi
    echo ""

    if [ -n "$tunables" ]; then
        GLIBC_TUNABLES="$tunables" ./producer_consumer_demo $args | grep -E "(Mode:|Allocation size:|Batch size:|Time elapsed:|Operations/sec:)"
    else
        ./producer_consumer_demo $args | grep -E "(Mode:|Allocation size:|Batch size:|Time elapsed:|Operations/sec:)"
    fi
    echo ""
    echo "-----------------------------------------------------------"
    echo ""
}

echo "Experiment: Default glibc (tcache_count=7, default)"
echo ""

# Test 1: Cross-thread with default settings
run_with_tunables "Default tcache (7 items per bin)" "" "--pairs 1"

# Test 2: Enlarged tcache
run_with_tunables "Enlarged tcache (127 items per bin)" "glibc.malloc.tcache_count=127" "--pairs 1"

# Test 3: Very large tcache
run_with_tunables "Very large tcache (1000 items per bin)" "glibc.malloc.tcache_count=1000" "--pairs 1"

# Test 4: Tcache disabled for comparison
run_with_tunables "Tcache disabled (count=0)" "glibc.malloc.tcache_count=0" "--pairs 1"

echo ""
echo "Now test with 1KB allocations (where gap was 14x):"
echo ""

run_with_tunables "1KB - Default tcache" "" "--pairs 1 --alloc-size 1024"
run_with_tunables "1KB - Enlarged tcache (127)" "glibc.malloc.tcache_count=127" "--pairs 1 --alloc-size 1024"
run_with_tunables "1KB - Very large tcache (1000)" "glibc.malloc.tcache_count=1000" "--pairs 1 --alloc-size 1024"

echo ""
echo "For comparison, here's tcmalloc:"
echo ""
echo "=== tcmalloc - 64B ==="
LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 ./producer_consumer_demo --pairs 1 | grep -E "(Time elapsed:|Operations/sec:)"
echo ""
echo "=== tcmalloc - 1KB ==="
LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libtcmalloc_minimal.so.4 ./producer_consumer_demo --pairs 1 --alloc-size 1024 | grep -E "(Time elapsed:|Operations/sec:)"
echo ""

echo "============================================"
echo "    Analysis completed!                    "
echo "============================================"
