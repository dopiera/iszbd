#include "journal.h"

#include <fcntl.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>

// STUB IMPLEMENTATION - Students must improve this!
// This implementation is NOT resilient to hardware failures.
// It will FAIL all tests.

Journal::Journal(const std::string& path) : path_(path) {
  fd_ = open(path.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd_ < 0) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to open journal");
  }
}

Journal::~Journal() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

void Journal::AppendRecord(const std::string& data) {
  uint32_t size = data.size();
  (void)write(fd_, &size, sizeof(size));
  (void)write(fd_, data.data(), size);
  fsync(fd_);
}

std::vector<std::string> Journal::ReadRecords() {
  std::vector<std::string> records;
  lseek(fd_, 0, SEEK_SET);

  while (true) {
    uint32_t size;
    ssize_t n = read(fd_, &size, sizeof(size));
    if (n == 0) break;
    if (n != sizeof(size)) break;

    std::string data(size, '\0');
    n = read(fd_, data.data(), size);
    if (n != static_cast<ssize_t>(size)) break;

    records.push_back(std::move(data));
  }

  return records;
}
