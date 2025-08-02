#ifndef BUILDER_H_
#define BUILDER_H_

#include "util.h"
#include "graph_comp.h"
#include "graph.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Builder {
    using Vertex = CSR_Vertex<Vertex_t, Edge_t, Graph_t>;

public:
    Builder() {}

    Graph<Vertex_t, Edge_t, Graph_t> BuildGraph(VectorGraph<Vertex_t, Edge_t> &vg) {
        if constexpr (DEBUG) 
            VerifyVectorGraph(vg);

        std::cout << "Building " << Graph_t << " graph with "
            << vg.matrix.size() << " vertices" << std::endl;
        auto timer = timer_start();

        // sort, remove duplictes, and return offsets(if undirected)
        auto edges_offset = PrepAdjacencyMatrix(vg.matrix);
        // std::cout << vg << std::endl;
        std::cout << "  - Sorting + Correcting " << Graph_t << " Vector Graph: "
            << timer_stop(timer) << " seconds" << std::endl;

        // create CSR Graph
        Graph<Vertex_t, Edge_t, Graph_t> g = FlattenVectorGraph(vg, edges_offset);
        // std::cout << g << std::endl;
        auto time = timer_stop(timer);
        std::cout << "  - Total Graph(" << g.num_vertices() << " vertices, " << g.num_edges()
            << " edges) build time: " << time << " seconds" << std::endl;

        return g;
    }

