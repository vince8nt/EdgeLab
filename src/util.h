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
    vertex_ID_t dest() const { return dest_; }
protected:
    vertex_ID_t dest_;    
};
struct AdjacencyW : public Adjacency {
    weight_t weight() const { return weight_; }
protected:
    weight_t weight_;
};
template<typename Adjacency_t> // unique (densely packed) ID for each edge
struct AdjacencyID_t : public Adjacency_t {
    edge_ID_t ID() const { return ID_; }
protected:
    edge_ID_t ID_;
};
using AdjacencyID = AdjacencyID_t<Adjacency>;
using AdjacencyWID = AdjacencyID_t<AdjacencyW>;


// Adjacency list/matrix (used when making CSR)
template<typename Adjacency_t>
using AdjacencyList = std::vector<Adjacency_t>;
template<typename Adjacency_t>
using AdjacencyMatrix = std::vector<AdjacencyList<Adjacency_t>>;


// Directed edges contianing both source and destination IDs (used by generator and when loading edgelist files)
template<typename Adjacency_t>
struct Edge {
    Edge(vertex_ID_t source, Adjacency_t adjacency) : source_(source), adjacency_(adjacency) {}
    vertex_ID_t source() const { return source_; }
    vertex_ID_t dest() const { return adjacency_.dest(); }
    const Adjacency_t &adjacency() const { return adjacency_; }
protected:
    const vertex_ID_t source_;
    const Adjacency_t adjacency_;
};
template<typename Adjacency_t>
using EdgeList = std::vector<Edge<Adjacency_t>>;


// Vertex Stats (excluding adjacency data)
struct VertexStats {}; // base class
struct VertexStatsW : public VertexStats {
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};
template<typename VertexStats_t, typename Data_t> // include arbitrary mutable data in place
struct VertexStatsData_t : public VertexStats_t {   // good for small/frequently accessed data
    Data_t data() const { return data_; }
protected:
    Data_t data_;
};
template<typename VertexStats_t, typename DataRef_t>  // include arbitrary mutable data reference
struct VertexStatsDataRef_t : public VertexStats_t { // good for large/infrequently accessed data
    DataRef_t &data_ref() const { return data_ref_; }
protected:
    Data_t &data_ref_;
};
template<typename Data_t>
using VertexStatsData = VertexStatsData_t<VertexStats, Data_t>;
template<typename Data_t>
using VertexStatsWData = VertexStatsData_t<VertexStatsW, Data_t>;
template<typename DataRef_t>
using VertexStatsDataRef = VertexStatsDataRef_t<VertexStats, DataRef_t>;
template<typename DataRef_t>
using VertexStatsWDataRef = VertexStatsDataRef_t<VertexStatsW, DataRef_t>;
template<typename Data_t, typename DataRef_t>
using VertexStatsDataDataRef = VertexStatsDataRef_t<VertexStatsData<Data_t>, DataRef_t>;
template<typename Data_t, typename DataRef_t>
using VertexStatsWDataDataRef = VertexStatsDataRef_t<VertexStatsWData<Data_t>, DataRef_t>;


// Vertex containing all data (currently unused)
template<typename VertexStats_t, typename Adjacency_t>
struct VertexComplete {
    VertexComplete(vertex_ID_t ID, AdjacencyList<Adjacency_t> adjacencies, VertexStats_t stats) :
        ID_(ID), adjacencies_(adjacencies), degree_(adjacencies.size()), stats_(stats) {}
    vertex_ID_t ID() const { return ID_; }
    const AdjacencyList<Adjacency_t> &adjacencies() const { return adjacencies_; }
    vertex_ID_t degree() const { return degree_; }
    const VertexStats_t &stats() const { return stats_; }
protected:
    const vertex_ID_t ID_;
    const AdjacencyList<Adjacency_t> adjacencies_;
    const vertex_ID_t degree_;
    VertexStats_t stats_;
};


#endif // UTIL_H_
