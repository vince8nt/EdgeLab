#ifndef GRAPH_COMP_H_
#define GRAPH_COMP_H_
// graph components class

#include "util.h"





// Vertex (excluding ID and adjacency data)
// Contains optional weight + mutable data
// Mutable Data stored inplace - for large data sizes, use a seperate array indexed by vertexID.
struct VertexUW {
    using data_type = void;
    weight_t weight() const { return default_weight; }
};
#pragma pack (push, 4)
template<typename Data_t>
struct VertexUWD : public VertexUW {
    using data_type = Data_t;
    VertexUWD() : data_(Data_t()) {} // default for builder
    VertexUWD(Data_t data) : data_(data) {}
    Data_t &data() { return data_; }
protected:
    Data_t data_;
};
struct VertexW : public VertexUW {
    VertexW() : weight_(default_weight) {} // default for builder
    VertexW(weight_t weight) : weight_(weight) {}
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};
template<typename Data_t>
struct VertexWD : public VertexUWD<Data_t> {
    VertexWD() : VertexUWD<Data_t>(), weight_(default_weight) {} // default for builder
    VertexWD(weight_t weight) : VertexUWD<Data_t>(), weight_(weight) {}
    VertexWD(weight_t weight, Data_t data) : VertexUWD<Data_t>(data), weight_(weight) {}
    weight_t weight() const { return weight_; }
protected:
    const weight_t weight_;
};
#pragma pack (pop)


// Edge (excluding sourceID)
// Contains destination and optional weight + mutable data
// Mutable Data stored inplace - for large data sizes, use a seperate array indexed by edgeIndex.
struct EdgeUW {
    EdgeUW() : dest_(0) {}  // Default constructor
    EdgeUW(vertex_ID_t dest) : dest_(dest) {}
    using data_type = void;
    vertex_ID_t dest() const { return dest_; }
    weight_t weight() const { return default_weight; }
    EdgeUW inverse(vertex_ID_t src) const { return(EdgeUW(src)); }
protected:
    vertex_ID_t dest_;    
};
#pragma pack (push, 4)
template<typename Data_t>
struct EdgeUWD : public EdgeUW {
    EdgeUWD() : EdgeUW(), data_() {}  // Default constructor
    EdgeUWD(vertex_ID_t dest, Data_t data) : EdgeUW(dest), data_(data) {}
    using data_type = Data_t;
    Data_t &data() { return data_; }
    EdgeUWD inverse(vertex_ID_t src) const { return(EdgeUWD(src, data_)); }
protected:
    Data_t data_;
};
#pragma pack (pop)
struct EdgeW : public EdgeUW {
    EdgeW() : EdgeUW(), weight_(default_weight) {}  // Default constructor
    EdgeW(vertex_ID_t dest, weight_t weight) : EdgeUW(dest), weight_(weight) {}
    weight_t weight() const { return weight_; }
    EdgeW inverse(vertex_ID_t src) const { return(EdgeW(src, weight_)); }
protected:
    weight_t weight_;
};
#pragma pack (push, 4)
template<typename Data_t>
struct EdgeWD : public EdgeUWD<Data_t> {
    EdgeWD() : EdgeUWD<Data_t>(), weight_(default_weight) {}  // Default constructor
    EdgeWD(vertex_ID_t dest, weight_t weight, Data_t data) :
        EdgeUWD<Data_t>(dest, data), weight_(weight) {}
    weight_t weight() const { return weight_; }
    EdgeWD inverse(vertex_ID_t src) const { return(EdgeWD(src, weight_, this->data_)); }
protected:
    weight_t weight_;
};
#pragma pack (pop)


