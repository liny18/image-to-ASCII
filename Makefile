CXX := mpicxx  # Use the MPI wrapper compiler for C++
CXXFLAGS := -std=c++14 -Wall -O2 $(shell pkg-config --cflags opencv4)
LDFLAGS := $(shell pkg-config --libs opencv4)

# Name of the executable
TARGET := image_to_ascii

# Source files
SRC := image_to_ascii.cpp
# Object files
OBJ := $(SRC:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(TARGET) $(OBJ)

.PHONY: all clean
