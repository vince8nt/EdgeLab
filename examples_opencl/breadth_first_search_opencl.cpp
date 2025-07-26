#include "../src/cli_dispatch.h"
#include "../src/opencl_wrapper.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

// OpenCL BFS implementation
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
long long breadth_first_search_opencl(Graph<Vertex_t, Edge_t, Graph_t>& graph, 
                                     vertex_ID_t src, vertex_ID_t dest) {
    
    OpenCLWrapper opencl;
    opencl.initialize();
    opencl.selectDevice(CL_DEVICE_TYPE_GPU);
    
    std::cout << "Using OpenCL device: " << opencl.getDeviceName() << std::endl;
    std::cout << "Max work group size: " << opencl.getMaxWorkGroupSize() << std::endl;
    std::cout << "Max compute units: " << opencl.getMaxComputeUnits() << std::endl;
    
    // Load OpenCL kernel source
    std::ifstream kernel_file("../kernels/bfs.cl");
    if (!kernel_file.is_open()) {
        throw std::runtime_error("Failed to open kernel file: ../kernels/bfs.cl");
    }
    
    std::stringstream kernel_source;
    kernel_source << kernel_file.rdbuf();
    kernel_file.close();
    
    // Create OpenCL program
    cl_program program = opencl.createProgram(kernel_source.str());
    
    // Create kernels
    cl_kernel init_kernel = opencl.createKernel(program, "bfs_init");
    cl_kernel level_kernel = opencl.createKernel(program, "bfs_level");
    cl_kernel check_kernel = opencl.createKernel(program, "check_destination");
    
    // Prepare graph data for OpenCL
    vertex_ID_t num_vertices = graph.num_vertices();
    
    // Convert graph to CSR format for OpenCL
    std::vector<cl_uint> vertex_offsets(num_vertices + 1);
    std::vector<cl_uint> edge_destinations;
    std::vector<float> edge_weights;
    
    edge_ID_t edge_offset = 0;
    for (vertex_ID_t i = 0; i < num_vertices; i++) {
        vertex_offsets[i] = edge_offset;
        auto vertex = graph[i];
        for (auto edge : vertex) {
            edge_destinations.push_back(edge.dest());
            if constexpr (WeightedEdgeType<Edge_t>) {
                edge_weights.push_back(edge.weight());
            } else {
                edge_weights.push_back(1.0f);
            }
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
    
    cl_mem distances_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                                 num_vertices * sizeof(int));
    
    cl_mem visited_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                               num_vertices * sizeof(int));
    
    cl_mem current_level_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                                     num_vertices * sizeof(vertex_ID_t));
    
    cl_mem next_level_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                                  num_vertices * sizeof(vertex_ID_t));
    
    cl_mem next_level_size_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                                       sizeof(int));
    
    cl_mem found_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                             sizeof(int));
    
    cl_mem result_distance_buffer = opencl.createBuffer(CL_MEM_READ_WRITE,
                                                       sizeof(int));
    
    // Initialize BFS data structures
    int zero = 0;
    opencl.writeBuffer(next_level_size_buffer, sizeof(int), &zero);
    opencl.writeBuffer(found_buffer, sizeof(int), &zero);
    
    // Set kernel arguments for initialization
    clSetKernelArg(init_kernel, 0, sizeof(cl_mem), &distances_buffer);
    clSetKernelArg(init_kernel, 1, sizeof(cl_mem), &visited_buffer);
    clSetKernelArg(init_kernel, 2, sizeof(vertex_ID_t), &num_vertices);
    clSetKernelArg(init_kernel, 3, sizeof(vertex_ID_t), &src);
    
    // Execute initialization kernel
    opencl.executeKernel(init_kernel, num_vertices);
    
    // Main BFS loop
    int current_distance = 0;
    int current_level_size = 1;
    std::vector<vertex_ID_t> current_level = {src};
    
    while (current_level_size > 0) {
        // Copy current level to device
        opencl.writeBuffer(current_level_buffer, 
                          current_level_size * sizeof(vertex_ID_t), 
                          current_level.data());
        
        // Reset next level size
        opencl.writeBuffer(next_level_size_buffer, sizeof(int), &zero);
        
        // Set kernel arguments for level processing
        clSetKernelArg(level_kernel, 0, sizeof(cl_mem), &vertices_buffer);
        clSetKernelArg(level_kernel, 1, sizeof(cl_mem), &edges_buffer);
        clSetKernelArg(level_kernel, 2, sizeof(cl_mem), &current_level_buffer);
        clSetKernelArg(level_kernel, 3, sizeof(cl_mem), &next_level_buffer);
        clSetKernelArg(level_kernel, 4, sizeof(cl_mem), &next_level_size_buffer);
        clSetKernelArg(level_kernel, 5, sizeof(cl_mem), &distances_buffer);
        clSetKernelArg(level_kernel, 6, sizeof(cl_mem), &visited_buffer);
        clSetKernelArg(level_kernel, 7, sizeof(vertex_ID_t), &num_vertices);
        clSetKernelArg(level_kernel, 8, sizeof(int), &current_distance);
        
        // Execute level processing kernel
        // Use a work group size that's a power of 2 and doesn't exceed max work group size
        size_t max_work_group_size = opencl.getMaxWorkGroupSize();
        size_t local_size = 256; // Common choice for GPU kernels
        if (local_size > max_work_group_size) {
            local_size = max_work_group_size;
        }
        
        // Round up global size to be a multiple of local size
        size_t global_size = ((num_vertices + local_size - 1) / local_size) * local_size;
        

        opencl.executeKernel(level_kernel, global_size, local_size);
        
        // Check if destination is reached
        clSetKernelArg(check_kernel, 0, sizeof(cl_mem), &distances_buffer);
        clSetKernelArg(check_kernel, 1, sizeof(cl_mem), &found_buffer);
        clSetKernelArg(check_kernel, 2, sizeof(cl_mem), &result_distance_buffer);
        clSetKernelArg(check_kernel, 3, sizeof(vertex_ID_t), &dest);
        
        opencl.executeKernel(check_kernel, global_size, local_size);
        
        // Check if we found the destination
        int found;
        opencl.readBuffer(found_buffer, sizeof(int), &found);
        

        
        if (found) {
            int result_distance;
            opencl.readBuffer(result_distance_buffer, sizeof(int), &result_distance);
            
            // Cleanup
            opencl.releaseBuffer(vertices_buffer);
            opencl.releaseBuffer(edges_buffer);
            opencl.releaseBuffer(distances_buffer);
            opencl.releaseBuffer(visited_buffer);
            opencl.releaseBuffer(current_level_buffer);
            opencl.releaseBuffer(next_level_buffer);
            opencl.releaseBuffer(next_level_size_buffer);
            opencl.releaseBuffer(found_buffer);
            opencl.releaseBuffer(result_distance_buffer);
            clReleaseKernel(init_kernel);
            clReleaseKernel(level_kernel);
            clReleaseKernel(check_kernel);
            clReleaseProgram(program);
            
            return result_distance;
        }
        
        // Get next level size
        int next_level_size;
        opencl.readBuffer(next_level_size_buffer, sizeof(int), &next_level_size);
        

        
        if (next_level_size == 0) {
            break; // No path exists
        }
        
        // Read next level vertices
        current_level.resize(next_level_size);
        opencl.readBuffer(next_level_buffer, 
                         next_level_size * sizeof(vertex_ID_t), 
                         current_level.data());
        
        current_level_size = next_level_size;
        current_distance++;
    }
    
    // Cleanup
    opencl.releaseBuffer(vertices_buffer);
    opencl.releaseBuffer(edges_buffer);
    opencl.releaseBuffer(distances_buffer);
    opencl.releaseBuffer(visited_buffer);
    opencl.releaseBuffer(current_level_buffer);
    opencl.releaseBuffer(next_level_buffer);
    opencl.releaseBuffer(next_level_size_buffer);
    opencl.releaseBuffer(found_buffer);
    opencl.releaseBuffer(result_distance_buffer);
    clReleaseKernel(init_kernel);
    clReleaseKernel(level_kernel);
    clReleaseKernel(check_kernel);
    clReleaseProgram(program);
    
    throw std::runtime_error("No path exists between source and destination.");
}

// Functor for dispatching templated function via CLI options
struct OpenCLDispatcher {
    int &exit_code;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        vertex_ID_t src = 0;
        vertex_ID_t dest = graph.num_vertices() - 1;
        auto timer = timer_start();
        try {
            long long dist = breadth_first_search_opencl<V, E, G>(graph, src, dest);
            auto time = timer_stop(timer);
            std::cout << "OpenCL BFS returned: " << dist << " in " << time << " seconds" << std::endl;
        }
        catch (std::exception &e) {
            auto time = timer_stop(timer);
            std::cerr << "Caught OpenCL BFS exception: " << e.what() << " in " << time << " seconds" << std::endl;
            exit_code = 1;
        }
    }
};

int main(int argc, char** argv) {
    int exit_code = 0;
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, OpenCLDispatcher{exit_code});
    if (exit_code)
        std::cerr << "Failed with exit code: " << exit_code << std::endl;
    else
        std::cout << "Succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
}
