#ifndef GRAPH_H_
#define GRAPH_H_

#include <util.h>
#include <vector>
#include <optional>



template<typename Adjacency_t, typename VertexStats_t>
class Graph {
public:
    Graph(GraphType graph_type, Adjacency_t* edges, VertexStats_t* vertex_stats) :
            graph_type_(graph_type),
            num_vertices_(vertex_stats->size()),
            num_edges_(graph_type_ == GraphType::UNDIRECTED ? edges->size() / 2 : edges->size()),
            edges_(edges), vertex_stats_(vertex_stats) {
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
    struct Vertex : public VertexStats_t {
        const edge_ID_t csr_index_;
        const vertex_ID_t degree_;
    }
    const GraphType graph_type_;
    const vertex_ID_t num_vertices_;
    const edge_ID_t num_edges_;
    const Adjacency_t* const edges_;
    const Vertex* vertex_stats_;
    
}



#endif // GRAPH_H_
