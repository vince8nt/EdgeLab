#ifndef CLI_H_
#define CLI_H_

#include <string>
#include <iostream>
#include <algorithm>
#include <cctype>
#include "graph_comp.h"
#include "generator.h" // for GenType

// Forward declarations for custom types (replace with actual includes in your main file)
struct VertexUW;
struct VertexW;
template<typename T> struct VertexUWD;
template<typename T> struct VertexWD;
struct EdgeUW;
struct EdgeW;
template<typename T> struct EdgeUWD;
template<typename T> struct EdgeWD;
enum class GraphType;
template<NonDataVertexType V, NonDataEdgeType E, GraphType G> class Generator;
template<typename V, typename E, GraphType G> class Builder;
template<typename V, typename E, GraphType G> class Graph;
template<typename V, typename E> struct VectorGraph;

// Enum definitions for CLI
enum class CLIGraphType {
    UNDIRECTED,
    DIRECTED
};

enum class CLIVertexType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA
};

enum class CLIEdgeType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA
};

struct CLIOptions {
    CLIGraphType graph_type = CLIGraphType::UNDIRECTED;
    CLIVertexType vertex_type = CLIVertexType::UNWEIGHTED;
    CLIEdgeType edge_type = CLIEdgeType::UNWEIGHTED;
    int scale;
    int degree;
    GenType gen_type;
};

inline std::string to_lower(const std::string& s) {
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

inline void print_usage(const char* prog_name) {
    std::cout << "Usage: " << prog_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --graph-type <directed|undirected>      (default: undirected)\n";
    std::cout << "  --vertex-type <unweighted|weighted|unweighted_data|weighted_data>  (default: unweighted)\n";
    std::cout << "  --edge-type <unweighted|weighted|unweighted_data|weighted_data>    (default: unweighted)\n";
    std::cout << "  --scale <int>                          (required)\n";
    std::cout << "  --degree <int>                         (required)\n";
    std::cout << "  --gen-type <erdos_renyi|watts_strogatz|barabasi_albert> (required)\n";
    std::cout << std::endl;
}

inline bool parse_enum(const std::string& value, CLIGraphType& out) {
    std::string v = to_lower(value);
    if (v == "undirected") { out = CLIGraphType::UNDIRECTED; return true; }
    if (v == "directed")   { out = CLIGraphType::DIRECTED;   return true; }
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
inline GraphType to_graph_type(CLIGraphType t) {
    return (t == CLIGraphType::UNDIRECTED) ? GraphType::UNDIRECTED : GraphType::DIRECTED;
}

// Parse CLI arguments and return CLIOptions. Prints usage and exits on error.
inline CLIOptions parse_cli(int argc, char** argv) {
    CLIOptions opts{};
    bool got_scale = false, got_degree = false, got_gen_type = false;
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


// Generic type dispatcher for any test program
// Usage:
//   dispatch_types(opts, MyFunctor{opts});
//   or with a lambda (C++20):
//   dispatch_types(opts, [&](auto Vertex_t, auto Edge_t, auto G) { ... });
//
// The callable must have a templated operator() or be a generic lambda.

template <typename Callable>
void dispatch_types(const CLIOptions& opts, Callable&& func) {
    switch (opts.vertex_type) {
        case CLIVertexType::UNWEIGHTED:
            switch (opts.edge_type) {
                case CLIEdgeType::UNWEIGHTED:
                    switch (to_graph_type(opts.graph_type)) {
                        case GraphType::UNDIRECTED:
                            func.template operator()<VertexUW, EdgeUW, GraphType::UNDIRECTED>();
                            break;
                        case GraphType::DIRECTED:
                            func.template operator()<VertexUW, EdgeUW, GraphType::DIRECTED>();
                            break;
                    }
                    break;
                case CLIEdgeType::WEIGHTED:
                    switch (to_graph_type(opts.graph_type)) {
                        case GraphType::UNDIRECTED:
                            func.template operator()<VertexUW, EdgeW, GraphType::UNDIRECTED>();
                            break;
                        case GraphType::DIRECTED:
                            func.template operator()<VertexUW, EdgeW, GraphType::DIRECTED>();
                            break;
                    }
                    break;
                default:
                    std::cerr << "[EdgeLab CLI] Only unweighted/weighted edge types are supported in dispatch_types.\n";
                    break;
            }
            break;
        case CLIVertexType::WEIGHTED:
            switch (opts.edge_type) {
                case CLIEdgeType::UNWEIGHTED:
                    switch (to_graph_type(opts.graph_type)) {
                        case GraphType::UNDIRECTED:
                            func.template operator()<VertexW, EdgeUW, GraphType::UNDIRECTED>();
                            break;
                        case GraphType::DIRECTED:
                            func.template operator()<VertexW, EdgeUW, GraphType::DIRECTED>();
                            break;
                    }
                    break;
                case CLIEdgeType::WEIGHTED:
                    switch (to_graph_type(opts.graph_type)) {
                        case GraphType::UNDIRECTED:
                            func.template operator()<VertexW, EdgeW, GraphType::UNDIRECTED>();
                            break;
                        case GraphType::DIRECTED:
                            func.template operator()<VertexW, EdgeW, GraphType::DIRECTED>();
                            break;
                    }
                    break;
                default:
                    std::cerr << "[EdgeLab CLI] Only unweighted/weighted edge types are supported in dispatch_types.\n";
                    break;
            }
            break;
        default:
            std::cerr << "[EdgeLab CLI] Only unweighted/weighted vertex types are supported in dispatch_types.\n";
            break;
    }
}

#endif // CLI_H_