// Vertex concepts
template <typename Vertex_t>
concept VertexType = std::derived_from<Vertex_t, VertexUW>;
template <typename Vertex_t>
concept EmptyVertexType = VertexType<Vertex_t> && std::is_same_v<Vertex_t, VertexUW>;
template <typename Vertex_t>
concept NonEmptyVertexType = VertexType<Vertex_t> && !EmptyVertexType<Vertex_t>;
template <typename Vertex_t>
concept DataVertexType = VertexType<Vertex_t> && std::derived_from<Vertex_t, VertexUWD<typename Vertex_t::data_type>>;
template <typename Vertex_t>
concept NonDataVertexType = VertexType<Vertex_t> && !DataVertexType<Vertex_t>;
template <typename Vertex_t>
concept WeightedVertexType = VertexType<Vertex_t> && (std::is_same_v<Vertex_t, VertexW> ||
                             std::is_same_v<Vertex_t, VertexWD<typename Vertex_t::data_type>>);
template <typename Vertex_t>
concept NonWeightedVertexType = VertexType<Vertex_t> && !WeightedVertexType<Vertex_t>;


// edge concepts
template <typename Edge_t>
concept EdgeType = std::derived_from<Edge_t, EdgeUW>;
template <typename Edge_t>
concept DataEdgeType = EdgeType<Edge_t> && std::derived_from<Edge_t, EdgeUWD<typename Edge_t::data_type>>;
template <typename Edge_t>
concept NonDataEdgeType = EdgeType<Edge_t> && !DataEdgeType<Edge_t>;
template <typename Edge_t>
concept WeightedEdgeType = EdgeType<Edge_t> && (std::is_same_v<Edge_t, EdgeW> ||
                           std::is_same_v<Edge_t, EdgeWD<typename Edge_t::data_type>>);
template <typename Edge_t>
concept NonWeightedEdgeType = EdgeType<Edge_t> && !WeightedEdgeType<Edge_t>;


// Vertex and Edge vector-based containers (unsorted and unflattened) (used during generation / loading)
template<std::derived_from<EdgeUW> Edge_t>
using AdjacencyList = std::vector<Edge_t>;
template<std::derived_from<EdgeUW> Edge_t>
using AdjacencyMatrix = std::vector<AdjacencyList<Edge_t>>;


// Vector Graph (vertex list + adjacency matrix) (used during generation / loading)
template<typename Vertex_t, typename Edge_t>
struct VectorGraph;
template<typename Vertex_t, typename Edge_t> // Specialization for EmptyVertex
    requires EmptyVertexType<Vertex_t>
struct VectorGraph<Vertex_t, Edge_t> {
    VectorGraph() {};
    VectorGraph(vertex_ID_t num_vertices) : matrix(AdjacencyMatrix<Edge_t>(num_vertices)) {};
    friend std::ostream& operator<<(std::ostream& os, const VectorGraph<Vertex_t, Edge_t>& vg) {
        return vg_to_stream(os, vg);
    }
    AdjacencyMatrix<Edge_t> matrix;
};
template<typename Vertex_t, typename Edge_t> // Specialization for NonEmptyVertex
    requires NonEmptyVertexType<Vertex_t>
struct VectorGraph<Vertex_t, Edge_t> {
    VectorGraph() {};
    VectorGraph(vertex_ID_t num_vertices) : matrix(AdjacencyMatrix<Edge_t>(num_vertices)) {};
    friend std::ostream& operator<<(std::ostream& os, const VectorGraph<Vertex_t, Edge_t>& vg) {
        return vg_to_stream(os, vg);
    }
    std::vector<Vertex_t> vertices;
    AdjacencyMatrix<Edge_t> matrix;
};
// printing functionality for VectorGraph
template<typename Vertex_t, typename Edge_t>
std::ostream& vg_to_stream(std::ostream& os, const VectorGraph<Vertex_t, Edge_t>& vg) {
    for (vertex_ID_t i = 0; i < vg.matrix.size(); i++) {
        if constexpr (WeightedVertexType<Vertex_t>)
            os << "["<< i << " " << std::setprecision(3) << vg.vertices[i].weight() << "]: ";
        else
            os << i << ": ";
        for (auto &e : vg.matrix[i]) {
            if constexpr (WeightedEdgeType<Edge_t>)
                os << "[" << e.dest() << " " << std::setprecision(3) << e.weight() << "] ";
            else
                os << e.dest() << " ";
        }
        os << std::endl;
    }
    return os;
}








#endif // GRAPH_COMP_H_