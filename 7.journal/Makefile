CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -O2
LDFLAGS = -ldl

all: journal_test example

# Fault injection library
fault_injection.o: fault_injection.cc fault_injection.h
	$(CXX) $(CXXFLAGS) -c fault_injection.cc -o fault_injection.o

# Student's journal implementation
journal.o: journal.cc journal.h
	$(CXX) $(CXXFLAGS) -c journal.cc -o journal.o

# Stub implementation (for demonstration)
journal_stub.o: journal_stub.cc journal.h
	$(CXX) $(CXXFLAGS) -c journal_stub.cc -o journal_stub.o

# Test program with fault injection
journal_test: journal_test.cc fault_injection.o journal.o
	$(CXX) $(CXXFLAGS) journal_test.cc fault_injection.o journal.o -o journal_test $(LDFLAGS)

# Test program with stub (will fail tests)
journal_test_stub: journal_test.cc fault_injection.o journal_stub.o
	$(CXX) $(CXXFLAGS) journal_test.cc fault_injection.o journal_stub.o -o journal_test_stub $(LDFLAGS)

# Example usage program
example: example.cc journal.o
	$(CXX) $(CXXFLAGS) example.cc journal.o -o example

# Example with stub
example_stub: example.cc journal_stub.o
	$(CXX) $(CXXFLAGS) example.cc journal_stub.o -o example_stub

clean:
	rm -f *.o journal_test journal_test_stub example example_stub /tmp/test_journal.dat /tmp/example_journal.dat

test: journal_test
	./journal_test

.PHONY: all clean test
