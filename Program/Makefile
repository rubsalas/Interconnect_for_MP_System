# Makefile for Multiprocessor Architecture Simulator

CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Iinclude
SRC_DIR  := src

# Find all source files in src directory
SRCS     := $(wildcard $(SRC_DIR)/*.cpp)
# Object files have .o extension corresponding to .cpp
OBJS     := $(SRCS:.cpp=.o)

# Output executable
TARGET   := interconnect_sim

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compile rule: generate .o from .cpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	@echo "Running $(TARGET)..."
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
