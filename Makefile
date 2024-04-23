CXX := mpicxx # MPI compiler
NVCC := nvcc # CUDA compiler
CXXFLAGS := -std=c++14 -Wall -O2 $(shell pkg-config --cflags opencv4)
CUDAFLAGS := -g -G -std=c++14 -ccbin=$(CXX) -Xcompiler "$(CXXFLAGS)" $(shell pkg-config --cflags opencv4)
LDFLAGS := $(shell pkg-config --libs opencv4) -L/usr/local/cuda-11.2/lib64 -lcudadevrt -lcudart -lstdc++

TARGET := image_to_ascii
OBJ_DIR := obj

MPI_SRC := main.cpp utils.cpp constants.cpp
CUDA_SRC := image_processing.cu

MPI_OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(MPI_SRC))
CUDA_OBJ := $(patsubst %.cu, $(OBJ_DIR)/%.o, $(CUDA_SRC))

all: $(TARGET)

$(TARGET): $(MPI_OBJ) $(CUDA_OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cu
	mkdir -p $(OBJ_DIR)
	$(NVCC) $(CUDAFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

.PHONY: all clean
