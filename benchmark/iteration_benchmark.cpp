#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include "../src/cli_dispatch.h"
#include "../src/graph.h"
#include "../src/graph_comp.h"
#include "../src/util.h"

using namespace std;

enum class IterationDirection {
    FORWARD,
    BACKWARD,
    RANDOM
};

// Functor to run benchmarks
struct BenchmarkFunctor {
    IterationDirection direction;
    int num_iterations;
    
    BenchmarkFunctor(IterationDirection dir, int iter) 
        : direction(dir), num_iterations(iter) {}
    
    template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
    void operator()(Graph<Vertex_t, Edge_t, Graph_t>& graph) {
        cout << "Graph loaded: " << graph.num_vertices() << " vertices, " 
             << graph.num_edges() << " edges" << endl;
        
        // Run iteration benchmark
        benchmark_iteration(graph, direction, num_iterations);
        
        // Run iterator vs indexing benchmark
        benchmark_iterator_vs_indexing(graph);
        
        // Run memory access pattern benchmark
        benchmark_memory_access_patterns(graph);
    }
};

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void benchmark_iteration(const Graph<Vertex_t, Edge_t, Graph_t>& graph, 
                        IterationDirection direction, int num_iterations = 1000) {
    auto start = timer_start();
    
    size_t total_edges = 0;
    
    for (int iter = 0; iter < num_iterations; ++iter) {
        switch (direction) {
            case IterationDirection::FORWARD: {
                // Forward iteration through vertices
                for (vertex_ID_t v = 0; v < graph.num_vertices(); ++v) {
                    for (const auto& edge : graph[v]) {
                        total_edges += edge.dest();
                    }
                }
                break;
            }
            case IterationDirection::BACKWARD: {
                // Backward iteration through vertices
                for (vertex_ID_t v = graph.num_vertices() - 1; v < graph.num_vertices(); --v) {
                    for (const auto& edge : graph[v]) {
                        total_edges += edge.dest();
                    }
                }
                break;
            }
            case IterationDirection::RANDOM: {
                // Random vertex access
                random_device rd;
                mt19937 gen(rd());
                uniform_int_distribution<vertex_ID_t> dis(0, graph.num_vertices() - 1);
                
                for (int i = 0; i < graph.num_vertices(); ++i) {
                    vertex_ID_t v = dis(gen);
                    for (const auto& edge : graph[v]) {
                        total_edges += edge.dest();
                    }
                }
                break;
            }
        }
    }
    
    double duration_seconds = timer_stop(start);
    double duration_microseconds = duration_seconds * 1000000.0;
    
    cout << "Iteration benchmark completed:" << endl;
    cout << "  Direction: " << (direction == IterationDirection::FORWARD ? "FORWARD" : 
                               direction == IterationDirection::BACKWARD ? "BACKWARD" : "RANDOM") << endl;
    cout << "  Iterations: " << num_iterations << endl;
    cout << "  Total time: " << duration_microseconds << " microseconds" << endl;
    cout << "  Average time per iteration: " << duration_microseconds / num_iterations << " microseconds" << endl;
    cout << "  Total edges accessed: " << total_edges << endl;
}

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void benchmark_iterator_vs_indexing(const Graph<Vertex_t, Edge_t, Graph_t>& graph) {
    cout << "\n=== Iterator vs Indexing Benchmark ===" << endl;
    
    // Iterator-based iteration
    auto start = timer_start();
    size_t iterator_sum = 0;
    for (vertex_ID_t v = 0; v < graph.num_vertices(); ++v) {
        for (const auto& edge : graph[v]) {
            iterator_sum += edge.dest();
        }
    }
    double iterator_time_seconds = timer_stop(start);
    double iterator_time_microseconds = iterator_time_seconds * 1000000.0;
    
    // Indexing-based iteration
    start = timer_start();
    size_t indexing_sum = 0;
    for (vertex_ID_t v = 0; v < graph.num_vertices(); ++v) {
        for (edge_ID_t e = 0; e < graph[v].degree(); ++e) {
            indexing_sum += graph[v][e].dest();
        }
    }
    double indexing_time_seconds = timer_stop(start);
    double indexing_time_microseconds = indexing_time_seconds * 1000000.0;
    
    cout << "Iterator-based iteration:" << endl;
    cout << "  Time: " << iterator_time_microseconds << " microseconds" << endl;
    cout << "  Sum: " << iterator_sum << endl;
    
    cout << "Indexing-based iteration:" << endl;
    cout << "  Time: " << indexing_time_microseconds << " microseconds" << endl;
    cout << "  Sum: " << indexing_sum << endl;
    
    if (iterator_sum == indexing_sum) {
        cout << "  Results match ✓" << endl;
    } else {
        cout << "  Results differ ✗" << endl;
    }
    
    double speedup = indexing_time_microseconds / iterator_time_microseconds;
    cout << "  Iterator speedup: " << speedup << "x" << endl;
}

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void benchmark_memory_access_patterns(const Graph<Vertex_t, Edge_t, Graph_t>& graph) {
    cout << "\n=== Memory Access Pattern Benchmark ===" << endl;
    
    // Sequential access
    auto start = timer_start();
    size_t sequential_sum = 0;
    for (vertex_ID_t v = 0; v < graph.num_vertices(); ++v) {
        for (const auto& edge : graph[v]) {
            sequential_sum += edge.dest();
        }
    }
    double sequential_time_seconds = timer_stop(start);
    double sequential_time_microseconds = sequential_time_seconds * 1000000.0;
    
    // Strided access (every 10th vertex)
    start = timer_start();
    size_t strided_sum = 0;
    for (vertex_ID_t v = 0; v < graph.num_vertices(); v += 10) {
        for (const auto& edge : graph[v]) {
            strided_sum += edge.dest();
        }
    }
    double strided_time_seconds = timer_stop(start);
    double strided_time_microseconds = strided_time_seconds * 1000000.0;
    
    cout << "Sequential access:" << endl;
    cout << "  Time: " << sequential_time_microseconds << " microseconds" << endl;
    cout << "  Sum: " << sequential_sum << endl;
    
    cout << "Strided access (every 10th vertex):" << endl;
    cout << "  Time: " << strided_time_microseconds << " microseconds" << endl;
    cout << "  Sum: " << strided_sum << endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <graph_file> [direction] [iterations]" << endl;
        cerr << "  direction: forward, backward, random (default: forward)" << endl;
        cerr << "  iterations: number of iterations (default: 1000)" << endl;
        return 1;
    }
    
    string graph_file = argv[1];
    IterationDirection direction = IterationDirection::FORWARD;
    int num_iterations = 1000;
    
    if (argc > 2) {
        string dir_str = argv[2];
        if (dir_str == "backward") {
            direction = IterationDirection::BACKWARD;
        } else if (dir_str == "random") {
            direction = IterationDirection::RANDOM;
        }
    }
    
    if (argc > 3) {
        num_iterations = stoi(argv[3]);
    }
    
    cout << "Loading graph from: " << graph_file << endl;
    
    // Load the graph using the existing infrastructure
    CLIOptions opts;
    opts.load_file_path = graph_file;
    
    BenchmarkFunctor benchmark_func(direction, num_iterations);
    dispatch_cli(opts, benchmark_func);
    
    return 0;
} 