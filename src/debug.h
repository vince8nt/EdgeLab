#ifndef DEBUG_H_
#define DEBUG_H_

#include <iomanip>
#include "util.h"
// #include "graph.cpp"
// #include "builder.h"

template<typename Vertex_t, typename Edge_t>
class Debug {
public:

    void print(AdjacencyList<Edge_t> &adjacency_list) {
        for (auto &edge : adjacency_list) {
            if constexpr (WeightedEdgeType<Edge_t>) {
                std::cout << "(" << edge.dest() << " "
                    << std::setprecision(3) << edge.weight() << ") ";
            }
            else {
                std::cout << edge.dest() << " ";
            }
        }
        std::cout << std::endl;
    }

    void print(SparseRowGraph<Vertex_t, Edge_t> &srg) {
        std::cout << "Sparse Row Graph:" << std::endl;
        for (vertex_ID_t i = 0; i < srg.matrix.size(); i++) {
            if constexpr (WeightedVertexType<Vertex_t>)
                std::cout << "["<< i << " " <<
                    std::setprecision(3) << srg.vertices[i].weight() << "]: ";
            else
                std::cout << i << ": ";
            print(srg.matrix[i]);
        }
    }
/*
    // should have the same output as print_adjacency_matrix
    void print_graph(Graph<Adjacency_t, VertexStats_t> &graph) {
        std::cout << "Graph (printed with indexing):" << std::endl;
        for (vertex_ID_t v = 0; v < graph.num_vertices(); v++) {
            std::cout << v << ": ";
            auto edges = graph[v].edges();
            for (vertex_ID_t e = 0; e < edges.size(); e++) {
                std::cout << edges[e].dest() << " ";
            }
            std::cout << std::endl;
        }
    }

    // should have the same output as print_adjacency_matrix
    void print_graph_it(Graph<Adjacency_t, VertexStats_t> &graph) {
        std::cout << "Graph (printed with iterators):" << std::endl;
        vertex_ID_t v_id = 0;
        for (auto &v : graph) { // TODO: find a way to get vertex ID from iterator
            std::cout << v_id << ": ";
            for (auto e : v.edges()) {
                std::cout << e.dest() << " ";
            }
            std::cout << std::endl;
            v_id++;
        }
    }
        */
};

#endif // DEBUG_H_
