#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <vector>
#include <iostream>
#include <type_traits>
#include <algorithm>


using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = float;

// specification of graph types
enum class GraphType {
    UNDIRECTED,
    DIRECTED
};
enum class VertexType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA,
    UNWEIGHTED_DATAREF,
    WEIGHTED_DATAREF,
    UNWEIGHTED_DATA_DATAREF,
    WEIGHTED_DATA_DATAREF
};
enum class EdgeType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_ID,
    WEIGHTED_ID
};


// Adjacency containing only destination ID (used for CSR)
struct Adjacency {
    vertex_ID_t dest_;    
};
struct AdjacencyW : public Adjacency {
   weight_t weight_;
};
template<typename Adjacency_t> // unique ID for each edge 
struct AdjacencyID : public Adjacency_t { // usually not useful since we can just use (sourceID, destID)
    edge_ID_t ID_;
};

// Adjacency list/matrix (used when making CSR)
template<typename Adjacency_t>
using AdjacencyList = std::vector<Adjacency_t>;
template<typename Adjacency_t>
using AdjacencyMatrix = std::vector<AdjacencyList<Adjacency_t>>;


// Directed edges contianing both source and destination IDs (used by generator and when loading edgelist files)
template<typename Adjacency_t>
struct Edge {
    const vertex_ID_t source_;
    const Adjacency_t adjacency_;
};
template<typename Adjacency_t>
using EdgeList = std::vector<Edge<Adjacency_t>>;


// Vertex Stats (excluding adjacency data)
struct VertexStats {}; // base class
struct VertexStatsW : public VertexStats {
    const weight_t weight_;
};
template<typename VertexStats_t, typename Data_t> // include arbitrary mutable data in place
struct VertexStatsData : public VertexStats_t {   // good for small/frequently accessed data
    Data_t data_;
};
template<typename VertexStats_t, typename Data_t>  // include arbitrary mutable data reference
struct VertexStatsDataRef : public VertexStats_t { // good for large/infrequently accessed data
    Data_t &data_ref_;
};


// Vertex containing all data (currently unused)
template<typename VertexStats_t, typename Adjacency_t>
struct VertexComplete {
    const vertex_ID_t ID_;
    const vertex_ID_t degree_;
    const AdjacencyList<Adjacency_t> adjacencies_;
    VertexStats_t stats_;
};


#endif // UTIL_H_
