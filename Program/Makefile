# Makefile for Multiprocessor Architecture Simulator

# Compiler and flags
CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Iinclude

# Directories
SRC_DIR := src
INC_DIR := include

# Recursively gather all .cpp source files
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cpp')

# (Optional) Gather all header files for dependency tracking
HDRS := $(shell find $(INC_DIR) -type f -name '*.h')

# Object files corresponding to source files
OBJS := $(SRCS:.cpp=.o)

# Output executable
TARGET := interconnect_sim

.PHONY: all clean run

# Default target: build executable
all: $(TARGET)

# Link object files into the final binary
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile rule: any .cpp to its .o counterpart
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the simulator
run: all
	@echo "Running $(TARGET)..."
	./$(TARGET)

# Clean object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
