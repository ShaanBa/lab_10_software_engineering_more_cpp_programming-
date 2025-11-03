# Makefile for EECS 348 Lab 10
# Author: Shaan Bawa

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Werror -o

# Target executable name
TARGET = calculator

# Source file
SRCS = main.cpp

# Default rule: build the target
all: $(TARGET)

# Rule to build the target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(TARGET) $(SRCS)

# Rule to clean up compiled files
clean:
	rm -f $(TARGET)