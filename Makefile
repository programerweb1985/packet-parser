CXX      ?= g++
CXXFLAGS ?= -g -O0 -std=c++17 -Wall -Wextra

# Debug build (server demo)
all: parser

parser: parser.cpp main.cpp parser.h
	$(CXX) $(CXXFLAGS) parser.cpp main.cpp -o parser

# Test build (memory-safety verification)
asan: tests.cpp parser.cpp parser.h
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
	  parser.cpp tests.cpp -o test_asan

clean:
	rm -f parser test_asan

.PHONY: all asan clean
