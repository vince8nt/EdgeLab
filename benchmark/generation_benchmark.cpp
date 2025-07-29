#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include "../src/generator.h"
#include "../src/graph_comp.h"
#include "../src/builder.h"
#include "../src/util.h"

using namespace std;

struct GenerationConfig {
    vertex_ID_t num_vertices;
    edge_ID_t num_edges;
    string name;
    float density;
};

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void benchmark_generation(const GenerationConfig& config, int num_runs = 5) {
    cout << "\n=== Generation Benchmark: " << config.name << " ===" << endl;
    cout << "Vertices: " << config.num_vertices << endl;
    cout << "Edges: " << config.num_edges << endl;
    cout << "Density: " << (config.density * 100) << "%" << endl;
    cout << "Runs: " << num_runs << endl;
    
    vector<double> generation_times;
    vector<double> building_times;
    vector<double> total_times;
    
    for (int run = 0; run < num_runs; ++run) {
        cout << "  Run " << (run + 1) << "/" << num_runs << "... ";
        
        // Time graph generation
        auto start_gen = timer_start();
        // Calculate scale and degree for the generator
        int scale = static_cast<int>(log2(config.num_vertices));
        int degree = static_cast<int>(config.num_edges / config.num_vertices);
        Generator<Vertex_t, Edge_t, Graph_t> generator(GenType::ERDOS_RENYI, scale, degree);
        auto vg = generator.Generate();
        double gen_time_seconds = timer_stop(start_gen);
        double gen_time_milliseconds = gen_time_seconds * 1000.0;
        
        // Time graph building
        auto start_build = timer_start();
        Builder<Vertex_t, Edge_t, Graph_t> builder;
        auto graph = builder.BuildGraph(vg);
        double build_time_seconds = timer_stop(start_build);
        double build_time_milliseconds = build_time_seconds * 1000.0;
        
        double total_time_milliseconds = gen_time_milliseconds + build_time_milliseconds;
        
        generation_times.push_back(gen_time_milliseconds);
        building_times.push_back(build_time_milliseconds);
        total_times.push_back(total_time_milliseconds);
        
        cout << "Done (" << total_time_milliseconds << "ms)" << endl;
    }
    
    // Calculate statistics
    auto calc_stats = [](const vector<double>& times) {
        double sum = 0, min_val = times[0], max_val = times[0];
        for (double t : times) {
            sum += t;
            min_val = min(min_val, t);
            max_val = max(max_val, t);
        }
        double mean = sum / times.size();
        
        double variance = 0;
        for (double t : times) {
            variance += (t - mean) * (t - mean);
        }
        variance /= times.size();
        double std_dev = sqrt(variance);
        
        return make_tuple(mean, std_dev, min_val, max_val);
    };
    
    auto [gen_mean, gen_std, gen_min, gen_max] = calc_stats(generation_times);
    auto [build_mean, build_std, build_min, build_max] = calc_stats(building_times);
    auto [total_mean, total_std, total_min, total_max] = calc_stats(total_times);
    
    cout << "\nResults:" << endl;
    cout << "  Generation:" << endl;
    cout << "    Mean: " << gen_mean << "ms ± " << gen_std << "ms" << endl;
    cout << "    Range: " << gen_min << "ms - " << gen_max << "ms" << endl;
    
    cout << "  Building:" << endl;
    cout << "    Mean: " << build_mean << "ms ± " << build_std << "ms" << endl;
    cout << "    Range: " << build_min << "ms - " << build_max << "ms" << endl;
    
    cout << "  Total:" << endl;
    cout << "    Mean: " << total_mean << "ms ± " << total_std << "ms" << endl;
    cout << "    Range: " << total_min << "ms - " << total_max << "ms" << endl;
    
    // Calculate throughput
    double edges_per_second = (config.num_edges * 1000.0) / total_mean;
    cout << "  Throughput: " << edges_per_second << " edges/second" << endl;
}

