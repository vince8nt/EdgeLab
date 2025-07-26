#include "../src/cli_dispatch.h"
#include "../src/opencl_wrapper.h"
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
    std::ifstream kernel_file("../kernels/triangle_counting.cl");
    if (!kernel_file.is_open()) {
        throw std::runtime_error("Failed to open kernel file: ../kernels/triangle_counting.cl");
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
    
    // Debug: Print CSR format for verification
    std::cout << "CSR format - Vertices: " << num_vertices << ", Edges: " << edge_destinations.size() << std::endl;
    std::cout << "First few vertex offsets: ";
    for (int i = 0; i < std::min(10, (int)vertex_offsets.size()); i++) {
        std::cout << vertex_offsets[i] << " ";
    }
    std::cout << std::endl;
    std::cout << "First few edge destinations: ";
    for (int i = 0; i < std::min(10, (int)edge_destinations.size()); i++) {
        std::cout << edge_destinations[i] << " ";
    }
    std::cout << std::endl;
    
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
    
    std::cout << "Executing triangle counting kernel with " << global_size << " work items" << std::endl;
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
    
    std::cout << "Total triangles found: " << triangle_count << std::endl;
    
    // Debug: Let's also verify the CSR format by doing a simple CPU count
    // This will help us identify if the issue is in the CSR conversion or the kernel
    cl_uint cpu_triangles = 0;
    for (vertex_ID_t u = 0; u < num_vertices; u++) {
        cl_uint u_start = vertex_offsets[u];
        cl_uint u_end = vertex_offsets[u + 1];
        for (cl_uint i = u_start; i < u_end; i++) {
            cl_uint v = edge_destinations[i];
            if (u >= v) continue;
            cl_uint v_start = vertex_offsets[v];
            cl_uint v_end = vertex_offsets[v + 1];
            for (cl_uint j = v_start; j < v_end; j++) {
                cl_uint w = edge_destinations[j];
                if (v >= w) continue;
                // Binary search for w in u's neighbor list
                cl_uint left = u_start, right = u_end;
                while (left < right) {
                    cl_uint mid = left + (right - left) / 2;
                    if (edge_destinations[mid] < w) {
                        left = mid + 1;
                    } else {
                        right = mid;
                    }
                }
                if (left < u_end && edge_destinations[left] == w) {
                    cpu_triangles++;
                }
            }
        }
    }
    std::cout << "CPU verification triangles: " << cpu_triangles << std::endl;
    
    return triangle_count;
}

// Functor for dispatching templated function via CLI options
struct OpenCLTriangleDispatcher {
    int &exit_code;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        try {
            auto timer = timer_start();
            long long triangles = triangle_counting_opencl<V, E, G>(graph);
            auto time = timer_stop(timer);
            std::cout << "OpenCL Triangle Counting returned: " << triangles << " in " << time << " seconds" << std::endl;
        }
        catch (std::exception &e) {
            std::cerr << "Caught OpenCL Triangle Counting exception: " << e.what() << std::endl;
            exit_code = 1;
        }
    }
};

int main(int argc, char** argv) {
    int exit_code = 0;
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, OpenCLTriangleDispatcher{exit_code});
    if (exit_code)
        std::cerr << "Failed with exit code: " << exit_code << std::endl;
    else
        std::cout << "Succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
} 