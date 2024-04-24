CXX := mpicxx # MPI compiler
NVCC := nvcc # CUDA compiler
CXXFLAGS := -std=c++14 -Wall -O2 -march=native $(shell pkg-config --cflags opencv4)
LDFLAGS := $(shell pkg-config --libs opencv4) -lstdc++

OBJ_DIR := obj
TARGET := out

# Default value for USE_GPU
USE_GPU ?= 0

# Conditional setup based on USE_GPU flag
ifeq ($(USE_GPU), 1)
    CUDAFLAGS := -g -G -std=c++14 -ccbin=$(CXX) -Xcompiler "$(CXXFLAGS)" $(shell pkg-config --cflags opencv4)
    CXXFLAGS += -DUSE_GPU
    LDFLAGS += -L/usr/local/cuda/lib64 -lcudadevrt -lcudart
    SRC := main.cpp utils.cpp constants.cpp image_processing.cu
    OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(filter %.cpp, $(SRC))) $(patsubst %.cu, $(OBJ_DIR)/%.o, $(filter %.cu, $(SRC)))
else
    SRC := main.cpp utils.cpp constants.cpp image_processing.cpp
    OBJ := $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(SRC))
endif

all: $(TARGET)

$(TARGET): $(OBJ)
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
