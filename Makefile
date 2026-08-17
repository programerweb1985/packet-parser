CXX      ?= g++
CXXFLAGS ?= -g -O0 -std=c++17 -Wall -Wextra

# Normal build (server demo)
all: parser

parser: parser.cpp main.cpp parser.h
	$(CXX) $(CXXFLAGS) parser.cpp main.cpp -o parser

# Sanitizer build (memory-safety verification)
asan: parser.cpp exploit.cpp parser.h
	$(CXX) $(CXXFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer \
	  parser.cpp exploit.cpp -o exploit_asan

clean:
	rm -f parser exploit_asan

.PHONY: all asan clean
