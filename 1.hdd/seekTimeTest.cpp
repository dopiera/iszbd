#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <chrono>
#include <random>

int64_t getDeviceSize(int fd) {
    int64_t size;
    if (ioctl(fd, BLKGETSIZE64, &size) == -1) {
        perror("ioctl BLKGETSIZE64");
        return -1;
    }
    return size;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <device_path>" << std::endl;
        return 1;
    }

    const char* device_path = argv[1];
    const size_t block_size = 4096;
    const int num_samples = 1000;

    // Open device with O_DIRECT
    int fd = open(device_path, O_RDONLY | O_DIRECT);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    // Get device size
    int64_t device_size = getDeviceSize(fd);
    if (device_size == -1) {
        close(fd);
        return 1;
    }

    std::cerr << "Device size: " << device_size << " bytes ("
              << device_size / (1024 * 1024 * 1024) << " GB)" << std::endl;

    // Allocate aligned buffer for O_DIRECT
    void* buffer;
    if (posix_memalign(&buffer, block_size, block_size) != 0) {
        perror("posix_memalign");
        close(fd);
        return 1;
    }

    // Initialize random number generator with timestamp
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937_64 rng(seed);

    // Calculate number of possible blocks
    int64_t num_blocks = device_size / block_size;
    std::uniform_int_distribution<int64_t> dist(0, num_blocks - 1);

    int64_t last_offset = 0;
    double total_time = 0.0;

    // Write CSV header to stdout
    std::cout << "distance,time_s" << std::endl;

    std::cerr << "Starting " << num_samples << " random read operations..." << std::endl;

    for (int i = 0; i < num_samples; i++) {
        // Generate random block number
        int64_t block_num = dist(rng);
        int64_t offset = block_num * block_size;

        // Calculate distance from last read (with sign)
        int64_t distance = offset - last_offset;

        // Seek to position
        if (lseek(fd, offset, SEEK_SET) == -1) {
            perror("lseek");
            return 1;
        }

        // Measure read time
        auto start = std::chrono::high_resolution_clock::now();
        ssize_t bytes_read = read(fd, buffer, block_size);
        auto end = std::chrono::high_resolution_clock::now();

        if (bytes_read != block_size) {
            if (bytes_read == -1) {
                perror("read");
            } else {
                std::cerr << "Error: partial read at offset " << offset << std::endl;
            }
            return 1;
        }

        // Calculate elapsed time in seconds
        std::chrono::duration<double> elapsed = end - start;
        double time_s = elapsed.count();

        // Write to stdout immediately
        std::cout << distance << "," << std::fixed << time_s << std::endl;
        last_offset = offset;
        total_time += time_s;

        if ((i + 1) % (num_samples / 10) == 0) {
            std::cerr << "Progress: " << (i + 1) << "/" << num_samples << std::endl;
        }
    }

    double avg_time = total_time / num_samples;
    std::cerr << "Completed " << num_samples << " measurements" << std::endl;
    std::cerr << "Average seek time: " << std::fixed << avg_time << " seconds" << std::endl;

    // Cleanup
    free(buffer);
    close(fd);

    return 0;
}
