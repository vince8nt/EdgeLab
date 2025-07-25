#ifndef CLI_H_
#define CLI_H_

#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <utility> // For std::forward
#include "util.h"
#include "graph_comp.h"
#include "generator.h" // for GenType
#include "loader.h"



inline std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

inline void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --load-file <path>                      (mutually exclusive with all other options)\n";
    std::cout << "  --graph-type <directed|undirected>      (default: undirected)\n";
    std::cout << "  --vertex-type <unweighted|weighted|unweighted_data|weighted_data>  (default: unweighted)\n";
    std::cout << "  --edge-type <unweighted|weighted|unweighted_data|weighted_data>    (default: unweighted)\n";
    std::cout << "  --scale <int>                          (required)\n";
    std::cout << "  --degree <int>                         (required)\n";
    std::cout << "  --gen-type <erdos_renyi|watts_strogatz|barabasi_albert> (required)\n";
    std::cout << std::endl;
}

inline bool parse_enum(const std::string& value, GraphType& out) {
    std::string v = to_lower(value);
    if (v == "undirected") { out = GraphType::UNDIRECTED; return true; }
    if (v == "directed")   { out = GraphType::DIRECTED;   return true; }
    return false;
}
inline bool parse_enum(const std::string& value, CLIVertexType& out) {
    std::string v = to_lower(value);
    if (v == "unweighted")        { out = CLIVertexType::UNWEIGHTED;        return true; }
    if (v == "weighted")          { out = CLIVertexType::WEIGHTED;          return true; }
    if (v == "unweighted_data")   { out = CLIVertexType::UNWEIGHTED_DATA;   return true; }
    if (v == "weighted_data")     { out = CLIVertexType::WEIGHTED_DATA;     return true; }
    return false;
}
inline bool parse_enum(const std::string& value, CLIEdgeType& out) {
    std::string v = to_lower(value);
    if (v == "unweighted")        { out = CLIEdgeType::UNWEIGHTED;        return true; }
    if (v == "weighted")          { out = CLIEdgeType::WEIGHTED;          return true; }
    if (v == "unweighted_data")   { out = CLIEdgeType::UNWEIGHTED_DATA;   return true; }
    if (v == "weighted_data")     { out = CLIEdgeType::WEIGHTED_DATA;     return true; }
    return false;
}
inline bool parse_enum(const std::string& value, GenType& out) {
    std::string v = to_lower(value);
    if (v == "erdos_renyi")       { out = GenType::ERDOS_RENYI;       return true; }
    if (v == "watts_strogatz")    { out = GenType::WATTS_STROGATZ;    return true; }
    if (v == "barabasi_albert")   { out = GenType::BARABASI_ALBERT;   return true; }
    return false;
}

// Map CLI enums to internal enums
// enum class GraphType { UNDIRECTED, DIRECTED };
// enum class GenType { ERDOS_RENYI, WATTS_STROGATZ, BARABASI_ALBERT };

// Parse CLI arguments and return CLIOptions. Prints usage and exits on error.
inline CLIOptions parse_cli(int argc, char** argv) {
    CLIOptions opts{};
    bool got_scale = false, got_degree = false, got_gen_type = false;
    bool got_load_file = false;
    // int load_file_arg_index = -1;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--load-file" && i+1 < argc) {
            if (got_load_file) {
                std::cerr << "Duplicate --load-file option." << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
            opts.load_file_path = argv[++i];
            got_load_file = true;
            // load_file_arg_index = i - 1;
        }
    }
    if (got_load_file) {
        // Only --load-file and program name are allowed
        if (argc != 3) {
            std::cerr << "--load-file is mutually exclusive with all other options." << std::endl;
            print_usage(argv[0]);
            exit(1);
        }
        return opts;
    }
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--graph-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.graph_type)) {
                std::cerr << "Invalid graph type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "--vertex-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.vertex_type)) {
                std::cerr << "Invalid vertex type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "--edge-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.edge_type)) {
                std::cerr << "Invalid edge type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
        } else if (arg == "--scale" && i+1 < argc) {
            opts.scale = std::stoi(argv[++i]);
            got_scale = true;
        } else if (arg == "--degree" && i+1 < argc) {
            opts.degree = std::stoi(argv[++i]);
            got_degree = true;
        } else if (arg == "--gen-type" && i+1 < argc) {
            if (!parse_enum(argv[++i], opts.gen_type)) {
                std::cerr << "Invalid gen type: " << argv[i] << std::endl;
                print_usage(argv[0]);
                exit(1);
            }
            got_gen_type = true;
        } else {
            std::cerr << "Unknown or incomplete option: " << arg << std::endl;
            print_usage(argv[0]);
            exit(1);
        }
    }
    if (!(got_scale && got_degree && got_gen_type)) {
        std::cerr << "Missing required options." << std::endl;
        print_usage(argv[0]);
        exit(1);
    }
    return opts;
}

// Generate and build Graph via CLI generator options
template <typename Callable, typename V, typename E, GraphType G>
void CLI_create_graph (const CLIOptions& opts, Callable&& func) {
    // generate edge list
    Generator<V, E, G> generator(opts.gen_type, opts.scale, opts.degree);
    VectorGraph<V, E> vg = generator.Generate();

    // generaate CLI Graph
    Builder<V, E, G> builder;
    Graph<V, E, G> graph = builder.BuildGraph(vg);

    // template and call func with graph
    func.template operator()<V, E, G>(graph);
}

#endif // CLI_H_


