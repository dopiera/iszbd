CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -O2
LDFLAGS = -lboost_system -pthread

TARGETS = tcp_receiver tcp_sender

all: $(TARGETS)

tcp_receiver: tcp_receiver.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

tcp_sender: tcp_sender.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cc message.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGETS) *.o

.PHONY: all clean
