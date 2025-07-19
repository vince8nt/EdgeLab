#ifndef GRAPH_H_
#define GRAPH_H_

#include "util.h"
#include "graph_comp.h"



template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Graph {
    using Edges = Edges<Edge_t>;
    using Vertex = Vertex<Vertex_t, Edge_t>;

public:
    // constructors / destructor
    Graph(vertex_ID_t num_vertices, Vertex* vertices, edge_ID_t num_edges, Edge_t* edges) :
            num_vertices_(num_vertices), vertices_(vertices),
            num_edges_(num_edges), edges_(edges) {
    }
    Graph(const Graph&) = delete;
    Graph(Graph&& other) noexcept : // only enable move constructor.
        num_vertices_(other.num_vertices_),
        vertices_(other.vertices_),
        num_edges_(other.num_edges_),
        edges_(other.edges_) {
    }
    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) noexcept = delete;
    ~Graph() {
        free(const_cast<Edge_t*>(edges_));
        free(const_cast<Vertex*>(vertices_));
    }


    // getters
    edge_ID_t num_edges() const {
        if constexpr (Graph_t == GraphType::UNDIRECTED)
            return num_edges_ / 2;
        else
            return num_edges_;
    }
    vertex_ID_t num_vertices() const { return num_vertices_; }
    const Vertex& operator[](vertex_ID_t i) const { return vertices_[i]; }


    // Iterator support for vertices
    const Vertex* begin() const { return vertices_; }
    const Vertex* end() const { return vertices_ + num_vertices_; }


private:
    const vertex_ID_t num_vertices_;
    const Vertex* vertices_;
    const edge_ID_t num_edges_;
    const Edge_t* const edges_;
};



#endif // GRAPH_H_
