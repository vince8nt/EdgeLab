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
    cl_device_id getDevice() const;
    cl_context getContext() const;
    cl_command_queue getQueue() const;

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

    void checkError(cl_int error, const std::string& operation) const;
    std::string getErrorString(cl_int error) const;
};

// OpenCLWrapper implementation
#ifdef OPENCL_AVAILABLE

OpenCLWrapper::OpenCLWrapper() : platform_(nullptr), device_(nullptr), context_(nullptr), command_queue_(nullptr), initialized_(false) {}

OpenCLWrapper::~OpenCLWrapper() {
    if (initialized_) {
        if (command_queue_) clReleaseCommandQueue(command_queue_);
        if (context_) clReleaseContext(context_);
        if (device_) clReleaseDevice(device_);
    }
}

void OpenCLWrapper::initialize() {
    if (initialized_) return;
    
    cl_int error;
    
    // Get platform
    cl_uint num_platforms;
    error = clGetPlatformIDs(0, nullptr, &num_platforms);
    checkError(error, "clGetPlatformIDs");
    
    std::vector<cl_platform_id> platforms(num_platforms);
    error = clGetPlatformIDs(num_platforms, platforms.data(), nullptr);
    checkError(error, "clGetPlatformIDs");
    
    platform_ = platforms[0]; // Use first platform
    
    initialized_ = true;
}

void OpenCLWrapper::selectDevice(cl_device_type device_type) {
    if (!initialized_) initialize();
    
    cl_int error;
    
    // Get devices
    cl_uint num_devices;
    error = clGetDeviceIDs(platform_, device_type, 0, nullptr, &num_devices);
    checkError(error, "clGetDeviceIDs");
    
    std::vector<cl_device_id> devices(num_devices);
    error = clGetDeviceIDs(platform_, device_type, num_devices, devices.data(), nullptr);
    checkError(error, "clGetDeviceIDs");
    
    device_ = devices[0]; // Use first device
    
    // Create context
    context_ = clCreateContext(nullptr, 1, &device_, nullptr, nullptr, &error);
    checkError(error, "clCreateContext");
    
    // Create command queue
    command_queue_ = clCreateCommandQueueWithProperties(context_, device_, nullptr, &error);
    checkError(error, "clCreateCommandQueueWithProperties");
}

cl_device_id OpenCLWrapper::getDevice() const { return device_; }
cl_context OpenCLWrapper::getContext() const { return context_; }
cl_command_queue OpenCLWrapper::getQueue() const { return command_queue_; }

cl_program OpenCLWrapper::createProgram(const std::string& source) {
    cl_int error;
    const char* source_cstr = source.c_str();
    size_t source_size = source.length();
    
    cl_program program = clCreateProgramWithSource(context_, 1, &source_cstr, &source_size, &error);
    checkError(error, "clCreateProgramWithSource");
    
    error = clBuildProgram(program, 1, &device_, nullptr, nullptr, nullptr);
    if (error != CL_SUCCESS) {
        size_t log_size;
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        std::string error_msg = "clBuildProgram failed: " + std::string(log.data());
        throw std::runtime_error(error_msg);
    }
    
    return program;
}

cl_kernel OpenCLWrapper::createKernel(cl_program program, const std::string& kernel_name) {
    cl_int error;
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &error);
    checkError(error, "clCreateKernel");
    return kernel;
}

cl_mem OpenCLWrapper::createBuffer(cl_mem_flags flags, size_t size, void* host_ptr) {
    cl_int error;
    cl_mem buffer = clCreateBuffer(context_, flags, size, host_ptr, &error);
    checkError(error, "clCreateBuffer");
    return buffer;
}

