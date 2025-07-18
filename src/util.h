#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <vector>
#include <iostream>
#include <type_traits>
#include <algorithm>
#include <concepts> // For std::derived_from
#include <memory>


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
};
enum class EdgeType {
    UNWEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED,
    WEIGHTED_DATA
};


// Edge (excluding sourceID)
// Contains destination and optional weight + mutable data
// Mutable Data stored inplace - for large data sizes, use a reference/pointer.
struct EdgeUW {
    EdgeUW(vertex_ID_t dest) : dest_(dest) {}
    using data_type = void;
    vertex_ID_t dest() const { return dest_; }
    weight_t weight() const { return 1.0; }
protected:
    const vertex_ID_t dest_;    
};
template<typename Data_t>
struct EdgeUWD : public EdgeUW {
    EdgeUWD(vertex_ID_t dest, Data_t data) : EdgeUW(dest), data_(data) {}
    using data_type = Data_t;
    Data_t &data() { return data_; }
protected:
    Data_t data_;
};
struct EdgeW : public EdgeUW {
    EdgeW(vertex_ID_t dest, weight_t weight) : EdgeUW(dest), weight_(weight) {}
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};
template<typename Data_t>
struct EdgeWD : public EdgeUWD<Data_t> {
    EdgeWD(vertex_ID_t dest, weight_t weight, Data_t data) :
        EdgeUWD<Data_t>(dest, data), weight_(weight) {}
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};


// Adjacency list/matrix (used when making CSR)
template<std::derived_from<EdgeUW> Edge_t>
using AdjacencyList = std::vector<Edge_t>;
template<std::derived_from<EdgeUW> Edge_t>
using AdjacencyMatrix = std::vector<AdjacencyList<Edge_t>>;


// Vertex (excluding ID and adjacency data)
// Contains optional weight + mutable data
// Mutable Data stored inplace - for large data sizes, use a reference/pointer.
struct VertexUW {
    using data_type = void;
    weight_t weight() const { return 1.0; }
};
template<typename Data_t>
struct VertexUWD : public VertexUW {
    using data_type = Data_t;
    VertexUWD(Data_t data) : data_(data) {}
    Data_t &data() { return data_; }
protected:
    Data_t data_;
};
struct VertexW : public VertexUW {
    VertexW(weight_t weight) : weight_(weight) {}
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};
template<typename Data_t>
struct VertexWD : public VertexUWD<Data_t> {
    VertexWD(weight_t weight, Data_t data) : VertexUWD<Data_t>(data), weight_(weight) {}
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};


#endif // UTIL_H_