private:

    // verify that vector graph is valid
    void VerifyVectorGraph(VectorGraph<Vertex_t, Edge_t> &vg) {
        if constexpr (!EmptyVertexType<Vertex_t>) {
            if (vg.vertices.size() != vg.matrix.size()) {
                std::cout << "Error: vertex data size does not match adjacency list size" << std::endl;
                std::cout << "  - [" << vg.vertices.size() << " != " << vg.matrix.size() << "]" << std::endl;
                exit(1);
            }
        }
        for (vertex_ID_t vertex_id = 0; vertex_id < vg.matrix.size(); vertex_id++) {
            for (auto &edge : vg.matrix[vertex_id]) {
                if (edge.dest() >= vg.matrix.size()) {
                    std::cout << "Error: edge destination out of bounds" << std::endl;
                    std::cout << "  - [" << vertex_id << " -> " << edge.dest() << "]" << std::endl;
                    exit(1);
                }
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (edge.dest() <= vertex_id) {
                        std::cout << "Error: edge destination is not greater than source" << std::endl;
                        std::cout << "  - [" << vertex_id << " -> " << edge.dest() << "]" << std::endl;
                        exit(1);
                    }
                }
            }
        }
    }

    // sort and remove duplicate edges from AdjacencyList
    // for duplicate edges with different weights, only keep smallest weight
    void SortAndRemoveDuplicates(AdjacencyList<Edge_t> &adjacency_list) {
        std::sort(adjacency_list.begin(), adjacency_list.end(),
            [](const Edge_t &a, const Edge_t &b) {
                if constexpr (WeightedEdgeType<Edge_t>)
                    return a.dest() < b.dest() or (a.dest() == b.dest() and a.weight() < b.weight());
                else
                    return a.dest() < b.dest();
            });
        auto last = std::unique(adjacency_list.begin(), adjacency_list.end(),
            [](const Edge_t &a, const Edge_t &b) {
                return a.dest() == b.dest();
            });
        adjacency_list.resize(last - adjacency_list.begin());
    }

    // sort and remove duplicate edges from AdjacencyMatrix
    // for directed graphs, return number of edges
    // for undirected graphs, return edges_offset vector (allows symmetrizing edges directly in CSR)
    auto PrepAdjacencyMatrix(AdjacencyMatrix<Edge_t> &adjacency_matrix) {
        if constexpr (Graph_t == GraphType::UNDIRECTED or Graph_t == GraphType::BIDIRECTED) {
            std::vector<edge_ID_t> edges_offset(adjacency_matrix.size() + 1);
            for (vertex_ID_t vertex_id = 0; vertex_id < adjacency_matrix.size(); vertex_id++) {
                AdjacencyList<Edge_t> &adjacency_list = adjacency_matrix[vertex_id];
                SortAndRemoveDuplicates(adjacency_list);
                edges_offset[vertex_id + 1] += edges_offset[vertex_id] + adjacency_list.size();
                for (auto &edge : adjacency_list)
                    edges_offset[edge.dest() + 1]++;
            }
            return edges_offset;
        }
        else {
            edge_ID_t num_edges = 0;
            for (vertex_ID_t vertex_id = 0; vertex_id < adjacency_matrix.size(); vertex_id++) {
                AdjacencyList<Edge_t> &adjacency_list = adjacency_matrix[vertex_id];
                SortAndRemoveDuplicates(adjacency_list);
                num_edges += adjacency_list.size();
            }
            return num_edges;
        }
    }

    // create CSR (undirected graphs only)
    Graph<Vertex_t, Edge_t, Graph_t> FlattenVectorGraph( VectorGraph<Vertex_t, Edge_t> &vg,
            std::vector<edge_ID_t> &edges_offset) requires (Graph_t == GraphType::UNDIRECTED) {
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

        // fill in edges (symmetrize in place)
        for (vertex_ID_t vertex_id = 0; vertex_id < vg.matrix.size(); vertex_id++) {
            for (auto &edge : vg.matrix[vertex_id]) {
                edges[edges_offset[vertex_id]++] = edge;
                edges[edges_offset[edge.dest()]++] = edge.inverse(vertex_id);
            }
        }

        return Graph<Vertex_t, Edge_t, Graph_t>(vg.matrix.size(), vertices, edges_offset.back(), edges);
    }

    // create CSR (directed graphs only)
    Graph<Vertex_t, Edge_t, Graph_t> FlattenVectorGraph( VectorGraph<Vertex_t, Edge_t> &vg,
            edge_ID_t num_edges) requires (Graph_t == GraphType::DIRECTED) {
        Vertex* vertices = (Vertex*)malloc((vg.matrix.size() + 1) * sizeof(Vertex));
        Edge_t* edges = (Edge_t*)malloc(num_edges * sizeof(Edge_t));
        
        // fill in vertices and edges
        auto edges_begin = edges;
        vertex_ID_t vertex_id = 0;
        for (; vertex_id < vg.matrix.size(); vertex_id++) {
            if constexpr (EmptyVertexType<Vertex_t>)
                new (&vertices[vertex_id]) Vertex(edges_begin);
            else   
                new (&vertices[vertex_id]) Vertex(vg.vertices[vertex_id], edges_begin);
            for (auto &edge : vg.matrix[vertex_id])
                *edges_begin++ = edge;
        }
        // end vertex (only used for its edge offset)
        if constexpr (EmptyVertexType<Vertex_t>)
            new (&vertices[vertex_id]) Vertex(edges_begin);
        else
            new (&vertices[vertex_id]) Vertex(Vertex_t(), edges_begin);

        return Graph<Vertex_t, Edge_t, Graph_t>(vg.matrix.size(), vertices, num_edges, edges);
    }

    // create CSR (bidirected graphs only)
    Graph<Vertex_t, Edge_t, Graph_t> FlattenVectorGraph( VectorGraph<Vertex_t, Edge_t> &vg,
            std::vector<edge_ID_t> &edges_offset) requires (Graph_t == GraphType::BIDIRECTED) {
        Vertex* vertices = (Vertex*)malloc((vg.matrix.size() + 1) * sizeof(Vertex));
        Edge_t* edges = (Edge_t*)malloc(edges_offset.back() * sizeof(Edge_t));
        Edge_t* edges_in = (Edge_t*)malloc(edges_offset.back() * sizeof(Edge_t));
        
        // fill in vertices
        auto edges_begin = edges;
        vertex_ID_t vertex_id = 0;
        for (; vertex_id < vg.matrix.size(); vertex_id++) {
            auto edges_in_begin = edges_in + edges_offset[vertex_id];
            if constexpr (EmptyVertexType<Vertex_t>)
                new (&vertices[vertex_id]) Vertex(edges_begin, edges_in_begin);
            else   
                new (&vertices[vertex_id]) Vertex(vg.vertices[vertex_id], edges_begin, edges_in_begin);
            edges_begin += vg.matrix[vertex_id].size();
        }
        // end vertex (only used for its edge offset)
        auto edges_in_begin = edges_in + edges_offset[vertex_id];
        if constexpr (EmptyVertexType<Vertex_t>)
            new (&vertices[vertex_id]) Vertex(edges_begin, edges_in_begin);
        else
            new (&vertices[vertex_id]) Vertex(Vertex_t(), edges_begin, edges_in_begin);

        // fill in edges (create inverse edges in place)
        edges_begin = edges;
        for (vertex_ID_t vertex_id = 0; vertex_id < vg.matrix.size(); vertex_id++) {
            for (auto &edge : vg.matrix[vertex_id]) {
                *(edges_begin++) = edge;
                edges_in[edges_offset[edge.dest()]++] = edge.inverse(vertex_id);
            }
        }

        return Graph<Vertex_t, Edge_t, Graph_t>(vg.matrix.size(), vertices, edges_offset.back(), edges, edges_in);
    }


};



#endif // BUILDER_H_
