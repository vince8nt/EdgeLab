#ifndef BUILDER_H_
#define BUILDER_H_

#include "util.h"
#include "graph_comp.h"
#include "graph.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Builder {
    using Vertex = Graph<Vertex_t, Edge_t, Graph_t>::Vertex;

public:
    Builder() {}

    Graph<Vertex_t, Edge_t, Graph_t> BuildGraph(VectorGraph<Vertex_t, Edge_t> &vg) {
        std::cout << "Building " << Graph_t << " graph with "
            << vg.matrix.size() << " vertices" << std::endl;
        auto timer = timer_start();

        // sort, remove duplictes, and create inverse edges(if undirected)
        auto edges_offset = SortAndRemoveDuplicates(vg.matrix);
        // std::cout << vg << std::endl;
        std::cout << "  - Sorting + Correcting " << Graph_t << " Vector Graph: "
            << timer_stop(timer) << " seconds" << std::endl;

        // create CSR Graph
        Graph<Vertex_t, Edge_t, Graph_t> g = FlattenVectorGraph(vg, edges_offset);
        // std::cout << g << std::endl;
        auto time = timer_stop(timer);
        std::cout << "  - Total Graph(" << vg.matrix.size() << " vertices, " << edges_offset.back()
            << " edges) build time: " << time << " seconds" << std::endl;

        return g;
    }

private:

    // sort and remove duplicate edges (put at end) from AdjacencyMatrix
    // for duplicate edges with different weights/IDs, this is non-deterministic
    std::vector<edge_ID_t> SortAndRemoveDuplicates(AdjacencyMatrix<Edge_t> &adjacency_matrix) {
        std::vector<edge_ID_t> edges_offset(adjacency_matrix.size() + 1);
        for (vertex_ID_t vertex_id = 0; vertex_id < adjacency_matrix.size(); vertex_id++) {
            AdjacencyList<Edge_t> &adjacency_list = adjacency_matrix[vertex_id];
            std::sort(adjacency_list.begin(), adjacency_list.end(),
                [](const Edge_t &a, const Edge_t &b) {
                    return a.dest() < b.dest();
                });
            auto last = std::unique(adjacency_list.begin(), adjacency_list.end(),
                [](const Edge_t &a, const Edge_t &b) {
                    return a.dest() == b.dest();
                });
            adjacency_list.resize(last - adjacency_list.begin());

            // update dest offsets for undirected graphs
            vertex_ID_t new_edges = std::distance(adjacency_list.begin(), last);
            if constexpr (Graph_t == GraphType::UNDIRECTED) {
                for (vertex_ID_t i = 0; i < new_edges; i++) {
                    edges_offset[adjacency_list[i].dest() + 1]++;
                }
            }
            edges_offset[vertex_id + 1] += edges_offset[vertex_id] + new_edges;
        }
        return edges_offset;
    }

    // create CSR
    Graph<Vertex_t, Edge_t, Graph_t> FlattenVectorGraph( VectorGraph<Vertex_t, Edge_t> &vg,
            std::vector<edge_ID_t> &edges_offset) {
        Vertex* vertices = (Vertex*)malloc((vg.matrix.size() + 1) * sizeof(Vertex));
        Edge_t* edges = (Edge_t*)malloc(edges_offset.back() * sizeof(Edge_t));
        
        // fill in vertices
        vertex_ID_t vertex_id = 0;
        for (; vertex_id < vg.matrix.size(); vertex_id++) {
            auto edges_begin = edges + edges_offset[vertex_id];
            if constexpr (EmptyVertexType<Vertex_t>)
                new (&vertices[vertex_id]) Vertex(edges_begin);
            else   
                new (&vertices[vertex_id]) Vertex(vg.vertices[vertex_id], edges_begin);
        }
        // end vertex (only used for its edge offset)
        auto edges_begin = edges + edges_offset[vertex_id];
        if constexpr (EmptyVertexType<Vertex_t>)
            new (&vertices[vertex_id]) Vertex(edges_begin);
        else
            new (&vertices[vertex_id]) Vertex(Vertex_t(), edges_begin);

        
        // fill in edges
        for (vertex_ID_t vertex_id = 0; vertex_id < vg.matrix.size(); vertex_id++) {
            for (auto &edge : vg.matrix[vertex_id]) {
                edges[edges_offset[vertex_id]++] = edge;
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    edges[edges_offset[edge.dest()]++] = edge.inverse(vertex_id);
                }
            }
        }
        return Graph<Vertex_t, Edge_t, Graph_t>(vg.matrix.size(), vertices, edges_offset.back(), edges);
    }


};



#endif // BUILDER_H_
