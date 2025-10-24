// Cache Size Visualization Demo
//
// This program demonstrates the impact of cache size on performance through
// random jumping in linked lists of different sizes.
//
// When data fits in cache (L1/L2/L3), access is fast.
// When it exceeds cache, each access requires DRAM fetch.

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// Node structure with padding to fill cache line
struct Node {
  Node* next;
  char padding[56];  // Together with next* gives 64 bytes (typical cache line)
};

class CacheSizeDemo {
 public:
  explicit CacheSizeDemo(size_t num_nodes) : nodes_(num_nodes) {
    // Create random cycle through all nodes
    std::vector<size_t> indices(num_nodes);
    for (size_t i = 0; i < num_nodes; ++i) {
      indices[i] = i;
    }

    // Shuffle for random access pattern
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);

    // Connect nodes in a cycle
    for (size_t i = 0; i < num_nodes; ++i) {
      size_t next_idx = (i + 1) % num_nodes;
      nodes_[indices[i]].next = &nodes_[indices[next_idx]];
    }
  }

  // Run test - traverse list N times
  double RunTest(size_t iterations) {
    Node* current = &nodes_[0];

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < iterations; ++i) {
      current = current->next;
    }

    auto end = std::chrono::high_resolution_clock::now();
    if (current == nullptr) {
      std::cout << "whatever, just make the compiler not optimize this out"
                << std::endl;
    }

    std::chrono::duration<double, std::nano> elapsed = end - start;
    return elapsed.count() / iterations;  // Nanoseconds per access
  }

 private:
  std::vector<Node> nodes_;
};


int main() {

  return 0;
}
