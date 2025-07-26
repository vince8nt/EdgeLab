#ifndef OPENCL_WRAPPER_H_
#define OPENCL_WRAPPER_H_

#ifdef OPENCL_AVAILABLE
#include <CL/cl.h>
#else
// Dummy OpenCL types for when OpenCL is not available
typedef void* cl_platform_id;
typedef void* cl_device_id;
typedef void* cl_context;
typedef void* cl_command_queue;
typedef void* cl_program;
typedef void* cl_kernel;
typedef void* cl_mem;
typedef unsigned int cl_uint;
typedef int cl_int;
typedef unsigned long cl_ulong;
typedef unsigned long cl_device_type;
typedef unsigned long cl_mem_flags;
#define CL_DEVICE_TYPE_GPU 0
#define CL_MEM_READ_ONLY 0
#define CL_MEM_READ_WRITE 0
#define CL_MEM_COPY_HOST_PTR 0
#define CL_SUCCESS 0
// Add some common OpenCL error codes for the stub implementation
#define CL_DEVICE_NOT_FOUND 0
#define CL_DEVICE_NOT_AVAILABLE 0
#define CL_COMPILER_NOT_AVAILABLE 0
#define CL_MEM_OBJECT_ALLOCATION_FAILURE 0
#define CL_OUT_OF_RESOURCES 0
#define CL_OUT_OF_HOST_MEMORY 0
#define CL_PROFILING_INFO_NOT_AVAILABLE 0
#define CL_MEM_COPY_OVERLAP 0
#define CL_IMAGE_FORMAT_MISMATCH 0
#define CL_IMAGE_FORMAT_NOT_SUPPORTED 0
#define CL_BUILD_PROGRAM_FAILURE 0
#define CL_MAP_FAILURE 0
#define CL_MISALIGNED_SUB_BUFFER_OFFSET 0
#define CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST 0
#define CL_COMPILE_PROGRAM_FAILURE 0
#define CL_LINKER_NOT_AVAILABLE 0
#define CL_LINK_PROGRAM_FAILURE 0
#define CL_DEVICE_PARTITION_FAILED 0
#define CL_KERNEL_ARG_INFO_NOT_AVAILABLE 0
#define CL_INVALID_VALUE 0
#define CL_INVALID_DEVICE_TYPE 0
#define CL_INVALID_PLATFORM 0
#define CL_INVALID_DEVICE 0
#define CL_INVALID_CONTEXT 0
#define CL_INVALID_QUEUE_PROPERTIES 0
#define CL_INVALID_COMMAND_QUEUE 0
#define CL_INVALID_HOST_PTR 0
#define CL_INVALID_MEM_OBJECT 0
#define CL_INVALID_IMAGE_FORMAT_DESCRIPTOR 0
#define CL_INVALID_IMAGE_SIZE 0
#define CL_INVALID_SAMPLER 0
#define CL_INVALID_BINARY 0
#define CL_INVALID_BUILD_OPTIONS 0
#define CL_INVALID_PROGRAM 0
#define CL_INVALID_PROGRAM_EXECUTABLE 0
#define CL_INVALID_KERNEL_NAME 0
#define CL_INVALID_KERNEL_DEFINITION 0
#define CL_INVALID_KERNEL 0
#define CL_INVALID_ARG_INDEX 0
#define CL_INVALID_ARG_VALUE 0
#define CL_INVALID_ARG_SIZE 0
#define CL_INVALID_KERNEL_ARGS 0
#define CL_INVALID_WORK_DIMENSION 0
#define CL_INVALID_WORK_GROUP_SIZE 0
#define CL_INVALID_WORK_ITEM_SIZE 0
#define CL_INVALID_GLOBAL_OFFSET 0
#define CL_INVALID_EVENT_WAIT_LIST 0
#define CL_INVALID_EVENT 0
#define CL_INVALID_OPERATION 0
#define CL_INVALID_GL_OBJECT 0
#define CL_INVALID_BUFFER_SIZE 0
#define CL_INVALID_MIP_LEVEL 0
#define CL_INVALID_GLOBAL_WORK_SIZE 0
#define CL_INVALID_PROPERTY 0
#endif
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