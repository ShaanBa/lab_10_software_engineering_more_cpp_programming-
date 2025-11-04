# Makefile for EECS 348 Lab 10
# Author: Shaan Bawa (fixed)
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Werror
TARGET = calculator
SRCS = main.cpp

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
