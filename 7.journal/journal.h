#ifndef JOURNAL_H_
#define JOURNAL_H_

#include <string>
#include <system_error>
#include <vector>

class Journal {
 public:
  // Example:
  //   int fd = open(path.c_str(), O_RDWR | O_CREAT, 0644);
  //   if (fd < 0) {
  //     throw std::system_error(errno, std::generic_category(),
  //                             "Failed to open journal");
  //   }
  explicit Journal(const std::string& path);

  ~Journal();

  Journal(const Journal&) = delete;
  Journal& operator=(const Journal&) = delete;

  // Appends a record to the journal
  // Throws std::system_error on error
  void AppendRecord(const std::string& data);

  // Reads all records from the journal
  // Returns a vector with data of each record
  // If unrecoverable corruption is detected, throws std::system_error
  // If recoverable corruption is detected, should repair/skip it
  std::vector<std::string> ReadRecords();

 private:
  int fd_;
  std::string path_;
};

#endif  // JOURNAL_H_
