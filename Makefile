CXX := g++
CXXFLAGS := -std=c++17 -O2 -pthread -Wall -Wextra -Iinclude
LDFLAGS := -pthread

SRC := $(wildcard src/*.cpp)
OBJ := $(SRC:.cpp=.o)

BIN_DIR := bin
BIN_NODE := $(BIN_DIR)/node
BIN_CLIENT := $(BIN_DIR)/client

all: $(BIN_NODE) $(BIN_CLIENT)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BIN_NODE): $(OBJ) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ) $(LDFLAGS)

# Build client separately to avoid linking server objects twice
src/client_main.o: src/client_main.cpp include/client.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BIN_CLIENT): src/client_main.o src/client.o
	$(CXX) $(CXXFLAGS) -o $@ src/client_main.o src/client.o $(LDFLAGS)

clean:
	rm -rf $(BIN_DIR) src/*.o
