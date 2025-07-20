#ifndef GRAPH_H_
#define GRAPH_H_

#include "util.h"
#include "graph_comp.h"



template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Graph {

public:

    // Edges used to iterate through flattened CSR Graph
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
    struct Vertex : public Vertex_t {
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
    vertex_ID_t ID(const Vertex* it) const { return it - vertices_; }

private:
    const vertex_ID_t num_vertices_;
    const Vertex* vertices_;
    const edge_ID_t num_edges_;
    const Edge_t* const edges_;
};



#endif // GRAPH_H_
