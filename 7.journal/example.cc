#include "journal.h"

#include <iostream>
#include <string>

int main() {
  const char* path = "/tmp/example_journal.dat";

  try {
    // Write some records
    {
      std::cout << "Writing records to journal..." << std::endl;
      Journal journal(path);

      journal.AppendRecord("First record");
      journal.AppendRecord("Second record with more data");
      journal.AppendRecord("Third record: 12345");
      journal.AppendRecord("Fourth record: Hello, World!");

      std::cout << "Successfully wrote 4 records" << std::endl;
    }

    // Read them back
    {
      std::cout << "Reading records from journal..." << std::endl;
      Journal journal(path);

      auto records = journal.ReadRecords();

      std::cout << "Read " << records.size() << " records:" << std::endl;
      for (size_t i = 0; i < records.size(); ++i) {
        std::cout << "  [" << i << "] " << records[i] << std::endl;
      }
    }

  } catch (const std::system_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
