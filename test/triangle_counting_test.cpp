#include "../src/cli_dispatch.h"
#include "../src/generator.h"
#include "../src/builder.h"
#include "../examples/triangle_counting.h"
#include "../examples_threaded/triangle_counting.h"
#include <iostream>
#include <vector>
#include <cassert>

// Check if OpenCL headers are available
#ifdef OPENCL_AVAILABLE
#include "../examples_opencl/triangle_counting.h"
#define HAS_OPENCL 1
#else
#define HAS_OPENCL 0
#endif

// Test configuration
struct TestConfig {
    int scale;
    int degree;
    std::string name;
};

// Test all triangle counting implementations on a given graph
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
bool test_triangle_counting_implementations(Graph<Vertex_t, Edge_t, Graph_t>& graph, const std::string& test_name) {
    std::cout << "\n=== Testing " << test_name << " ===" << std::endl;
    std::cout << "Graph: " << graph.num_vertices() << " vertices, " << graph.num_edges() << " edges" << std::endl;
    
    // Test CPU implementation
    std::cout << "Running CPU triangle counting..." << std::endl;
    edge_ID_t cpu_result = triangle_counting<Vertex_t, Edge_t, Graph_t>(graph);
    std::cout << "CPU result: " << cpu_result << " triangles" << std::endl;
    
    // Test threaded implementation
    std::cout << "Running threaded triangle counting..." << std::endl;
    edge_ID_t threaded_result = triangle_counting_threaded<Vertex_t, Edge_t, Graph_t>(graph);
    std::cout << "Threaded result: " << threaded_result << " triangles" << std::endl;
    
    // Test OpenCL implementation (if available)
    edge_ID_t opencl_result = -1;
    #if HAS_OPENCL
    try {
        std::cout << "Running OpenCL triangle counting..." << std::endl;
        opencl_result = triangle_counting_opencl<Vertex_t, Edge_t, Graph_t>(graph);
        std::cout << "OpenCL result: " << opencl_result << " triangles" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "OpenCL failed: " << e.what() << std::endl;
    }
    #else
    std::cout << "OpenCL not available (headers not found)" << std::endl;
    #endif
    
    // Verify results match
    bool all_match = true;
    if (cpu_result != threaded_result) {
        std::cerr << "ERROR: CPU (" << cpu_result << ") != Threaded (" << threaded_result << ")" << std::endl;
        all_match = false;
    }
    
    if (opencl_result != static_cast<edge_ID_t>(-1)) {
        if (cpu_result != opencl_result) {
            std::cerr << "ERROR: CPU (" << cpu_result << ") != OpenCL (" << opencl_result << ")" << std::endl;
            all_match = false;
        }
        if (threaded_result != opencl_result) {
            std::cerr << "ERROR: Threaded (" << threaded_result << ") != OpenCL (" << opencl_result << ")" << std::endl;
            all_match = false;
        }
    }
    
    if (all_match) {
        std::cout << "✓ All implementations match: " << cpu_result << " triangles" << std::endl;
    } else {
        std::cout << "✗ Results do not match!" << std::endl;
    }
    
    return all_match;
}

// Generate and test graphs with different configurations
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int run_triangle_counting_tests() {
    std::cout << "Starting Triangle Counting Implementation Tests" << std::endl;
    std::cout << "================================================" << std::endl;
    
    // Test configurations: {scale, degree, name}
    std::vector<TestConfig> test_configs = {
        {6, 3, "Small sparse graph"},
        {8, 5, "Medium graph"},
        {10, 8, "Large dense graph"},
        {12, 12, "Very large dense graph"}
    };
    
    int passed_tests = 0;
    int total_tests = test_configs.size();
    
    for (const auto& config : test_configs) {
        std::cout << "\n" << std::string(50, '-') << std::endl;
        std::cout << "Test: " << config.name << std::endl;
        std::cout << "Scale: " << config.scale << ", Degree: " << config.degree << std::endl;
        
        try {
            // Generate test graph
            Generator<Vertex_t, Edge_t, Graph_t> generator(GenType::ERDOS_RENYI, config.scale, config.degree);
            VectorGraph<Vertex_t, Edge_t> vg = generator.Generate();
            
            // Build CSR Graph
            Builder<Vertex_t, Edge_t, Graph_t> builder;
            Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
            
            // Test all implementations
            bool test_passed = test_triangle_counting_implementations<Vertex_t, Edge_t, Graph_t>(
                graph, config.name);
            
            if (test_passed) {
                passed_tests++;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "ERROR: Failed to generate or test graph: " << e.what() << std::endl;
        }
    }
    
    // Summary
    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "TEST SUMMARY" << std::endl;
    std::cout << std::string(50, '=') << std::endl;
    std::cout << "Passed: " << passed_tests << "/" << total_tests << " tests" << std::endl;
    
    if (passed_tests == total_tests) {
        std::cout << "✓ All triangle counting implementations are consistent!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ Some implementations have inconsistencies!" << std::endl;
        return 1;
    }
}

int main(int argc, char** argv) {
    (void)argc;  // Suppress unused parameter warning
    (void)argv;  // Suppress unused parameter warning
    int exit_code = 0;
    
    // Run tests with default graph type (undirected, unweighted)
    std::cout << "Running triangle counting tests with VertexUW, EdgeUW, UNDIRECTED..." << std::endl;
    exit_code = run_triangle_counting_tests<VertexUW, EdgeUW, GraphType::UNDIRECTED>();
    
    if (exit_code == 0) {
        std::cout << "\nAll tests completed successfully!" << std::endl;
    } else {
        std::cout << "\nSome tests failed!" << std::endl;
    }
    
    return exit_code;
} 