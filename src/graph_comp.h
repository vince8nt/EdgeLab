#ifndef GRAPH_COMP_H_
#define GRAPH_COMP_H_
// graph components class

#include "util.h"


// Vertex (excluding ID and adjacency data)
// Contains optional weight + mutable data
// Mutable Data stored inplace - for large data sizes, use a reference/pointer.
struct VertexUW {
    using data_type = void;
    weight_t weight() const { return 1.0; }
    // make this whole file into 1 templated class so we don't have annoying stuff like this
    /*Graph<VertexUW, Edge_t, Graph_t>::Vertex vertex(const Edge_t* edges_begin, vertex_ID_t degree) {
        return {edges_begin, degree};
    }*/
};
template<typename Data_t>
struct VertexUWD : public VertexUW {
    using data_type = Data_t;
    VertexUWD(Data_t data) : data_(data) {}
    Data_t &data() { return data_; }
    /*Graph<VertexUWD, Edge_t, Graph_t>::Vertex vertex(const Edge_t* edges_begin, vertex_ID_t degree) {
        return {data_, edges_begin, degree};
    }*/
protected:
    alignas(vertex_ID_t) Data_t data_;
};
struct VertexW : public VertexUW {
    VertexW(weight_t weight) : weight_(weight) {}
    weight_t weight() const { return weight_; }
    /*Graph<VertexW, Edge_t, Graph_t>::Vertex vertex(const Edge_t* edges_begin, vertex_ID_t degree) {
        return {weight_, edges_begin, degree};
    }*/
protected:
    const weight_t weight_;
};
template<typename Data_t>
struct VertexWD : public VertexUWD<Data_t> {
    VertexWD(weight_t weight, Data_t data) : VertexUWD<Data_t>(data), weight_(weight) {}
    weight_t weight() const { return weight_; }
    /*Graph<VertexWD, Edge_t, Graph_t>::Vertex vertex(const Edge_t* edges_begin, vertex_ID_t degree) {
        return {data_, weight_, edges_begin, degree};
    }*/
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
    EdgeUW inverse(vertex_ID_t src) const { return(EdgeUW(src)); }
protected:
    vertex_ID_t dest_;    
};
template<typename Data_t>
struct EdgeUWD : public EdgeUW {
    EdgeUWD(vertex_ID_t dest, Data_t data) : EdgeUW(dest), data_(data) {}
    using data_type = Data_t;
    Data_t &data() { return data_; }
    EdgeUWD inverse(vertex_ID_t src) const { return(EdgeUWD(src, data_)); }
protected:
    alignas(vertex_ID_t) Data_t data_;
};
struct EdgeW : public EdgeUW {
    EdgeW(vertex_ID_t dest, weight_t weight) : EdgeUW(dest), weight_(weight) {}
    weight_t weight() const { return weight_; }
    EdgeW inverse(vertex_ID_t src) const { return(EdgeW(src, weight_)); }
protected:
    weight_t weight_;
};
template<typename Data_t>
struct EdgeWD : public EdgeUWD<Data_t> {
    EdgeWD(vertex_ID_t dest, weight_t weight, Data_t data) :
        EdgeUWD<Data_t>(dest, data), weight_(weight) {}
    weight_t weight() const { return weight_; }
    EdgeWD inverse(vertex_ID_t src) const { return(EdgeWD(src, weight_, this->data_)); }
protected:
    weight_t weight_;
};


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
// vector graph (vertex list + adjacency matrix)
template<typename Vertex_t, typename Edge_t>
struct VectorGraph;
template<typename Vertex_t, typename Edge_t> // Specialization for EmptyVertex
    requires EmptyVertexType<Vertex_t>
struct VectorGraph<Vertex_t, Edge_t> {
    VectorGraph() {};
    VectorGraph(vertex_ID_t num_vertices) : matrix(AdjacencyMatrix<Edge_t>(num_vertices)) {};
    AdjacencyMatrix<Edge_t> matrix;
};
template<typename Vertex_t, typename Edge_t> // Specialization for NonEmptyVertex
    requires NonEmptyVertexType<Vertex_t>
struct VectorGraph<Vertex_t, Edge_t> {
    VectorGraph() {};
    VectorGraph(vertex_ID_t num_vertices) : matrix(AdjacencyMatrix<Edge_t>(num_vertices)) {};
    std::vector<Vertex_t> vertices;
    AdjacencyMatrix<Edge_t> matrix;
};


// Edges used to iterate through flattened CSR Graph
template <EdgeType Edge_t>
struct Edges {
private:
    const vertex_ID_t size_;
    const Edge_t* begin_;
    const Edge_t* end_;
public:
    Edges(const Edge_t* edges_begin, vertex_ID_t size) :
        size_(size), begin_(edges_begin), end_(edges_begin + size) {}
    
    // getters
    vertex_ID_t size() const { return size_; }
    Edge_t operator[](vertex_ID_t i) const { return begin_[i]; }

    // Iterator support for edges
    const Edge_t* begin() const { return begin_; }
    const Edge_t* end() const { return end_; }
};


// Vertex used in flattened CSR Graph
template <VertexType Vertex_t, EdgeType Edge_t>
struct Vertex : public Vertex_t {
    using Edges = Edges<Edge_t>;
private:
    alignas(vertex_ID_t) const Edge_t* edges_begin_;
    const vertex_ID_t degree_;
public:
    // For empty vertex - relies on Empty Base Class Optimization (EBCO)
    Vertex(const Edge_t* edges_begin, vertex_ID_t degree) requires EmptyVertexType<Vertex_t> :
            edges_begin_(edges_begin), degree_(degree) {}

    // For non empty vertex
    Vertex(const Vertex_t& vertex, const Edge_t* edges_begin, vertex_ID_t degree)
            requires NonEmptyVertexType<Vertex_t> :
            Vertex_t(vertex), edges_begin_(edges_begin), degree_(degree) {}

    // getters - also includes weight() and optionally data()
    vertex_ID_t degree() const { return degree_; }
    Edges edges() const { return Edges(edges_begin_, degree_); }

    // TODO(vince) verify that compiler optimizes edges object
    // has an edge connected to target vertex ID
    bool has_edge_to(vertex_ID_t target_id) const {
        return std::binary_search(edges().begin(), edges().end(), target_id);
    }
    // to access edge weight/data
    const Edge_t* get_edge_to(vertex_ID_t target_id) const {
        auto it = std::lower_bound(edges().begin(), edges().end(), target_id,
            [](const Edge_t& edge, vertex_ID_t target) {
                return edge.dest() < target;
            });
        if (it != edges().end() and it->dest() == target_id)
            // return &(*it);
            return it;
        return edges().end();
    }
};

#endif // GRAPH_COMP_H_