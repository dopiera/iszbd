#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// Number of allocation/deallocation cycles per thread
constexpr size_t kIterations = 100000000;

// Size of allocations (should be within tcache range)
constexpr size_t kAllocSize = 64;

// Run benchmark with specified number of threads
double RunBenchmark(int num_threads, size_t iterations_per_thread) {
  std::vector<std::thread> threads;

  auto start = std::chrono::high_resolution_clock::now();

  // Launch threads with lambda worker
  for (int i = 0; i < num_threads; i++) {
    threads.emplace_back([iterations_per_thread]() {
      for (size_t j = 0; j < iterations_per_thread; j++) {
        // Allocate memory
        void* ptr = malloc(kAllocSize);

        // Write to it to ensure it's actually allocated
        *static_cast<char*>(ptr) = 'x';
        if (*reinterpret_cast<unsigned const *>(ptr) == 0xdeadbeef) {
          std::cout << "some steaks are coming" << std::endl;
        }

        // Free immediately - this pattern benefits greatly from tcache
        free(ptr);
      }
    });
  }

  // Wait for all threads to complete
  for (auto& t : threads) {
    t.join();
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  return elapsed.count();
}

int main(int argc, char* argv[]) {
  int num_threads = std::thread::hardware_concurrency();

  if (argc > 1) {
    num_threads = std::atoi(argv[1]);
  }

  std::cout << "=== Thread Cache (tcache) Performance Demo ===" << std::endl;
  std::cout << "Hardware concurrency: " << std::thread::hardware_concurrency()
            << std::endl;
  std::cout << "Using " << num_threads << " threads" << std::endl;
  std::cout << "Iterations per thread: " << kIterations << std::endl;
  std::cout << "Allocation size: " << kAllocSize << " bytes" << std::endl;
  std::cout << "Total allocations: " << (num_threads * kIterations)
            << std::endl;
  std::cout << std::endl;

  // Check if tcache tunables are set
  const char* tunables = std::getenv("GLIBC_TUNABLES");
  if (tunables) {
    std::cout << "GLIBC_TUNABLES: " << tunables << std::endl;
  } else {
    std::cout << "GLIBC_TUNABLES: (not set - using defaults)" << std::endl;
  }
  std::cout << std::endl;

  // Run the benchmark
  std::cout << "Running benchmark..." << std::flush;
  double elapsed = RunBenchmark(num_threads, kIterations);
  std::cout << " Done!" << std::endl;
  std::cout << std::endl;

  // Calculate and display results
  double total_ops = num_threads * kIterations;
  double ops_per_second = total_ops / elapsed;

  std::cout << "Results:" << std::endl;
  std::cout << "  Time elapsed: " << std::fixed << std::setprecision(3)
            << elapsed << " seconds" << std::endl;
  std::cout << "  Operations/sec: " << std::fixed << std::setprecision(0)
            << ops_per_second << std::endl;
  std::cout << "  Time per operation: " << std::fixed << std::setprecision(3)
            << (elapsed / total_ops * 1e9) << " ns" << std::endl;

  return 0;
}
