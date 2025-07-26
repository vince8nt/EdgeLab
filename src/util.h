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

// Forward declaration
class Loader;

// Graph capacity specifications
using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = float;

// timer for benchmarking
std::chrono::steady_clock::time_point timer_start();
double timer_stop(const std::chrono::steady_clock::time_point &start);

// specification of graph types
enum class GraphType {
    UNDIRECTED,
    DIRECTED
};
std::ostream& operator<<(std::ostream& os, GraphType Graph_t);

// specification of generation types
enum class GenType {
    ERDOS_RENYI,     // Erdos-Renyi-Gilbert
    WATTS_STROGATZ,  // Watts-Strogatz
    BARABASI_ALBERT, // Barabasi-Albert
};
std::ostream& operator<<(std::ostream& os, GenType Gen_t);

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
