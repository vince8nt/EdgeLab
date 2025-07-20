#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <concepts> // For std::derived_from


using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = float;

// specification of graph types
enum class GraphType {
    UNDIRECTED,
    DIRECTED
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
