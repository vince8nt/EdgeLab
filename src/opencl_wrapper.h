#ifndef OPENCL_WRAPPER_H_
#define OPENCL_WRAPPER_H_

#include <CL/cl.h>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <functional>
#include "util.h"

class OpenCLWrapper {
public:
    OpenCLWrapper();
    ~OpenCLWrapper();

    // Device management
    void initialize();
    void selectDevice(cl_device_type device_type = CL_DEVICE_TYPE_GPU);
    cl_device_id getDevice() const { return device_; }
    cl_context getContext() const { return context_; }
    cl_command_queue getQueue() const { return command_queue_; }

    // Kernel management
    cl_program createProgram(const std::string& source);
    cl_kernel createKernel(cl_program program, const std::string& kernel_name);
    
    // Memory management
    cl_mem createBuffer(cl_mem_flags flags, size_t size, void* host_ptr = nullptr);
    void writeBuffer(cl_mem buffer, size_t size, const void* data);
    void readBuffer(cl_mem buffer, size_t size, void* data);
    void releaseBuffer(cl_mem buffer);

    // Execution
    void executeKernel(cl_kernel kernel, size_t global_size, size_t local_size = 0);
    void finish();

    // Utility
    size_t getMaxWorkGroupSize() const;
    size_t getMaxComputeUnits() const;
    std::string getDeviceName() const;

private:
    cl_platform_id platform_;
    cl_device_id device_;
    cl_context context_;
    cl_command_queue command_queue_;
    bool initialized_;

    void checkError(cl_int error, const std::string& operation);
    std::string getErrorString(cl_int error);
};

// RAII wrapper for OpenCL objects
template<typename T>
class OpenCLObject {
public:
    OpenCLObject(T obj, std::function<void(T)> release_func) 
        : obj_(obj), release_func_(release_func) {}
    
    ~OpenCLObject() {
        if (obj_ && release_func_) {
            release_func_(obj_);
        }
    }
    
    T get() const { return obj_; }
    operator T() const { return obj_; }
    
    // Prevent copying
    OpenCLObject(const OpenCLObject&) = delete;
    OpenCLObject& operator=(const OpenCLObject&) = delete;
    
    // Allow moving
    OpenCLObject(OpenCLObject&& other) noexcept 
        : obj_(other.obj_), release_func_(std::move(other.release_func_)) {
        other.obj_ = nullptr;
    }
    
    OpenCLObject& operator=(OpenCLObject&& other) noexcept {
        if (this != &other) {
            if (obj_ && release_func_) {
                release_func_(obj_);
            }
            obj_ = other.obj_;
            release_func_ = std::move(other.release_func_);
            other.obj_ = nullptr;
        }
        return *this;
    }

private:
    T obj_;
    std::function<void(T)> release_func_;
};

// Type aliases for common OpenCL objects
using OpenCLBuffer = OpenCLObject<cl_mem>;
using OpenCLProgram = OpenCLObject<cl_program>;
using OpenCLKernel = OpenCLObject<cl_kernel>;

#endif // OPENCL_WRAPPER_H_ 