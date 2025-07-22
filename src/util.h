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

// Graph structure specifications
using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = float;

// timer for benchmarking
std::chrono::steady_clock::time_point timer_start() { return std::chrono::steady_clock::now(); }
double timer_stop(const std::chrono::steady_clock::time_point &start) {
    auto end = std::chrono::steady_clock::now(); // Capture end time
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
}

// specification of graph types
enum class GraphType {
    UNDIRECTED,
    DIRECTED
};
std::ostream& operator<<(std::ostream& os, GraphType Graph_t) {
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
