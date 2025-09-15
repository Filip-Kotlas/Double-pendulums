#pragma once
#include <cuda.h>
#include <cuda_runtime.h>
#include <stdexcept>
#include <vector>
#include <iostream>

class CudaArray{
    public:
    CudaArray()
    : size_of_array(0),
    ptr(nullptr) {}

    void resize(int n){
        this->size_of_array = n;

        if (ptr != nullptr) {
            if (cudaFree(ptr) != cudaSuccess) {
                throw std::runtime_error("Error while deleting CudaArray in resize.");
            }        
        }
        if (cudaMalloc((void **) &ptr, this->size_of_array*sizeof(double)) != cudaSuccess) {
            throw std::runtime_error("Error while allocating CudaArray.");
        }
   }

    ~CudaArray() {
        if( ptr != nullptr) {
            if (cudaFree(ptr) != cudaSuccess) {
                throw std::runtime_error("Error while deleting CudaArray in destructor.");
            }
        }
    }

    // copy
    CudaArray(const CudaArray&) = delete;
    CudaArray& operator=(const CudaArray&) = delete;

    // move
    CudaArray(CudaArray&& other) noexcept : size_of_array(other.size_of_array), ptr(other.ptr) {
        other.size_of_array = 0;
        other.ptr = nullptr;
    }
    CudaArray& operator=(CudaArray&& other) noexcept {
        if (this != &other) {
            if (cudaFree(ptr) != cudaSuccess) {
                throw std::runtime_error("Error while deleting CudaArray in move assignment.");
            }
            this->size_of_array = other.size_of_array;
            this->ptr = other.ptr;
            other.size_of_array = 0;
            other.ptr = nullptr;
        }
        return *this;
    }

    int size() { return this->size_of_array; };

    double* data() { return this->ptr; };
    const double* data() const { return this->ptr; };

    std::vector<double> copy_to_host() {
        std::vector<double> data(this->size_of_array);
        if (cudaMemcpy(data.data(), this->ptr, this->size_of_array * sizeof(double), cudaMemcpyDeviceToHost) != cudaSuccess) {
            throw std::runtime_error("Error copying data from device to host.");
        }
        return data;
    }

    void copy_from_host(const std::vector<double>& data) {
        if (data.size() != this->size_of_array) {
            throw std::runtime_error("Data size does not match CudaArray size.");
        }
        if (cudaMemcpy(this->ptr, data.data(), this->size_of_array * sizeof(double), cudaMemcpyHostToDevice) != cudaSuccess) {
            throw std::runtime_error("Error copying data from host to device.");
        }
    }

    private:
    int size_of_array;
    double* ptr = nullptr;
};