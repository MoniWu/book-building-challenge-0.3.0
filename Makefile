CXX ?= g++
CXXFLAGS ?= -O2 -std=c++20 -Wall -Wextra -Icpp
LDFLAGS ?=

BIN_DIR := bin
SRC := cpp/main.cpp cpp/pcap.cpp cpp/protocol.cpp cpp/book.cpp cpp/snapshot_writer.cpp
OBJ := $(SRC:.cpp=.o)
BIN := $(BIN_DIR)/build-book
TEST_BIN := tests/test_protocol

.PHONY: build clean test run

build: $(BIN)

$(BIN): $(OBJ)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

cpp/%.o: cpp/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_BIN): tests/test_protocol.cpp cpp/protocol.o cpp/pcap.o cpp/book.o cpp/snapshot_writer.o
	$(CXX) $(CXXFLAGS) -o $@ tests/test_protocol.cpp cpp/protocol.o cpp/pcap.o cpp/book.o cpp/snapshot_writer.o $(LDFLAGS)

test: $(TEST_BIN)
	./$(TEST_BIN)

run: build
	./$(BIN) --input input.pcap --output output.csv

clean:
	rm -f $(OBJ) $(BIN) $(TEST_BIN)
