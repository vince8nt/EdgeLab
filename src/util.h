#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <concepts>
#include <chrono>
#include <memory>

#define DEBUG 1 // 1 for debug, 0 for release

// Forward declaration
class Loader;

// Graph capacity specifications
using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = float;

// timer for benchmarking
inline std::chrono::steady_clock::time_point timer_start() { 
    return std::chrono::steady_clock::now(); 
}

inline double timer_stop(const std::chrono::steady_clock::time_point &start) {
    auto end = std::chrono::steady_clock::now(); // Capture end time
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
}

// specification of graph types
enum class GraphType {
    UNDIRECTED,
    DIRECTED
};

inline std::ostream& operator<<(std::ostream& os, GraphType Graph_t) {
    switch (Graph_t) {
        case GraphType::UNDIRECTED:
            os << "Undirected";
            break;
        case GraphType::DIRECTED:
            os << "Directed";
            break;
        default:
            os << "Unknown Graph Type";
            break;
    }
    return os;
}

// specification of generation types
enum class GenType {
    ERDOS_RENYI,     // Erdos-Renyi-Gilbert
    WATTS_STROGATZ,  // Watts-Strogatz
    BARABASI_ALBERT, // Barabasi-Albert
};

inline std::ostream& operator<<(std::ostream& os, GenType Gen_t) {
    switch (Gen_t) {
        case GenType::ERDOS_RENYI:
            os << "Erdos-Renyi";
            break;
        case GenType::WATTS_STROGATZ:
            os << "Watts-Strogatz";
            break;
        case GenType::BARABASI_ALBERT:
            os << "Barabasi-Albert";
            break;
        default:
            os << "Unknown Generation Type";
            break;
    }
    return os;
}

// Enum definitions for CLI
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
    GraphType graph_type = GraphType::UNDIRECTED;
    CLIVertexType vertex_type = CLIVertexType::UNWEIGHTED;
    CLIEdgeType edge_type = CLIEdgeType::UNWEIGHTED;
    int scale;
    int degree;
    GenType gen_type;
    std::string load_file_path; // Path to load file, mutually exclusive
    std::unique_ptr<Loader> loader;
};

/*
enum class VertexType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA,
};
enum class EdgeType {
    UNWEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED,
    WEIGHTED_DATA
};
*/


#endif // UTIL_H_
