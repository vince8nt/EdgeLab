#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <vector>
#include <iostream>
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


// Vertex concepts
template <typename Vertex_t>
concept EmptyVertexType = std::is_same_v<Vertex_t, VertexUW>;
template <typename Vertex_t>
concept NonEmptyVertexType = std::derived_from<Vertex_t, VertexUW> and
                             !EmptyVertexType<Vertex_t>;
                             // !std::is_same_v<Vertex_t, VertexUW>;
template <typename Vertex_t>
concept DataVertexType = std::derived_from<Vertex_t, VertexUWD<typename Vertex_t::data_type>>;
template <typename Vertex_t>
concept NonDataVertexType = std::derived_from<Vertex_t, VertexUW> and
                            !DataVertexType<Vertex_t>;
template <typename Vertex_t>
concept WeightedVertexType = std::is_same_v<Vertex_t, VertexW> or
                             std::is_same_v<Vertex_t, VertexWD<typename Vertex_t::data_type>>;
template <typename Vertex_t>
concept NonWeightedVertexType = std::derived_from<Vertex_t, VertexUW> and
                                !WeightedVertexType<Vertex_t>;


// edge concepts
template <typename Edge_t>
concept DataEdgeType = std::derived_from<Edge_t, EdgeUWD<typename Edge_t::data_type>>;
template <typename Edge_t>
concept NonDataEdgeType = std::derived_from<Edge_t, EdgeUW> and
                          !DataEdgeType<Edge_t>;
template <typename Edge_t>
concept WeightedEdgeType = std::is_same_v<Edge_t, EdgeW> or
                           std::is_same_v<Edge_t, EdgeWD<typename Edge_t::data_type>>;
template <typename Edge_t>
concept NonWeightedEdgeType = std::derived_from<Edge_t, EdgeUW> and
                          !WeightedEdgeType<Edge_t>;


// Vertex and Edge vector-based containers (unsorted and unflattened) (used during generation / loading)
template<std::derived_from<EdgeUW> Edge_t>
using AdjacencyList = std::vector<Edge_t>;
template<std::derived_from<EdgeUW> Edge_t>
using AdjacencyMatrix = std::vector<AdjacencyList<Edge_t>>;
// Sparse Row graph (vertex list + adjacency matrix)
template<typename Vertex_t, typename Edge_t>
struct SparseRowGraph;
template<typename Vertex_t, typename Edge_t> // Specialization for EmptyVertex
    requires EmptyVertexType<Vertex_t>
struct SparseRowGraph<Vertex_t, Edge_t> {
    SparseRowGraph() {};
    SparseRowGraph(vertex_ID_t num_vertices) : matrix(AdjacencyMatrix<Edge_t>(num_vertices)) {};
    AdjacencyMatrix<Edge_t> matrix;
};
template<typename Vertex_t, typename Edge_t> // Specialization for NonEmptyVertex
    requires NonEmptyVertexType<Vertex_t>
struct SparseRowGraph<Vertex_t, Edge_t> {
    SparseRowGraph() {};
    SparseRowGraph(vertex_ID_t num_vertices) : matrix(AdjacencyMatrix<Edge_t>(num_vertices)) {};
    std::vector<Vertex_t> vertices;
    AdjacencyMatrix<Edge_t> matrix;
};


#endif // UTIL_H_
