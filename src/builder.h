#ifndef BUILDER_H_
#define BUILDER_H_

#include "util.h"
#include "graph_comp.h"
#include "graph.cpp"
#include "debug.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Builder {
    using AdjacencyList = AdjacencyList<Edge_t>;
    using AdjacencyMatrix = AdjacencyMatrix<Edge_t>;
    using VectorGraph = VectorGraph<Vertex_t, Edge_t>;
    using Vertex = Vertex<Vertex_t, Edge_t>;
    using Graph = Graph<Vertex_t, Edge_t, Graph_t>;

public:
    Builder() {}

    Graph BuildGraph(VectorGraph &vg) {
        std::vector<vertex_ID_t> degrees;
        degrees.reserve(vg.matrix.size());
        vertex_ID_t num_edges = SortAndRemoveDuplicates(vg.matrix, degrees);
        Debug<Vertex_t, Edge_t, Graph_t> D;
        D.print(vg);
        Graph graph = FlattenVectorGraph(num_edges, vg, degrees);
        return graph;
    }

private:


    // sort and remove duplicate edges (put at end) from AdjacencyMatrix
    // for duplicate edges with different weights/IDs, this is non-deterministic
    edge_ID_t SortAndRemoveDuplicates(AdjacencyMatrix &adjacency_matrix,
            std::vector<vertex_ID_t> &degrees) {
        edge_ID_t num_edges = 0;
        for (vertex_ID_t vertex_id = 0; vertex_id < adjacency_matrix.size(); vertex_id++) {
            AdjacencyList &adjacency_list = adjacency_matrix[vertex_id];
            std::sort(adjacency_list.begin(), adjacency_list.end(),
                [](const Edge_t &a, const Edge_t &b) {
                    return a.dest() < b.dest();
                });
            auto last = std::unique(adjacency_list.begin(), adjacency_list.end(),
                [](const Edge_t &a, const Edge_t &b) {
                    return a.dest() == b.dest();
                });
            // no need to resize vector now since it is inneficient
            // adjacency_list.resize(last - adjacency_list.begin()); // sus?
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
    Graph FlattenVectorGraph(vertex_ID_t num_edges, VectorGraph &vg, std::vector<vertex_ID_t> &degrees) {
        Vertex* vertices = (Vertex*)malloc(vg.matrix.size() * sizeof(Vertex));
        Edge_t* edges = (Edge_t*)malloc(num_edges * sizeof(Edge_t));

        edge_ID_t edges_index = 0;
        for (vertex_ID_t vertex_id = 0; vertex_id < vg.matrix.size(); vertex_id++) {
            // new (&vertices[vertex_id]) Vertex(vg.vertices[vertex_id], edges + edges_index, vg.matrix[vertex_id].size());
            auto edges_begin = edges + edges_index;
            vertex_ID_t degree = degrees[vertex_id];
            if constexpr (EmptyVertexType<Vertex_t>)
                new (&vertices[vertex_id]) Vertex(edges_begin, degree);
            else   
                new (&vertices[vertex_id]) Vertex(vg.vertices[vertex_id], edges_begin, degree);
            for (int i = 0; i < degrees[vertex_id]; i++) {
                edges[edges_index++] = vg.matrix[vertex_id][i];
            }
        }

        return Graph(vg.matrix.size(), vertices, num_edges, edges);
    }


};



#endif // BUILDER_H_
