#include "journal.h"

#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "fault_injection.h"

constexpr int kNumRecords = 10;
constexpr const char* kTestJournalPath = "/tmp/test_journal.dat";

static int write_counter = 0;
static int target_write = -1;
static int read_counter = 0;
static int target_read = -1;

std::string MakeTestRecord(int id) {
  return "Record number " + std::to_string(id) + " with some test data";
}

void WriteRecords(Journal& journal, int count) {
  for (int i = 0; i < count; ++i) {
    journal.AppendRecord(MakeTestRecord(i));
  }
}

bool VerifyRecords(const std::vector<std::string>& records, int expected_count) {
  if (records.size() != static_cast<size_t>(expected_count)) {
    std::cerr << "Expected " << expected_count << " records, got "
              << records.size() << std::endl;
    return false;
  }

  for (size_t i = 0; i < records.size(); ++i) {
    std::string expected = MakeTestRecord(i);
    if (records[i] != expected) {
      std::cerr << "Record " << i << " mismatch. Expected: \"" << expected
                << "\", got: \"" << records[i] << "\"" << std::endl;
      return false;
    }
  }

  return true;
}

// Test 1: Crash after writing half of the 5th record
bool TestCrashDuringWrite() {
  std::cout << "Test 1: Crash after writing half of 5th record... ";
  unlink(kTestJournalPath);
  write_counter = 0;
  target_write = 4;

  pid_t pid = fork();
  assert(pid >= 0);

  if (pid == 0) {
    fault_inject_write = [](int /* fd */, const void* /* buf */, size_t count,
                             ssize_t* ret, int* err) -> bool {
      if (write_counter == target_write) {
        *ret = count / 2;
        *err = 0;
        _exit(42);
      }
      write_counter++;
      return false;
    };

    try {
      Journal journal(kTestJournalPath);
      WriteRecords(journal, kNumRecords);
    } catch (...) {
      _exit(1);
    }
    _exit(0);
  }

  int status;
  waitpid(pid, &status, 0);
  assert(WIFEXITED(status));

  ResetFaultInjection();
  try {
    Journal journal(kTestJournalPath);
    auto records = journal.ReadRecords();

    if (!VerifyRecords(records, 4)) {
      std::cerr << "FAIL: Expected exactly 4 valid records!" << std::endl;
      return false;
    }

    std::cout << "PASS (recovered 4 valid records)" << std::endl;
    return true;
  } catch (const std::system_error& e) {
    std::cerr << "FAIL: Should recover 4 records, not throw: " << e.what()
              << std::endl;
    return false;
  }
}

// Test 2: Corrupt written data (bit flips)
bool TestCorruptedData() {
  std::cout << "Test 2: Corrupted data (bit flips)... ";
  unlink(kTestJournalPath);
  write_counter = 0;
  target_write = 4;

  fault_inject_write = [](int fd, const void* buf, size_t count, ssize_t* ret,
                           int* err) -> bool {
    if (write_counter == target_write) {
      // Corrupt the data by flipping some bits
      std::vector<uint8_t> corrupted(static_cast<const uint8_t*>(buf),
                                      static_cast<const uint8_t*>(buf) + count);
      if (corrupted.size() > 10) {
        corrupted[5] ^= 0xFF;  // Flip all bits in one byte
        corrupted[10] ^= 0x0F; // Flip some bits in another byte
      }

      // Write corrupted data using real write
      write_counter++;
      *ret = write(fd, corrupted.data(), count);
      *err = errno;
      return true;
    }
    write_counter++;
    return false;
  };

  try {
    Journal journal(kTestJournalPath);
    WriteRecords(journal, kNumRecords);
  } catch (...) {
  }

  ResetFaultInjection();

  try {
    Journal journal(kTestJournalPath);
    auto records = journal.ReadRecords();

    if (!VerifyRecords(records, records.size())) {
      std::cerr << "FAIL: Returned corrupted data!" << std::endl;
      return false;
    }

    std::cout << "PASS (repaired, recovered " << records.size()
              << " valid records)" << std::endl;
    return true;
  } catch (const std::system_error& e) {
    std::cout << "PASS (detected unrecoverable corruption: " << e.what() << ")"
              << std::endl;
    return true;
  }
}

// Test 3: Write only half of the 5th record
bool TestPartialWrite() {
  std::cout << "Test 3: Partial write of 5th record... ";
  unlink(kTestJournalPath);
  write_counter = 0;
  target_write = 4;

  fault_inject_write = [](int /* fd */, const void* /* buf */, size_t count,
                           ssize_t* ret, int* err) -> bool {
    if (write_counter == target_write) {
      *ret = count / 2;
      *err = 0;
      write_counter++;
      return true;
    }
    write_counter++;
    return false;
  };

  try {
    Journal journal(kTestJournalPath);
    WriteRecords(journal, kNumRecords);
  } catch (...) {
  }

  ResetFaultInjection();

  try {
    Journal journal(kTestJournalPath);
    auto records = journal.ReadRecords();

    if (!VerifyRecords(records, records.size())) {
      std::cerr << "FAIL: Returned corrupted data!" << std::endl;
      return false;
    }

    std::cout << "PASS (repaired, recovered " << records.size()
              << " valid records)" << std::endl;
    return true;
  } catch (const std::system_error& e) {
    std::cout << "PASS (detected unrecoverable corruption: " << e.what() << ")"
              << std::endl;
    return true;
  }
}

