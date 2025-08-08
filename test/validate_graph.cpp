#include "../src/graph_test.h"

// validate_graph.cpp
/*
    This program validates the graph by checking the following:
    1. The number of edges is 0 for empty graph
    2. The edges are compressed into a contiguous array (CSR format)
    3. All edges are valid (i.e. destination is within bounds)
    4. Edges (for each vertex) are sorted by strictly increasing destination (also implies no duplicates)

    If the graph is undirected, the following are also checked:
    5. The edges are not self-loops
    6. The inverse of each edge exists
    7. The weight/data of each edge matches the inverse edge
*/

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void validate_graph(Graph<Vertex_t, Edge_t, Graph_t> &graph) {

    if (graph.num_vertices() == 0) {
        if (graph.num_edges() != 0) {
            std::cerr << "Error: Number of edges is not 0 for empty graph" << std::endl;
            exit(1);
        }
        return;
    }

    size_t num_compressed_edges = std::distance(graph[0].begin(), graph[graph.num_vertices() - 1].end());
    if (num_compressed_edges != graph.num_edges()) {
        std::cerr << "Error: Number of compressed edges does not match number of edges" << std::endl;
        exit(1);
    }

    for (vertex_ID_t v_id = 0; v_id < graph.num_vertices(); v_id++) {
        for (Edge_t *it = graph[v_id].begin(); it != graph[v_id].end(); it++) {
            Edge_t e = *it;
            if (e.dest() >= graph.num_vertices()) {
                std::cerr << "Error: Edge destination out of bounds" << std::endl;
                exit(1);
            }
            if (it != graph[v_id].begin()) {
                if (e.dest() <= (it - 1)->dest()) {
                    std::cerr << "Error: Edges not sorted strictly increasing" << std::endl;
                    exit(1);
                }
            }
            if constexpr (Graph_t == GraphType::UNDIRECTED) {
                if (e.dest() == v_id) {
                    std::cerr << "Error: Self-loop detected" << std::endl;
                    exit(1);
                }
                auto dest_v = graph[e.dest()];
                auto it = dest_v.get_edge_to(v_id);
                if (it == dest_v.end()) {
                    std::cerr << "Error: No inverse of Edge ["
                        << v_id << "->" << e.dest() << "]" << std::endl;
                    exit(1);
                }
                if constexpr (WeightedEdgeType<Edge_t>) {
                    if (it->weight() != e.weight()) {
                        std::cerr << "Error: Edge weight mismatch ["
                        << v_id << "->" << e.dest() << "](w:" << e.weight() << ") != ["
                        << e.dest() << "->" << it->dest() << "](w:" << it->weight() << ")" << std::endl;
                        exit(1);
                    }
                }
                if constexpr (DataEdgeType<Edge_t>) {
                    if (it->data() != e.data()) {
                        std::cerr << "Error: Edge data mismatch ["
                        << v_id << "->" << e.dest() << "](w:" << e.data() << ") != ["
                        << e.dest() << "->" << it->dest() << "](w:" << it->data() << ")" << std::endl;
                        exit(1);
                    }
                }
            }
        }
    }

    std::cout << "Graph is valid" << std::endl;
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        validate_graph<V, E, G>(graph);
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, Dispatcher{});
    return 0;
}
