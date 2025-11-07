#!/bin/bash

# Compile the demo
echo "Compiling tcache_demo.cpp..."
g++ -O2 -std=c++17 -pthread tcache_demo.cpp -o tcache_demo

if [ $? -ne 0 ]; then
    echo "Compilation failed!"
    exit 1
fi

echo ""
echo "=========================================="
echo "Running WITH tcache (default settings):"
echo "=========================================="
./tcache_demo

echo ""
echo ""
echo "=========================================="
echo "Running WITHOUT tcache (disabled):"
echo "=========================================="
GLIBC_TUNABLES=glibc.malloc.tcache_count=0 ./tcache_demo

echo ""
echo ""
echo "=========================================="
echo "Running with SMALL tcache (max 32 bytes):"
echo "=========================================="
GLIBC_TUNABLES=glibc.malloc.tcache_max=32 ./tcache_demo

echo ""
echo "Comparison complete! Notice the dramatic performance difference."
echo "The tcache eliminates contention on the global heap lock."
