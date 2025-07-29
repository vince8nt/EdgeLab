#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>

#include "../src/cli_dispatch.h"
#include "../src/generator.h"
#include "../src/graph_comp.h"
#include "../src/builder.h"
#include "../src/loader.h"
#include "../src/util.h"
#include "../examples_threaded/triangle_counting.h"

using namespace std;

struct BenchmarkResult {
    int num_threads;
    double mean_time;
    double std_time;
    double min_time;
    double max_time;
    edge_ID_t result;
};

struct GraphConfig {
    string name;
    string type; // "file" or "generated"
    string filepath;
    int scale;
    int degree;
    GenType gen_type;
    CLIEdgeType edge_type;
    CLIVertexType vertex_type;
    GraphType graph_type;
};

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
Graph<Vertex_t, Edge_t, Graph_t> load_or_generate_graph(const GraphConfig& config) {
    if (config.type == "file") {
        Loader loader;
        CLIOptions opts;
        opts.load_file_path = config.filepath;
        loader.load_graph_header(opts);
        return loader.LoadGraphBody<Vertex_t, Edge_t, Graph_t>();
    } else {
        // Generate graph
        Generator<Vertex_t, Edge_t, Graph_t> generator(
            config.gen_type, config.scale, config.degree);
        auto vg = generator.Generate();
        Builder<Vertex_t, Edge_t, Graph_t> builder;
        return builder.BuildGraph(vg);
    }
}

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
BenchmarkResult benchmark_tc_threaded(const GraphConfig& config, 
                                     const vector<int>& thread_counts, 
                                     int num_runs = 3) {
    cout << "\n=== Triangle Counting Threaded Benchmark: " << config.name << " ===" << endl;
    
    // Load or generate the graph
    cout << "Loading/generating graph..." << endl;
    auto graph = load_or_generate_graph<Vertex_t, Edge_t, Graph_t>(config);
    cout << "Graph loaded: " << graph.num_vertices() << " vertices, " 
         << graph.num_edges() << " edges" << endl;
    
    vector<BenchmarkResult> results;
    
    for (int num_threads : thread_counts) {
        cout << "\nTesting with " << num_threads << " threads..." << endl;
        
        vector<double> times;
        edge_ID_t last_result = 0;
        
        for (int run = 0; run < num_runs; ++run) {
            cout << "  Run " << (run + 1) << "/" << num_runs << "... ";
            
            auto start = chrono::high_resolution_clock::now();
            edge_ID_t result = triangle_counting_threaded(graph, num_threads);
            auto end = chrono::high_resolution_clock::now();
            
            double time_ms = chrono::duration<double, milli>(end - start).count();
            times.push_back(time_ms);
            last_result = result;
            
            cout << "Done (" << time_ms << "ms, triangles: " << result << ")" << endl;
        }
        
        // Calculate statistics
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
        
        results.push_back({num_threads, mean, std_dev, min_val, max_val, last_result});
    }
    
    // Print summary
    cout << "\nResults Summary:" << endl;
    cout << setw(8) << "Threads" << setw(12) << "Mean (ms)" << setw(12) << "Std (ms)" 
         << setw(12) << "Min (ms)" << setw(12) << "Max (ms)" << setw(15) << "Triangles" << endl;
    cout << string(75, '-') << endl;
    
    for (const auto& result : results) {
        cout << setw(8) << result.num_threads 
             << setw(12) << fixed << setprecision(2) << result.mean_time
             << setw(12) << fixed << setprecision(2) << result.std_time
             << setw(12) << fixed << setprecision(2) << result.min_time
             << setw(12) << fixed << setprecision(2) << result.max_time
             << setw(15) << result.result << endl;
    }
    
    // Return the result for the highest thread count (usually most representative)
    return results.back();
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <config_type> [options]" << endl;
        cout << "Config types:" << endl;
        cout << "  file <filepath>                    - Test on file-based graph" << endl;
        cout << "  generated <scale> <degree> <type>  - Test on generated graph" << endl;
        cout << "  all                                - Test on all default configurations" << endl;
        cout << endl;
        cout << "Thread counts: 1, 2, 4, 8, 16 (default)" << endl;
        cout << "Generation types: erdos_renyi, watts_strogatz, barabasi_albert" << endl;
        return 1;
    }
    
    string config_type = argv[1];
    vector<int> thread_counts = {1, 2, 4, 8, 16};
    int num_runs = 3;
    
    // Parse additional arguments
    for (int i = 2; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "--threads") {
            thread_counts.clear();
            while (++i < argc && argv[i][0] != '-') {
                thread_counts.push_back(stoi(argv[i]));
            }
            --i; // Adjust for the loop increment
        } else if (arg == "--runs") {
            num_runs = stoi(argv[++i]);
        }
    }
    
    vector<GraphConfig> configs;
    
    if (config_type == "file" && argc >= 3) {
        configs.push_back({
            "File: " + string(argv[2]),
            "file",
            argv[2],
            0, 0, GenType::ERDOS_RENYI, CLIEdgeType::UNWEIGHTED, CLIVertexType::UNWEIGHTED, GraphType::UNDIRECTED
        });
    } else if (config_type == "generated" && argc >= 5) {
        int scale = stoi(argv[2]);
        int degree = stoi(argv[3]);
        string gen_type_str = argv[4];
        
        GenType gen_type;
        if (gen_type_str == "erdos_renyi") gen_type = GenType::ERDOS_RENYI;
        else if (gen_type_str == "watts_strogatz") gen_type = GenType::WATTS_STROGATZ;
        else if (gen_type_str == "barabasi_albert") gen_type = GenType::BARABASI_ALBERT;
        else {
            cout << "Unknown generation type: " << gen_type_str << endl;
            return 1;
        }
        
        configs.push_back({
            "Generated: scale=" + to_string(scale) + ", degree=" + to_string(degree) + ", " + gen_type_str,
            "generated",
            "",
            scale, degree, gen_type, CLIEdgeType::UNWEIGHTED, CLIVertexType::UNWEIGHTED, GraphType::UNDIRECTED
        });
    } else if (config_type == "all") {
        // Default configurations
        configs.push_back({
            "File: mini.el",
            "file",
            "../graphs/mini.el",
            0, 0, GenType::ERDOS_RENYI, CLIEdgeType::UNWEIGHTED, CLIVertexType::UNWEIGHTED, GraphType::UNDIRECTED
        });
        configs.push_back({
            "Generated: scale=8, degree=4, erdos_renyi",
            "generated",
            "",
            8, 4, GenType::ERDOS_RENYI, CLIEdgeType::UNWEIGHTED, CLIVertexType::UNWEIGHTED, GraphType::UNDIRECTED
        });
        configs.push_back({
            "Generated: scale=12, degree=4, erdos_renyi",
            "generated",
            "",
            12, 4, GenType::ERDOS_RENYI, CLIEdgeType::UNWEIGHTED, CLIVertexType::UNWEIGHTED, GraphType::UNDIRECTED
        });
        configs.push_back({
            "Generated: scale=16, degree=4, erdos_renyi",
            "generated",
            "",
            16, 4, GenType::ERDOS_RENYI, CLIEdgeType::UNWEIGHTED, CLIVertexType::UNWEIGHTED, GraphType::UNDIRECTED
        });
    } else {
        cout << "Invalid configuration type or missing arguments." << endl;
        return 1;
    }
    
    cout << "Triangle Counting Threaded Benchmark" << endl;
    cout << "Thread counts: ";
    for (size_t i = 0; i < thread_counts.size(); ++i) {
        if (i > 0) cout << ", ";
        cout << thread_counts[i];
    }
    cout << endl;
    cout << "Runs per configuration: " << num_runs << endl;
    
    vector<BenchmarkResult> all_results;
    
    for (const auto& config : configs) {
        try {
            auto result = benchmark_tc_threaded<VertexUW, EdgeUW, GraphType::UNDIRECTED>(
                config, thread_counts, num_runs);
            all_results.push_back(result);
        } catch (const exception& e) {
            cout << "Error benchmarking " << config.name << ": " << e.what() << endl;
        }
    }
    
    cout << "\n=== Overall Summary ===" << endl;
    cout << "Best performance per configuration:" << endl;
    for (size_t i = 0; i < configs.size() && i < all_results.size(); ++i) {
        cout << configs[i].name << ": " << all_results[i].mean_time << "ms ± " 
             << all_results[i].std_time << "ms (" << all_results[i].num_threads << " threads, "
             << all_results[i].result << " triangles)" << endl;
    }
    
    return 0;
} 