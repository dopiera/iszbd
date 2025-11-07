#include <pthread.h>
#include <sched.h>

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

void PinToCore(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) !=
      0) {
    std::cerr << "Warning: Failed to set CPU affinity to core " << core_id
              << std::endl;
  }
}

struct ExperimentResult {
  uint64_t total_count;
  uint64_t random_sum;
};

ExperimentResult RunExperiment(size_t num_threads,
                                std::chrono::duration<double> duration) {
  // Pin this process to core 0
  PinToCore(0);

  // Calculate deadline
  auto deadline = std::chrono::steady_clock::now() + duration;

  // Prepare threads
  std::vector<std::thread> threads;
  std::vector<uint64_t> counts(num_threads);
  std::vector<uint64_t> last_randoms(num_threads);

  for (size_t i = 0; i < num_threads; i++) {
    threads.emplace_back([&counts, &last_randoms, i, deadline]() {
      std::mt19937_64 rng(12345 + i);
      uint64_t count = 0;
      uint64_t last_random = 0;

      while (std::chrono::steady_clock::now() < deadline) {
        last_random = rng();
        count++;
        std::this_thread::yield();
      }

      counts[i] = count;
      last_randoms[i] = last_random;
    });
  }

  // Wait for all threads to complete
  for (auto& thread : threads) {
    thread.join();
  }

  // Sum up counts and random numbers
  uint64_t total_count = 0;
  uint64_t random_sum = 0;
  for (size_t i = 0; i < num_threads; i++) {
    total_count += counts[i];
    random_sum += last_randoms[i];
  }

  return {total_count, random_sum};
}

int main() {
  using namespace std::chrono_literals;

  constexpr auto kDuration = 10s;
  std::vector<size_t> thread_counts = {1, 2, 5, 10, 20, 50, 100, 200, 500};

  std::cout << "=== Context Switch Overhead Measurement ===" << std::endl;
  std::cout << "Method: Throughput-based with std::this_thread::yield()"
            << std::endl;
  std::cout << "Duration: " << kDuration.count() << " seconds per test"
            << std::endl;
  std::cout << "CPU affinity: Core 0 (all threads)" << std::endl;
  std::cout << std::endl;

  std::cout << std::setw(12) << "Threads" << std::setw(18) << "Total Yields"
            << std::setw(18) << "Yields/sec" << std::setw(20)
            << "Est. Switch (µs)" << std::endl;
  std::cout << std::string(68, '-') << std::endl;

  double baseline_rate = 0.0;
  uint64_t total_random_sum = 0;

  for (size_t num_threads : thread_counts) {
    std::cout << "Running with " << num_threads << " thread(s)... "
              << std::flush;

    auto result = RunExperiment(num_threads, kDuration);
    double yields_per_sec = result.total_count / kDuration.count();
    total_random_sum += result.random_sum;

    std::cout << "\r";  // Clear the "Running..." message

    std::cout << std::setw(12) << num_threads << std::setw(18)
              << result.total_count << std::setw(18) << std::fixed
              << std::setprecision(0) << yields_per_sec;

    if (num_threads == 1) {
      baseline_rate = yields_per_sec;
      std::cout << std::setw(20) << "baseline";
    } else {
      // Calculate context switch overhead
      // Expected: baseline_rate * duration * num_threads
      // Actual: total_count
      // Time lost: (expected - actual) / baseline_rate
      // Context switch time: time_lost / total_count
      double expected_count =
          baseline_rate * kDuration.count() * num_threads;
      double time_lost_sec =
          (expected_count - result.total_count) / baseline_rate;
      double context_switch_time_us =
          (time_lost_sec / result.total_count) * 1e6;  // microseconds

      std::cout << std::setw(20) << std::fixed << std::setprecision(2)
                << context_switch_time_us;
    }

    std::cout << std::endl;
  }

  std::cout << std::endl;
  std::cout << "Note: Estimated switch time includes scheduler overhead"
            << std::endl;
  std::cout << "      and increases with number of runnable threads."
            << std::endl;
  std::cout << std::endl;
  std::cout << "Random number checksum: " << total_random_sum
            << " (prevents optimization)" << std::endl;

  return 0;
}
