CXX = g++
CXXFLAGS = -O2 -std=c++17 -pthread -Wall -Wextra

TARGETS = tcache_demo producer_consumer_demo context_switch_demo

all: $(TARGETS)

tcache_demo: tcache_demo.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

producer_consumer_demo: producer_consumer_demo.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

context_switch_demo: context_switch_demo.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

.PHONY: all clean
