#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// Default configuration
constexpr size_t kDefaultAllocSize = 64;
constexpr size_t kDefaultBatchSize = 1000;
constexpr size_t kDefaultNumBatches = 10000;
constexpr size_t kDefaultNumPairs = 1;

// Thread-safe queue for passing pointers from producer to consumer
class PointerQueue {
 public:
  void Push(const std::vector<void*>& batch) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.push(batch);
    cv_.notify_one();
  }

  bool Pop(std::vector<void*>& batch) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty() || done_; });

    if (queue_.empty()) {
      return false;  // Producer is done and queue is empty
    }

    batch = queue_.front();
    queue_.pop();
    return true;
  }

  void SetDone() {
    std::unique_lock<std::mutex> lock(mutex_);
    done_ = true;
    cv_.notify_all();
  }

 private:
  std::queue<std::vector<void*>> queue_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool done_ = false;
};

void ProducerThread(PointerQueue* queue, size_t num_batches, size_t batch_size,
                    size_t alloc_size) {
  for (size_t i = 0; i < num_batches; i++) {
    std::vector<void*> batch;
    batch.reserve(batch_size);

    // Allocate a batch of memory
    for (size_t j = 0; j < batch_size; j++) {
      void* ptr = malloc(alloc_size);
      // Write to prevent optimization
      *static_cast<char*>(ptr) = 'x';
      if (*reinterpret_cast<unsigned const*>(ptr) == 0xdeadbeef) {
        std::cout << "impossible" << std::endl;
      }
      batch.push_back(ptr);
    }

    // Push batch to queue for consumer
    queue->Push(batch);
  }

  // Signal that producer is done
  queue->SetDone();
}

void ConsumerThread(PointerQueue* queue) {
  std::vector<void*> batch;

  while (queue->Pop(batch)) {
    // Free all items in the batch
    for (void* ptr : batch) {
      // Read from it to prevent optimization
      if (*reinterpret_cast<unsigned const*>(ptr) == 0xdeadbeef) {
        std::cout << "impossible" << std::endl;
      }
      free(ptr);
    }
  }
}

// Same-thread mode: one thread does both allocation and deallocation
void SameThreadWorker(size_t num_batches, size_t batch_size,
                      size_t alloc_size) {
  for (size_t i = 0; i < num_batches; i++) {
    std::vector<void*> batch;
    batch.reserve(batch_size);

    // Allocate a batch of memory
    for (size_t j = 0; j < batch_size; j++) {
      void* ptr = malloc(alloc_size);
      // Write to prevent optimization
      *static_cast<char*>(ptr) = 'x';
      if (*reinterpret_cast<unsigned const*>(ptr) == 0xdeadbeef) {
        std::cout << "impossible" << std::endl;
      }
      batch.push_back(ptr);
    }

    // Free all items in the batch
    for (void* ptr : batch) {
      // Read from it to prevent optimization
      if (*reinterpret_cast<unsigned const*>(ptr) == 0xdeadbeef) {
        std::cout << "impossible" << std::endl;
      }
      free(ptr);
    }
  }
}

void PrintUsage(const char* prog_name) {
  std::cout << "Usage: " << prog_name
            << " [--same-thread] [--pairs N] [--alloc-size N] "
            << "[--batch-size N] [--num-batches N]" << std::endl;
  std::cout << "  --same-thread    : Single thread mode (baseline)" << std::endl;
  std::cout << "  --pairs N        : Number of producer-consumer pairs (default: "
            << kDefaultNumPairs << ")" << std::endl;
  std::cout << "  --alloc-size N   : Allocation size in bytes (default: "
            << kDefaultAllocSize << ")" << std::endl;
  std::cout << "  --batch-size N   : Batch size (default: "
            << kDefaultBatchSize << ")" << std::endl;
  std::cout << "  --num-batches N  : Number of batches (default: "
            << kDefaultNumBatches << ")" << std::endl;
}

int main(int argc, char* argv[]) {
  // Parse command-line arguments
  bool same_thread = false;
  size_t num_pairs = kDefaultNumPairs;
  size_t alloc_size = kDefaultAllocSize;
  size_t batch_size = kDefaultBatchSize;
  size_t num_batches = kDefaultNumBatches;

  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "--same-thread") {
      same_thread = true;
    } else if (arg == "--pairs" && i + 1 < argc) {
      num_pairs = std::atoi(argv[++i]);
    } else if (arg == "--alloc-size" && i + 1 < argc) {
      alloc_size = std::atoi(argv[++i]);
    } else if (arg == "--batch-size" && i + 1 < argc) {
      batch_size = std::atoi(argv[++i]);
    } else if (arg == "--num-batches" && i + 1 < argc) {
      num_batches = std::atoi(argv[++i]);
    } else if (arg == "--help" || arg == "-h") {
      PrintUsage(argv[0]);
      return 0;
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      PrintUsage(argv[0]);
      return 1;
    }
  }

  // Print configuration
  std::cout << "=== Producer-Consumer Allocator Demo ===" << std::endl;
  if (same_thread) {
    std::cout << "Mode: Same-thread (baseline)" << std::endl;
    std::cout << "Number of threads: " << num_pairs << std::endl;
  } else {
    std::cout << "Mode: Cross-thread (producer-consumer)" << std::endl;
    std::cout << "Number of pairs: " << num_pairs << std::endl;
  }
  std::cout << "Allocation size: " << alloc_size << " bytes" << std::endl;
  std::cout << "Batch size: " << batch_size << " allocations" << std::endl;
  std::cout << "Number of batches: " << num_batches << std::endl;
  std::cout << "Total allocations: " << (batch_size * num_batches * num_pairs)
            << std::endl;
  std::cout << std::endl;

  auto start = std::chrono::high_resolution_clock::now();

  if (same_thread) {
    // Same-thread mode: launch N independent threads
    std::vector<std::thread> workers;
    for (size_t i = 0; i < num_pairs; i++) {
      workers.emplace_back(SameThreadWorker, num_batches, batch_size,
                           alloc_size);
    }
    for (auto& t : workers) {
      t.join();
    }
  } else {
    // Cross-thread mode: launch N producer-consumer pairs
    std::vector<PointerQueue> queues(num_pairs);
    std::vector<std::thread> threads;

    // Launch all producers and consumers
    for (size_t i = 0; i < num_pairs; i++) {
      threads.emplace_back(ProducerThread, &queues[i], num_batches, batch_size,
                           alloc_size);
      threads.emplace_back(ConsumerThread, &queues[i]);
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
      t.join();
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> elapsed = end - start;

  // Display results
  double total_ops = num_batches * batch_size * num_pairs;
  double ops_per_second = total_ops / elapsed.count();

  std::cout << "Results:" << std::endl;
  std::cout << "  Time elapsed: " << std::fixed << std::setprecision(3)
            << elapsed.count() << " seconds" << std::endl;
  std::cout << "  Operations/sec: " << std::fixed << std::setprecision(0)
            << ops_per_second << std::endl;
  std::cout << "  Time per operation: " << std::fixed << std::setprecision(3)
            << (elapsed.count() / total_ops * 1e9) << " ns" << std::endl;

  return 0;
}
