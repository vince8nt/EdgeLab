#ifndef GRAPH_H_
#define GRAPH_H_

#include <util.h>
#include <vector>
#include <optional>



template<typename Adjacency_t, typename VertexStats_t>
class Graph {
public:
    struct Edges {
    private:
        const vertex_ID_t size_;
        const Adjacency_t* begin_;
        const Adjacency_t* end_;
    public:
        Edges(Adjacency_t* edges, edge_ID_t edges_index, vertex_ID_t size) :
            size_(size), begin_(edges + edges_index), end_(edges + edges_index + size) {}
        
        // getters
        vertex_ID_t size() const { return size_; }
        Adjacency_t operator[](vertex_ID_t i) const { return begin_[i]; }

        // Iterator support for edges
        const Adjacency_t* begin() const { return vertices_; }
        const Adjacency_t* end() const { return vertices_ + num_vertices_; }

    };

    struct Vertex : public VertexStats_t {
    private:
        const edge_ID_t csr_index_;
        const vertex_ID_t degree_;
    public:
        // constructors
        typename std::enable_if<std::is_same<VertexStats_t, VertexStats>::value, void>::type
        Vertex(edge_ID_t csr_index, vertex_ID_t degree) :
            csr_index_(csr_index), degree_(degree) {}
        typename std::enable_if<!std::is_same<VertexStats_t, VertexStats>::value, void>::type
        Vertex(const VertexStats_t& vertex_stats, edge_ID_t csr_index, vertex_ID_t degree) :
            VertexStats_t(vertex_stats), csr_index_(csr_index), degree_(degree) {}

        // getters
        vertex_ID_t degree() const { return degree_; }
        Edges edges() const { return Edges(csr_index_, degree_); }

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
        free(edges_);
        free(vertex_stats_);
    }
    Graph(const Graph&) = delete;
    Graph& operator=(const Graph&) = delete;
    Graph(Graph&&) = delete;
    Graph& operator=(Graph&&) = delete;


    // getters
    GraphType graph_type() const { return graph_type_; }
    edge_ID_t num_edges() const { return num_edges_; }
    vertex_ID_t num_vertices() const { return num_vertices_; }
    Vertex& operator[](vertex_ID_t i) const { return vertices_[i]; }

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
