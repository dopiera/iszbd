// Atomic Contention Demo
//
// This program demonstrates how cache coherency protocols impact performance
// when multiple threads compete to update a shared variable.
// Compares atomic operations vs mutex-protected operations.

#include <atomic>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <sys/resource.h>

std::atomic<int64_t> global_counter{0};
std::mutex counter_mutex;
int64_t mutex_counter = 0;

const int64_t TARGET = 400000000LL;

class BenchmarkMeasurement {
 public:
  static void PrintHeader() {
    std::cout << std::setw(12) << "Type" << std::setw(10) << "Threads"
              << std::setw(15) << "Time (s)" << std::setw(12) << "CPU %"
              << std::endl;
    std::cout << std::string(49, '-') << std::endl;
  }

  void Start() {
    getrusage(RUSAGE_SELF, &usage_start_);
    start_ = std::chrono::high_resolution_clock::now();
  }

  void Stop() {
    end_ = std::chrono::high_resolution_clock::now();
    getrusage(RUSAGE_SELF, &usage_end_);
  }

  void Print(const std::string& type, int num_threads) const {
    std::chrono::duration<double> elapsed = end_ - start_;

    double cpu_time = (usage_end_.ru_utime.tv_sec - usage_start_.ru_utime.tv_sec) +
                      (usage_end_.ru_utime.tv_usec - usage_start_.ru_utime.tv_usec) / 1e6 +
                      (usage_end_.ru_stime.tv_sec - usage_start_.ru_stime.tv_sec) +
                      (usage_end_.ru_stime.tv_usec - usage_start_.ru_stime.tv_usec) / 1e6;

    double cpu_percent = (cpu_time / elapsed.count()) * 100.0;

    std::cout << std::setw(12) << type << std::setw(10) << num_threads
              << std::setw(15) << std::fixed << std::setprecision(2)
              << elapsed.count() << std::setw(11) << std::setprecision(1)
              << cpu_percent << "%" << std::endl;
  }

 private:
  std::chrono::high_resolution_clock::time_point start_, end_;
  struct rusage usage_start_, usage_end_;
};

BenchmarkMeasurement RunBenchmark(int num_threads) {
  global_counter.store(0);
  std::vector<std::thread> threads;

  BenchmarkMeasurement measure;
  measure.Start();

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([]() {
      while (true) {
        int64_t current = global_counter.fetch_add(1, std::memory_order_relaxed);
        if (current >= TARGET) {
          break;
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  measure.Stop();
  return measure;
}

BenchmarkMeasurement RunBenchmarkMutex(int num_threads) {
  mutex_counter = 0;
  std::vector<std::thread> threads;

  BenchmarkMeasurement measure;
  measure.Start();

  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([]() {
      while (true) {
        int64_t current;
        {
          std::lock_guard<std::mutex> lock(counter_mutex);
          current = mutex_counter++;
        }
        if (current >= TARGET) {
          break;
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  measure.Stop();
  return measure;
}

int main() {
  std::vector<int> thread_counts = {1, 2, 4};

  BenchmarkMeasurement::PrintHeader();

  for (int num_threads : thread_counts) {
    RunBenchmark(num_threads).Print("Atomic", num_threads);
    RunBenchmarkMutex(num_threads).Print("Mutex", num_threads);
  }

  return 0;
}
