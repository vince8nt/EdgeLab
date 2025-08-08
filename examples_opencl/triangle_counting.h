#ifndef TRIANGLE_COUNTING_OPENCL_H_
#define TRIANGLE_COUNTING_OPENCL_H_

#include "../src/graph_test_opencl.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// OpenCL Triangle Counting implementation
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
long long triangle_counting_opencl(Graph<Vertex_t, Edge_t, Graph_t>& graph) {
    
    OpenCLWrapper opencl;
    opencl.initialize();
    opencl.selectDevice(CL_DEVICE_TYPE_GPU);
    
    std::cout << "Using OpenCL device: " << opencl.getDeviceName() << std::endl;
    std::cout << "Max work group size: " << opencl.getMaxWorkGroupSize() << std::endl;
    std::cout << "Max compute units: " << opencl.getMaxComputeUnits() << std::endl;
    
    // Load OpenCL kernel source
            std::ifstream kernel_file("../examples_opencl/triangle_counting.cl");
        if (!kernel_file.is_open()) {
            throw std::runtime_error("Failed to open kernel file: ../examples_opencl/triangle_counting.cl");
    }
    
    std::stringstream kernel_source;
    kernel_source << kernel_file.rdbuf();
    kernel_file.close();
    
    // Create OpenCL program
    cl_program program = opencl.createProgram(kernel_source.str());
    
    // Create kernel
    cl_kernel triangle_kernel = opencl.createKernel(program, "count_triangles");
    
    // Prepare graph data for OpenCL
    vertex_ID_t num_vertices = graph.num_vertices();
    
    // Convert graph to CSR format for OpenCL
    std::vector<cl_uint> vertex_offsets(num_vertices + 1);
    std::vector<cl_uint> edge_destinations;
    
    edge_ID_t edge_offset = 0;
    for (vertex_ID_t i = 0; i < num_vertices; i++) {
        vertex_offsets[i] = edge_offset;
        auto vertex = graph[i];
        
        // Collect all neighbors for this vertex
        std::vector<cl_uint> neighbors;
        for (auto edge : vertex) {
            neighbors.push_back(edge.dest());
        }
        
        // Sort neighbors to ensure proper intersection algorithm
        std::sort(neighbors.begin(), neighbors.end());
        
        // Add sorted neighbors to edge destinations
        for (cl_uint neighbor : neighbors) {
            edge_destinations.push_back(neighbor);
        }
        
        edge_offset += vertex.degree();
    }
    vertex_offsets[num_vertices] = edge_offset;
    
    // Create OpenCL buffers
    cl_mem vertices_buffer = opencl.createBuffer(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                                vertex_offsets.size() * sizeof(cl_uint),
                                                vertex_offsets.data());
    
    cl_mem edges_buffer = opencl.createBuffer(CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                             edge_destinations.size() * sizeof(cl_uint),
                                             edge_destinations.data());
    
    cl_mem triangle_count_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                                      sizeof(cl_uint));
    
    // Initialize triangle count to zero
    cl_uint zero = 0;
    opencl.writeBuffer(triangle_count_buffer, sizeof(cl_uint), &zero);
    
    // Set kernel arguments
    clSetKernelArg(triangle_kernel, 0, sizeof(cl_mem), &vertices_buffer);
    clSetKernelArg(triangle_kernel, 1, sizeof(cl_mem), &edges_buffer);
    clSetKernelArg(triangle_kernel, 2, sizeof(cl_mem), &triangle_count_buffer);
    clSetKernelArg(triangle_kernel, 3, sizeof(vertex_ID_t), &num_vertices);
    
    // Execute kernel
    // Use a work group size that's a power of 2 and doesn't exceed max work group size
    size_t max_work_group_size = opencl.getMaxWorkGroupSize();
    size_t local_size = 256; // Common choice for GPU kernels
    if (local_size > max_work_group_size) {
        local_size = max_work_group_size;
    }
    
    // Round up global size to be a multiple of local size
    size_t global_size = ((num_vertices + local_size - 1) / local_size) * local_size;
    
    opencl.executeKernel(triangle_kernel, global_size, local_size);
    
    // Read result
    cl_uint triangle_count;
    opencl.readBuffer(triangle_count_buffer, sizeof(cl_uint), &triangle_count);
    
    // Cleanup
    opencl.releaseBuffer(vertices_buffer);
    opencl.releaseBuffer(edges_buffer);
    opencl.releaseBuffer(triangle_count_buffer);
    clReleaseKernel(triangle_kernel);
    clReleaseProgram(program);
    
    return triangle_count;
}

#endif // TRIANGLE_COUNTING_OPENCL_H 