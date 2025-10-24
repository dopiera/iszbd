# Makefile for CPU and Memory Demonstration Programs

CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pthread
LDFLAGS = -pthread

# Target executables
TARGETS = cache_size_demo atomic_contention_demo

# Default target
all: $(TARGETS)

# Individual targets
cache_size_demo: cache_size_demo.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

atomic_contention_demo: atomic_contention_demo.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)


# Clean build artifacts
clean:
	rm -f $(TARGETS) *.o

# Phony targets
.PHONY: all clean
