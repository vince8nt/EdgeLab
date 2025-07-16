#ifndef GRAPH_H_
#define GRAPH_H_

#include <util.h>
#include <vector>
#include <optional>



template<typename Adjacency_t, typename VertexStats_t>
class Graph {
public:
    struct Vertex : public VertexStats_t {
        const edge_ID_t csr_index_;
        const vertex_ID_t degree_;
        Vertex(const VertexStats_t& vertex_stats, edge_ID_t csr_index, vertex_ID_t degree) :
            VertexStats_t(vertex_stats), csr_index_(csr_index), degree_(degree) {}
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
private:
    const GraphType graph_type_;
    const edge_ID_t num_edges_;
    const Adjacency_t* const edges_;
    const vertex_ID_t num_vertices_;
    const Vertex* vertices_;
    
}



#endif // GRAPH_H_