// Test 4: Write to wrong offset
bool TestWrongWriteOffset() {
  std::cout << "Test 4: Write to wrong offset... ";
  unlink(kTestJournalPath);
  write_counter = 0;
  target_write = 4;

  fault_inject_pwrite = [](int /* fd */, const void* /* buf */, size_t /* count */,
                            off_t* offset, ssize_t* /* ret */, int* /* err */) -> bool {
    if (write_counter == target_write) {
      *offset = 0;
      write_counter++;
      return false;
    }
    write_counter++;
    return false;
  };

  try {
    Journal journal(kTestJournalPath);
    WriteRecords(journal, kNumRecords);
  } catch (...) {
  }

  ResetFaultInjection();

  try {
    Journal journal(kTestJournalPath);
    auto records = journal.ReadRecords();

    if (!VerifyRecords(records, records.size())) {
      std::cerr << "FAIL: Returned corrupted data!" << std::endl;
      return false;
    }

    std::cout << "PASS (repaired, recovered " << records.size()
              << " valid records)" << std::endl;
    return true;
  } catch (const std::system_error& e) {
    std::cout << "PASS (detected corruption: " << e.what() << ")" << std::endl;
    return true;
  }
}

// Test 5: Read from wrong offset
bool TestWrongReadOffset() {
  std::cout << "Test 5: Read from wrong offset... ";
  unlink(kTestJournalPath);

  try {
    Journal journal(kTestJournalPath);
    WriteRecords(journal, kNumRecords);
  } catch (const std::system_error& e) {
    std::cerr << "FAIL: Cannot write test data: " << e.what() << std::endl;
    return false;
  }

  read_counter = 0;
  target_read = 2;

  fault_inject_pread = [](int /* fd */, void* /* buf */, size_t /* count */,
                           off_t* offset, ssize_t* /* ret */, int* /* err */) -> bool {
    if (read_counter == target_read) {
      *offset = 0;
      read_counter++;
      return false;
    }
    read_counter++;
    return false;
  };

  try {
    Journal journal(kTestJournalPath);
    auto records = journal.ReadRecords();

    if (!VerifyRecords(records, kNumRecords)) {
      std::cerr << "FAIL: Returned corrupted data!" << std::endl;
      ResetFaultInjection();
      return false;
    }

    std::cout << "PASS (repaired and recovered all records)" << std::endl;
    ResetFaultInjection();
    return true;
  } catch (const std::system_error& e) {
    std::cout << "PASS (detected corruption: " << e.what() << ")" << std::endl;
    ResetFaultInjection();
    return true;
  }
}

// Test 6: Silently ignore one write
bool TestIgnoredWrite() {
  std::cout << "Test 6: Silently ignored write... ";
  unlink(kTestJournalPath);
  write_counter = 0;
  target_write = 4;

  fault_inject_write = [](int /* fd */, const void* /* buf */, size_t count,
                           ssize_t* ret, int* err) -> bool {
    if (write_counter == target_write) {
      *ret = count;
      *err = 0;
      write_counter++;
      return true;
    }
    write_counter++;
    return false;
  };

  try {
    Journal journal(kTestJournalPath);
    WriteRecords(journal, kNumRecords);
  } catch (...) {
  }

  ResetFaultInjection();

  try {
    Journal journal(kTestJournalPath);
    auto records = journal.ReadRecords();

    if (!VerifyRecords(records, records.size())) {
      std::cerr << "FAIL: Returned corrupted data!" << std::endl;
      return false;
    }

    std::cout << "PASS (repaired, recovered " << records.size()
              << " valid records)" << std::endl;
    return true;
  } catch (const std::system_error& e) {
    std::cout << "PASS (detected corruption: " << e.what() << ")" << std::endl;
    return true;
  }
}

int main() {
  std::cout << "Running fault injection tests..." << std::endl;
  std::cout << "Students must implement Journal to pass all tests."
            << std::endl;
  std::cout << std::endl;

  int passed = 0;
  int total = 0;

  if (TestCrashDuringWrite()) passed++;
  total++;

  if (TestCorruptedData()) passed++;
  total++;

  if (TestPartialWrite()) passed++;
  total++;

  if (TestWrongWriteOffset()) passed++;
  total++;

  if (TestWrongReadOffset()) passed++;
  total++;

  if (TestIgnoredWrite()) passed++;
  total++;

  std::cout << std::endl;
  std::cout << "Results: " << passed << "/" << total << " tests passed"
            << std::endl;

  if (passed == total) {
    std::cout << "All tests passed! Journal is resilient to hardware faults."
              << std::endl;
    return 0;
  } else {
    std::cout << "Some tests failed. Keep working on your implementation!"
              << std::endl;
    return 1;
  }
}