void OpenCLWrapper::writeBuffer(cl_mem buffer, size_t size, const void* data) {
    cl_int error = clEnqueueWriteBuffer(command_queue_, buffer, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
    checkError(error, "clEnqueueWriteBuffer");
}

void OpenCLWrapper::readBuffer(cl_mem buffer, size_t size, void* data) {
    cl_int error = clEnqueueReadBuffer(command_queue_, buffer, CL_TRUE, 0, size, data, 0, nullptr, nullptr);
    checkError(error, "clEnqueueReadBuffer");
}

void OpenCLWrapper::releaseBuffer(cl_mem buffer) {
    cl_int error = clReleaseMemObject(buffer);
    checkError(error, "clReleaseMemObject");
}

void OpenCLWrapper::executeKernel(cl_kernel kernel, size_t global_size, size_t local_size) {
    cl_int error;
    if (local_size > 0) {
        error = clEnqueueNDRangeKernel(command_queue_, kernel, 1, nullptr, &global_size, &local_size, 0, nullptr, nullptr);
    } else {
        error = clEnqueueNDRangeKernel(command_queue_, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
    }
    checkError(error, "clEnqueueNDRangeKernel");
}

void OpenCLWrapper::finish() {
    cl_int error = clFinish(command_queue_);
    checkError(error, "clFinish");
}

size_t OpenCLWrapper::getMaxWorkGroupSize() const {
    size_t max_work_group_size;
    cl_int error = clGetDeviceInfo(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &max_work_group_size, nullptr);
    checkError(error, "clGetDeviceInfo");
    return max_work_group_size;
}

size_t OpenCLWrapper::getMaxComputeUnits() const {
    cl_uint max_compute_units;
    cl_int error = clGetDeviceInfo(device_, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_compute_units, nullptr);
    checkError(error, "clGetDeviceInfo");
    return max_compute_units;
}

std::string OpenCLWrapper::getDeviceName() const {
    size_t name_size;
    cl_int error = clGetDeviceInfo(device_, CL_DEVICE_NAME, 0, nullptr, &name_size);
    checkError(error, "clGetDeviceInfo");
    
    std::vector<char> name(name_size);
    error = clGetDeviceInfo(device_, CL_DEVICE_NAME, name_size, name.data(), nullptr);
    checkError(error, "clGetDeviceInfo");
    
    return std::string(name.data());
}

void OpenCLWrapper::checkError(cl_int error, const std::string& operation) const {
    if (error != CL_SUCCESS) {
        std::stringstream ss;
        ss << "OpenCL error in " << operation << ": " << getErrorString(error) << " (" << error << ")";
        throw std::runtime_error(ss.str());
    }
}

std::string OpenCLWrapper::getErrorString(cl_int error) const {
    switch (error) {
        case CL_SUCCESS: return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
        case CL_COMPILER_NOT_AVAILABLE: return "CL_COMPILER_NOT_AVAILABLE";
        case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
        case CL_PROFILING_INFO_NOT_AVAILABLE: return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case CL_MEM_COPY_OVERLAP: return "CL_MEM_COPY_OVERLAP";
        case CL_IMAGE_FORMAT_MISMATCH: return "CL_IMAGE_FORMAT_MISMATCH";
        case CL_IMAGE_FORMAT_NOT_SUPPORTED: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
        case CL_MAP_FAILURE: return "CL_MAP_FAILURE";
        case CL_MISALIGNED_SUB_BUFFER_OFFSET: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case CL_COMPILE_PROGRAM_FAILURE: return "CL_COMPILE_PROGRAM_FAILURE";
        case CL_LINKER_NOT_AVAILABLE: return "CL_LINKER_NOT_AVAILABLE";
        case CL_LINK_PROGRAM_FAILURE: return "CL_LINK_PROGRAM_FAILURE";
        case CL_DEVICE_PARTITION_FAILED: return "CL_DEVICE_PARTITION_FAILED";
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";
        case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
        case CL_INVALID_DEVICE_TYPE: return "CL_INVALID_DEVICE_TYPE";
        case CL_INVALID_PLATFORM: return "CL_INVALID_PLATFORM";
        case CL_INVALID_DEVICE: return "CL_INVALID_DEVICE";
        case CL_INVALID_CONTEXT: return "CL_INVALID_CONTEXT";
        case CL_INVALID_QUEUE_PROPERTIES: return "CL_INVALID_QUEUE_PROPERTIES";
        case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
        case CL_INVALID_HOST_PTR: return "CL_INVALID_HOST_PTR";
        case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case CL_INVALID_IMAGE_SIZE: return "CL_INVALID_IMAGE_SIZE";
        case CL_INVALID_SAMPLER: return "CL_INVALID_SAMPLER";
        case CL_INVALID_BINARY: return "CL_INVALID_BINARY";
        case CL_INVALID_BUILD_OPTIONS: return "CL_INVALID_BUILD_OPTIONS";
        case CL_INVALID_PROGRAM: return "CL_INVALID_PROGRAM";
        case CL_INVALID_PROGRAM_EXECUTABLE: return "CL_INVALID_PROGRAM_EXECUTABLE";
        case CL_INVALID_KERNEL_NAME: return "CL_INVALID_KERNEL_NAME";
        case CL_INVALID_KERNEL_DEFINITION: return "CL_INVALID_KERNEL_DEFINITION";
        case CL_INVALID_KERNEL: return "CL_INVALID_KERNEL";
        case CL_INVALID_ARG_INDEX: return "CL_INVALID_ARG_INDEX";
        case CL_INVALID_ARG_VALUE: return "CL_INVALID_ARG_VALUE";
        case CL_INVALID_ARG_SIZE: return "CL_INVALID_ARG_SIZE";
        case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
        case CL_INVALID_WORK_DIMENSION: return "CL_INVALID_WORK_DIMENSION";
        case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
        case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
        case CL_INVALID_GLOBAL_OFFSET: return "CL_INVALID_GLOBAL_OFFSET";
        case CL_INVALID_EVENT_WAIT_LIST: return "CL_INVALID_EVENT_WAIT_LIST";
        case CL_INVALID_EVENT: return "CL_INVALID_EVENT";
        case CL_INVALID_OPERATION: return "CL_INVALID_OPERATION";
        case CL_INVALID_GL_OBJECT: return "CL_INVALID_GL_OBJECT";
        case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
        case CL_INVALID_MIP_LEVEL: return "CL_INVALID_MIP_LEVEL";
        case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";
        case CL_INVALID_PROPERTY: return "CL_INVALID_PROPERTY";
        default: return "Unknown error";
    }
}

#else
// Stub implementations when OpenCL is not available
OpenCLWrapper::OpenCLWrapper() : initialized_(false) {}
OpenCLWrapper::~OpenCLWrapper() {}

void OpenCLWrapper::initialize() {
    throw std::runtime_error("OpenCL is not available on this system");
}

void OpenCLWrapper::selectDevice(cl_device_type device_type) {
    throw std::runtime_error("OpenCL is not available on this system");
}

cl_device_id OpenCLWrapper::getDevice() const { return nullptr; }
cl_context OpenCLWrapper::getContext() const { return nullptr; }
cl_command_queue OpenCLWrapper::getQueue() const { return nullptr; }

cl_program OpenCLWrapper::createProgram(const std::string& source) {
    throw std::runtime_error("OpenCL is not available on this system");
}

cl_kernel OpenCLWrapper::createKernel(cl_program program, const std::string& kernel_name) {
    throw std::runtime_error("OpenCL is not available on this system");
}

cl_mem OpenCLWrapper::createBuffer(cl_mem_flags flags, size_t size, void* host_ptr) {
    throw std::runtime_error("OpenCL is not available on this system");
}

void OpenCLWrapper::writeBuffer(cl_mem buffer, size_t size, const void* data) {
    throw std::runtime_error("OpenCL is not available on this system");
}

void OpenCLWrapper::readBuffer(cl_mem buffer, size_t size, void* data) {
    throw std::runtime_error("OpenCL is not available on this system");
}

void OpenCLWrapper::releaseBuffer(cl_mem buffer) {
    throw std::runtime_error("OpenCL is not available on this system");
}

void OpenCLWrapper::executeKernel(cl_kernel kernel, size_t global_size, size_t local_size) {
    throw std::runtime_error("OpenCL is not available on this system");
}

void OpenCLWrapper::finish() {
    throw std::runtime_error("OpenCL is not available on this system");
}

size_t OpenCLWrapper::getMaxWorkGroupSize() const { return 0; }
size_t OpenCLWrapper::getMaxComputeUnits() const { return 0; }
std::string OpenCLWrapper::getDeviceName() const { return "OpenCL not available"; }

void OpenCLWrapper::checkError(cl_int error, const std::string& operation) const {
    throw std::runtime_error("OpenCL is not available on this system");
}

std::string OpenCLWrapper::getErrorString(cl_int error) const {
    return "OpenCL not available";
}

#endif

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