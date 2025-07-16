#ifndef GRAPH_H_
#define GRAPH_H_

#include "util.h"
#include <vector>



template<typename Adjacency_t, typename VertexStats_t>
class Graph {
public:
    struct Edges {
    private:
        const vertex_ID_t size_;
        const Adjacency_t* begin_;
        const Adjacency_t* end_;
    public:
        Edges(const Adjacency_t* edges_begin, vertex_ID_t size) :
            size_(size), begin_(edges_begin), end_(edges_begin + size) {}
        
        // getters
        vertex_ID_t size() const { return size_; }
        Adjacency_t operator[](vertex_ID_t i) const { return begin_[i]; }

        // Iterator support for edges
        const Adjacency_t* begin() const { return begin_; }
        const Adjacency_t* end() const { return end_; }

    };

    struct Vertex : public VertexStats_t {
    private:
        const Adjacency_t* edges_begin_;
        const vertex_ID_t degree_;
    public:
        // For VertexStats_t == VertexStats
        template<typename T = VertexStats_t, typename = typename std::enable_if<std::is_same<T, VertexStats>::value>::type>
        Vertex(const Adjacency_t* edges_begin, vertex_ID_t degree)
            : edges_begin_(edges_begin), degree_(degree) {}

        // For VertexStats_t != VertexStats
        template<typename T = VertexStats_t, typename = typename std::enable_if<!std::is_same<T, VertexStats>::value>::type>
        Vertex(const VertexStats_t& vertex_stats, const Adjacency_t* edges_begin, vertex_ID_t degree)
            : VertexStats_t(vertex_stats), edges_begin_(edges_begin), degree_(degree) {}

        // getters
        vertex_ID_t degree() const { return degree_; }
        Edges edges() const { return Edges(edges_begin_, degree_); }

        bool neighbors(vertex_ID_t target_id) const {
            return std::binary_search(edges().begin(), edges().end(), target_id);
        }
    };
    

    Graph(GraphType graph_type,
            edge_ID_t num_edges, Adjacency_t* edges,    
            vertex_ID_t num_vertices, Vertex* vertices
            ) :
            graph_type_(graph_type),
            num_edges_(graph_type_ == GraphType::UNDIRECTED ? num_edges / 2 : num_edges),
            edges_(edges), num_vertices_(num_vertices), vertices_(vertices) {
    }
    ~Graph() {
        free(const_cast<Adjacency_t*>(edges_));
        free(const_cast<Vertex*>(vertices_));
    }
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    Graph(Graph&& other) noexcept :
        graph_type_(std::move(other.graph_type_)),
        num_edges_(std::move(other.num_edges_)),
        edges_(std::move(other.edges_)),
        num_vertices_(std::move(other.num_vertices_)),
        vertices_(std::move(other.vertices_)) {
    }
    Graph& operator=(Graph&&) noexcept = delete;


    // getters
    GraphType graph_type() const { return graph_type_; }
    edge_ID_t num_edges() const { return num_edges_; }
    vertex_ID_t num_vertices() const { return num_vertices_; }
    const Vertex& operator[](vertex_ID_t i) const { return vertices_[i]; }

    // Iterator support for vertices
    const Vertex* begin() const { return vertices_; }
    const Vertex* end() const { return vertices_ + num_vertices_; }

private:
    const GraphType graph_type_;
    const edge_ID_t num_edges_;
    const Adjacency_t* const edges_;
    const vertex_ID_t num_vertices_;
    const Vertex* vertices_;
    
};



#endif // GRAPH_H_
