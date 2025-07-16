#ifndef DEBUG_H_
#define DEBUG_H_

#include <util.h>

template<typename Adjacency_t, typename VertexStats_t>
class Debug {
public:
    void print_edge_list(EdgeList<Adjacency_t> &edge_list) {
        for (auto &edge : edge_list) {
            std::cout << edge.source_ << " " << edge.adjacency_.dest_ << std::endl;
        }
    }

    void print_adjacency_list(AdjacencyList<Adjacency_t> &adjacency_list) {
        for (auto &adjacency : adjacency_list) {
            std::cout << adjacency.dest_ << " ";
        }
        std::cout << std::endl;
    }

    void print_adjacency_matrix(AdjacencyMatrix<Adjacency_t> &adjacency_matrix) {
        for (vertex_ID_t i = 0; i < adjacency_matrix.size(); i++) {
            std::cout << i << ": ";
            print_adjacency_list(adjacency_matrix[i]);
        }
    }

    // should have the same output as print_adjacency_matrix
    void print_graph(Graph<Adjacency_t, VertexStats_t> &graph) {
        for (vertex_ID_t v = 0; v < graph.num_vertices(); v++) {
            std::cout << v << ": ";
            auto edges = graph[v].edges();
            for (vertex_ID_t e = 0; e < edges.size(); e++) {
                std::cout << edges[e].dest_ << " ";
            }
            std::cout << std::endl;
        }
    }

    // should have the same output as print_adjacency_matrix
    void print_graph_it(Graph<Adjacency_t, VertexStats_t> &graph) {
        vertex_ID_t v_id = 0;
        for (auto &v : graph) { // TODO: find a way to get vertex ID from iterator
            std::cout << v_id << ": ";
            for (auto e : v.edges()) {
                std::cout << e.dest_ << " ";
            }
            std::cout << std::endl;
            v_id++;
        }
    }
};

#endif // DEBUG_H_
