all: main

# Makefile

# Compiler and compiler flags
CXX = g++
CXXFLAGS = -g -std=c++11

# Target executable name
TARGET = main.out

# Source files
SOURCES = main.cpp

# Object files
OBJECTS = $(SOURCES:.cpp=.o)

# Build target
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(TARGET)

# Compile source files to object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJECTS)

# Phony targets
.PHONY: clean
