#include "journal.h"

#include <fcntl.h>
#include <unistd.h>

Journal::Journal(const std::string& path) : path_(path) {
  fd_ = open(path.c_str(), O_RDWR | O_CREAT, 0644);
  if (fd_ < 0) {
    throw std::system_error(errno, std::generic_category(),
                            "Failed to open journal");
  }

  // TODO: Implement journal initialization
}

Journal::~Journal() {
  if (fd_ >= 0) {
    close(fd_);
  }
}

void Journal::AppendRecord(const std::string& /* data */) {
  // TODO: Implement this method
  throw std::system_error(ENOSYS, std::generic_category(),
                          "AppendRecord not implemented");
}

std::vector<std::string> Journal::ReadRecords() {
  // TODO: Implement this method
  throw std::system_error(ENOSYS, std::generic_category(),
                          "ReadRecords not implemented");
}
