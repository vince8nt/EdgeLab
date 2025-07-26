#include "opencl_wrapper.h"
#include <iostream>
#include <sstream>

#ifdef OPENCL_AVAILABLE

OpenCLWrapper::OpenCLWrapper() 
    : platform_(nullptr), device_(nullptr), context_(nullptr), 
      command_queue_(nullptr), initialized_(false) {
}

OpenCLWrapper::~OpenCLWrapper() {
    if (command_queue_) clReleaseCommandQueue(command_queue_);
    if (context_) clReleaseContext(context_);
    if (device_) clReleaseDevice(device_);
}

void OpenCLWrapper::initialize() {
    if (initialized_) return;

    // Get platform
    cl_uint num_platforms;
    checkError(clGetPlatformIDs(0, nullptr, &num_platforms), "Getting number of platforms");
    
    std::vector<cl_platform_id> platforms(num_platforms);
    checkError(clGetPlatformIDs(num_platforms, platforms.data(), nullptr), "Getting platforms");
    
    platform_ = platforms[0]; // Use first platform

    initialized_ = true;
}

void OpenCLWrapper::selectDevice(cl_device_type device_type) {
    if (!initialized_) initialize();

    // Get devices
    cl_uint num_devices;
    checkError(clGetDeviceIDs(platform_, device_type, 0, nullptr, &num_devices), 
               "Getting number of devices");
    
    std::vector<cl_device_id> devices(num_devices);
    checkError(clGetDeviceIDs(platform_, device_type, num_devices, devices.data(), nullptr), 
               "Getting devices");
    
    device_ = devices[0]; // Use first device

    // Create context
    cl_int error;
    context_ = clCreateContext(nullptr, 1, &device_, nullptr, nullptr, &error);
    checkError(error, "Creating context");

    // Create command queue
    command_queue_ = clCreateCommandQueueWithProperties(context_, device_, nullptr, &error);
    checkError(error, "Creating command queue");
}

cl_program OpenCLWrapper::createProgram(const std::string& source) {
    const char* source_ptr = source.c_str();
    size_t source_size = source.length();
    
    cl_int error;
    cl_program program = clCreateProgramWithSource(context_, 1, &source_ptr, &source_size, &error);
    checkError(error, "Creating program");

    // Build program
    error = clBuildProgram(program, 1, &device_, nullptr, nullptr, nullptr);
    if (error != CL_SUCCESS) {
        // Get build log
        size_t log_size;
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
        std::vector<char> log(log_size);
        clGetProgramBuildInfo(program, device_, CL_PROGRAM_BUILD_LOG, log_size, log.data(), nullptr);
        
        std::cerr << "OpenCL build error: " << std::string(log.data()) << std::endl;
        checkError(error, "Building program");
    }

    return program;
}

cl_kernel OpenCLWrapper::createKernel(cl_program program, const std::string& kernel_name) {
    cl_int error;
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &error);
    checkError(error, "Creating kernel");
    return kernel;
}

cl_mem OpenCLWrapper::createBuffer(cl_mem_flags flags, size_t size, void* host_ptr) {
    cl_int error;
    cl_mem buffer = clCreateBuffer(context_, flags, size, host_ptr, &error);
    checkError(error, "Creating buffer");
    return buffer;
}

void OpenCLWrapper::writeBuffer(cl_mem buffer, size_t size, const void* data) {
    checkError(clEnqueueWriteBuffer(command_queue_, buffer, CL_TRUE, 0, size, data, 0, nullptr, nullptr),
               "Writing buffer");
}

void OpenCLWrapper::readBuffer(cl_mem buffer, size_t size, void* data) {
    checkError(clEnqueueReadBuffer(command_queue_, buffer, CL_TRUE, 0, size, data, 0, nullptr, nullptr),
               "Reading buffer");
}

void OpenCLWrapper::releaseBuffer(cl_mem buffer) {
    clReleaseMemObject(buffer);
}

void OpenCLWrapper::executeKernel(cl_kernel kernel, size_t global_size, size_t local_size) {
    checkError(clEnqueueNDRangeKernel(command_queue_, kernel, 1, nullptr, &global_size, 
                                     local_size > 0 ? &local_size : nullptr, 0, nullptr, nullptr),
               "Executing kernel");
}

void OpenCLWrapper::finish() {
    clFinish(command_queue_);
}

size_t OpenCLWrapper::getMaxWorkGroupSize() const {
    size_t size;
    clGetDeviceInfo(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &size, nullptr);
    return size;
}

size_t OpenCLWrapper::getMaxComputeUnits() const {
    cl_uint units;
    clGetDeviceInfo(device_, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &units, nullptr);
    return units;
}

std::string OpenCLWrapper::getDeviceName() const {
    size_t name_size;
    clGetDeviceInfo(device_, CL_DEVICE_NAME, 0, nullptr, &name_size);
    std::vector<char> name(name_size);
    clGetDeviceInfo(device_, CL_DEVICE_NAME, name_size, name.data(), nullptr);
    return std::string(name.data());
}

void OpenCLWrapper::checkError(cl_int error, const std::string& operation) {
    if (error != CL_SUCCESS) {
        std::stringstream ss;
        ss << "OpenCL error in " << operation << ": " << getErrorString(error) << " (" << error << ")";
        throw std::runtime_error(ss.str());
    }
}

std::string OpenCLWrapper::getErrorString(cl_int error) {
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

void OpenCLWrapper::checkError(cl_int error, const std::string& operation) {
    throw std::runtime_error("OpenCL is not available on this system");
}

std::string OpenCLWrapper::getErrorString(cl_int error) {
    return "OpenCL not available";
}

#endif 