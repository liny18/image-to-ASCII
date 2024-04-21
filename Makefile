CXX := mpicxx # MPI compiler
NVCC := nvcc # CUDA compiler
CXXFLAGS := -std=c++14 -Wall -O2 $(shell pkg-config --cflags opencv4)
CUDAFLAGS := -g -G -std=c++14 -ccbin=$(CXX) -Xcompiler "$(CXXFLAGS)" $(shell pkg-config --cflags opencv4)

LDFLAGS := $(shell pkg-config --libs opencv4) -L/usr/local/cuda-11.2/lib64 -lcudadevrt -lcudart -lstdc++

# Name of the executable
TARGET := image_to_ascii

# Source files
MPI_SRC := main.cpp utils.cpp constants.cpp
CUDA_SRC := image_processing.cu
# Object files
MPI_OBJ := $(MPI_SRC:.cpp=.o)
CUDA_OBJ := $(CUDA_SRC:.cu=.o)

all: $(TARGET)

$(TARGET): $(MPI_OBJ) $(CUDA_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(MPI_OBJ): %.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(CUDA_OBJ): %.o: %.cu
	$(NVCC) $(CUDAFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(MPI_OBJ) $(CUDA_OBJ)

.PHONY: all clean
