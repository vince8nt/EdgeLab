#include "../src/graph_test.h"
#include "../examples/breadth_first_search.h"
#include "../examples_threaded/breadth_first_search.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <random>

// Check if OpenCL headers are available
#ifdef OPENCL_AVAILABLE
#include "../examples_opencl/breadth_first_search.h"
#define HAS_OPENCL 1
#else
#define HAS_OPENCL 0
#endif

// Test configuration
struct TestConfig {
    int scale;
    int degree;
    std::string name;
    uint32_t seed;  // For deterministic random vertex selection
};

// Test all BFS implementations on a given graph with specific source/dest
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
bool test_bfs_implementations(Graph<Vertex_t, Edge_t, Graph_t>& graph, 
                             vertex_ID_t src, vertex_ID_t dest, 
                             const std::string& test_name) {
    std::cout << "\n=== Testing " << test_name << " ===" << std::endl;
    std::cout << "Graph: " << graph.num_vertices() << " vertices, " << graph.num_edges() << " edges" << std::endl;
    std::cout << "Source: " << src << ", Destination: " << dest << std::endl;
    
    // Test CPU implementation
    std::cout << "Running CPU BFS..." << std::endl;
    long long cpu_result = breadth_first_search<Vertex_t, Edge_t, Graph_t>(graph, src, dest);
    std::cout << "CPU result: " << cpu_result << " (distance)" << std::endl;
    
    // Test threaded implementation
    std::cout << "Running threaded BFS..." << std::endl;
    long long threaded_result = breadth_first_search_threaded<Vertex_t, Edge_t, Graph_t>(graph, src, dest);
    std::cout << "Threaded result: " << threaded_result << " (distance)" << std::endl;
    
    // Test OpenCL implementation (if available)
    long long opencl_result = -2;  // Use -2 to distinguish from -1 (no path)
    #if HAS_OPENCL
    try {
        std::cout << "Running OpenCL BFS..." << std::endl;
        opencl_result = breadth_first_search_opencl<Vertex_t, Edge_t, Graph_t>(graph, src, dest);
        std::cout << "OpenCL result: " << opencl_result << " (distance)" << std::endl;
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
    
    if (opencl_result != -2) {
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
        if (cpu_result == -1) {
            std::cout << "✓ All implementations agree: No path exists" << std::endl;
        } else {
            std::cout << "✓ All implementations match: " << cpu_result << " (distance)" << std::endl;
        }
    } else {
        std::cout << "✗ Results do not match!" << std::endl;
    }
    
    return all_match;
}

// Generate deterministic random source and destination vertices
std::vector<std::pair<vertex_ID_t, vertex_ID_t>> generate_test_pairs(vertex_ID_t num_vertices, uint32_t seed, int num_pairs = 3) {
    std::mt19937 gen(seed);
    std::uniform_int_distribution<vertex_ID_t> vertex_dist(0, num_vertices - 1);
    
    std::vector<std::pair<vertex_ID_t, vertex_ID_t>> pairs;
    for (int i = 0; i < num_pairs; ++i) {
        vertex_ID_t src = vertex_dist(gen);
        vertex_ID_t dest = vertex_dist(gen);
        
        // Ensure src != dest for more interesting tests
        while (src == dest) {
            dest = vertex_dist(gen);
        }
        
        pairs.push_back({src, dest});
    }
    
    return pairs;
}

// Generate and test graphs with different configurations
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int run_bfs_tests() {
    std::cout << "Starting Breadth-First Search Implementation Tests" << std::endl;
    std::cout << "==================================================" << std::endl;
    
    // Test configurations: {scale, degree, name, seed}
    std::vector<TestConfig> test_configs = {
        {6, 3, "Small sparse graph", 12345},
        {8, 5, "Medium graph", 23456},
        {10, 8, "Large dense graph", 34567},
        {14, 3, "Very large sparse graph", 45678}
    };
    
    int passed_tests = 0;
    int total_tests = 0;
    
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
            
            // Generate test vertex pairs
            auto test_pairs = generate_test_pairs(graph.num_vertices(), config.seed, 3);
            
            std::cout << "Testing " << test_pairs.size() << " source-destination pairs..." << std::endl;
            
            // Test each pair
            for (size_t i = 0; i < test_pairs.size(); ++i) {
                auto [src, dest] = test_pairs[i];
                std::string pair_name = config.name + " (pair " + std::to_string(i + 1) + ")";
                
                bool test_passed = test_bfs_implementations<Vertex_t, Edge_t, Graph_t>(
                    graph, src, dest, pair_name);
                
                if (test_passed) {
                    passed_tests++;
                }
                total_tests++;
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
        std::cout << "✓ All BFS implementations are consistent!" << std::endl;
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
    std::cout << "Running BFS tests with VertexUW, EdgeUW, UNDIRECTED..." << std::endl;
    exit_code = run_bfs_tests<VertexUW, EdgeUW, GraphType::UNDIRECTED>();
    
    if (exit_code == 0) {
        std::cout << "\nAll tests completed successfully!" << std::endl;
    } else {
        std::cout << "\nSome tests failed!" << std::endl;
    }
    
    return exit_code;
} 