void benchmark_different_graph_types() {
    cout << "\n=== Graph Type Comparison ===" << endl;
    
    GenerationConfig config = {
        .num_vertices = 1000,
        .num_edges = 5000,
        .name = "Medium Sparse",
        .density = 0.01f
    };
    
    cout << "Testing unweighted undirected graphs..." << endl;
    benchmark_generation<VertexUW, EdgeUW, GraphType::UNDIRECTED>(config);
    
    cout << "\nTesting weighted undirected graphs..." << endl;
    benchmark_generation<VertexUW, EdgeW, GraphType::UNDIRECTED>(config);
    
    cout << "\nTesting unweighted directed graphs..." << endl;
    benchmark_generation<VertexUW, EdgeUW, GraphType::DIRECTED>(config);
    
    cout << "\nTesting weighted directed graphs..." << endl;
    benchmark_generation<VertexUW, EdgeW, GraphType::DIRECTED>(config);
}

void benchmark_scaling() {
    cout << "\n=== Scaling Benchmark ===" << endl;
    
    vector<GenerationConfig> configs = {
        {100, 500, "Small Sparse", 0.1f},
        {100, 4950, "Small Dense", 1.0f},
        {1000, 5000, "Medium Sparse", 0.01f},
        {1000, 499500, "Medium Dense", 1.0f},
        {10000, 50000, "Large Sparse", 0.001f},
        {10000, 49995000, "Large Dense", 1.0f}
    };
    
    for (const auto& config : configs) {
        benchmark_generation<VertexUW, EdgeUW, GraphType::UNDIRECTED>(config, 3);
    }
}

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void benchmark_memory_usage(const GenerationConfig& config) {
    cout << "\n=== Memory Usage Benchmark ===" << endl;
    cout << "Generating graph: " << config.name << endl;
    
    // Measure memory before
    // Note: This is a simplified measurement. In a real implementation,
    // you might want to use platform-specific memory measurement tools
    
    auto start = timer_start();
    
    // Calculate scale and degree for the generator
    int scale = static_cast<int>(log2(config.num_vertices));
    int degree = static_cast<int>(config.num_edges / config.num_vertices);
    Generator<Vertex_t, Edge_t, Graph_t> generator(GenType::ERDOS_RENYI, scale, degree);
    auto vg = generator.Generate();
    
    Builder<Vertex_t, Edge_t, Graph_t> builder;
    auto graph = builder.BuildGraph(vg);
    
    double duration_seconds = timer_stop(start);
    double duration_milliseconds = duration_seconds * 1000.0;
    
    // Calculate theoretical memory usage
    size_t vertex_memory = config.num_vertices * sizeof(typename Graph<Vertex_t, Edge_t, Graph_t>::Vertex);
    size_t edge_memory = config.num_edges * sizeof(Edge_t);
    size_t total_memory = vertex_memory + edge_memory;
    
    cout << "Generation time: " << duration_milliseconds << "ms" << endl;
    cout << "Theoretical memory usage:" << endl;
    cout << "  Vertices: " << vertex_memory << " bytes (" << vertex_memory / 1024.0 << " KB)" << endl;
    cout << "  Edges: " << edge_memory << " bytes (" << edge_memory / 1024.0 << " KB)" << endl;
    cout << "  Total: " << total_memory << " bytes (" << total_memory / 1024.0 << " KB)" << endl;
    cout << "  Memory per edge: " << (double)total_memory / config.num_edges << " bytes" << endl;
}

int main(int argc, char* argv[]) {
    cout << "EdgeLab Graph Generation Benchmark" << endl;
    cout << "==================================" << endl;
    
    if (argc > 1 && string(argv[1]) == "--help") {
        cout << "Usage: " << argv[0] << " [benchmark_type]" << endl;
        cout << "  benchmark_type:" << endl;
        cout << "    scaling     - Test different graph sizes (default)" << endl;
        cout << "    types       - Compare different graph types" << endl;
        cout << "    memory      - Test memory usage" << endl;
        cout << "    all         - Run all benchmarks" << endl;
        return 0;
    }
    
    string benchmark_type = argc > 1 ? argv[1] : "scaling";
    
    if (benchmark_type == "scaling" || benchmark_type == "all") {
        benchmark_scaling();
    }
    
    if (benchmark_type == "types" || benchmark_type == "all") {
        benchmark_different_graph_types();
    }
    
    if (benchmark_type == "memory" || benchmark_type == "all") {
        GenerationConfig config = {
            .num_vertices = 10000,
            .num_edges = 50000,
            .name = "Large Sparse",
            .density = 0.001f
        };
        benchmark_memory_usage<VertexUW, EdgeUW, GraphType::UNDIRECTED>(config);
    }
    
    cout << "\nBenchmark completed!" << endl;
    return 0;
} 