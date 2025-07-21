#ifndef BUILDER_H_
#define BUILDER_H_

#include "util.h"
#include "graph_comp.h"
#include "graph.cpp"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Builder {
    using Vertex = Graph<Vertex_t, Edge_t, Graph_t>::Vertex;

public:
    Builder() {}

    Graph<Vertex_t, Edge_t, Graph_t> BuildGraph(VectorGraph<Vertex_t, Edge_t> &vg) {
        std::vector<vertex_ID_t> degrees;
        degrees.reserve(vg.matrix.size());
        vertex_ID_t num_edges = SortAndRemoveDuplicates(vg.matrix, degrees);
        // std::cout << vg << std::endl;
        Graph<Vertex_t, Edge_t, Graph_t> g = FlattenVectorGraph(num_edges, vg, degrees);
        // std::cout << g << std::endl;
        return g;
    }

private:

    // sort and remove duplicate edges (put at end) from AdjacencyMatrix
    // for duplicate edges with different weights/IDs, this is non-deterministic
    edge_ID_t SortAndRemoveDuplicates(AdjacencyMatrix<Edge_t> &adjacency_matrix,
            std::vector<vertex_ID_t> &degrees) {
        edge_ID_t num_edges = 0;
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
            // no need to resize vector now since it is inneficient
            // adjacency_list.resize(last - adjacency_list.begin());
            vertex_ID_t unique_edges = std::distance(adjacency_list.begin(), last);
            num_edges += unique_edges;
            degrees.push_back(unique_edges);

            // fill in back side of edges (with increasing src to dest ID)
            if constexpr (Graph_t == GraphType::UNDIRECTED) {
                for (Edge_t &edge : adjacency_list) {
                    // can be combined into loop to make faster
                    if (vertex_id < edge.dest()) // also look into constructing in place
                        adjacency_matrix[edge.dest()].push_back(edge.inverse(vertex_id));
                }
            }
        }
        return num_edges;
    }

    // create CSR
    Graph<Vertex_t, Edge_t, Graph_t> FlattenVectorGraph(vertex_ID_t num_edges,
            VectorGraph<Vertex_t, Edge_t> &vg, std::vector<vertex_ID_t> &degrees) {
        Vertex* vertices = (Vertex*)malloc((vg.matrix.size() + 1) * sizeof(Vertex));
        Edge_t* edges = (Edge_t*)malloc(num_edges * sizeof(Edge_t));

        edge_ID_t edges_index = 0;
        vertex_ID_t vertex_id = 0;
        for (; vertex_id < vg.matrix.size(); vertex_id++) {
            auto edges_begin = edges + edges_index;
            vertex_ID_t degree = degrees[vertex_id];
            if constexpr (EmptyVertexType<Vertex_t>)
                new (&vertices[vertex_id]) Vertex(edges_begin);
            else   
                new (&vertices[vertex_id]) Vertex(vg.vertices[vertex_id], edges_begin);
            for (vertex_ID_t i = 0; i < degree; i++) {
                edges[edges_index++] = vg.matrix[vertex_id][i];
            }
        }

        // end vertex
        auto edges_begin = edges + edges_index;
        if constexpr (EmptyVertexType<Vertex_t>)
            new (&vertices[vertex_id]) Vertex(edges_begin);
        else
            new (&vertices[vertex_id]) Vertex(Vertex_t(), edges_begin);

        return Graph<Vertex_t, Edge_t, Graph_t>(vg.matrix.size(), vertices, num_edges, edges);
    }


};



#endif // BUILDER_H_
