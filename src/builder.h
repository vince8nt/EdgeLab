#ifndef BUILDER_H_
#define BUILDER_H_

#include "util.h"
#include "graph.cpp"
#include "debug.h"

template<typename Adjacency_t, typename VertexStats_t>
class Builder {
private:
    GraphType graph_type_;

    // build AdjacencyMatrix from EdgeList
    AdjacencyMatrix<Adjacency_t> EdgeListToMatrix(EdgeList<Adjacency_t> &edge_list) {
        AdjacencyMatrix<Adjacency_t> adjacency_matrix;
        for (auto &edge : edge_list) {
            const vertex_ID_t max_vertex_id = std::max(edge.source_, edge.adjacency_.dest_);
            while (max_vertex_id >= adjacency_matrix.size()) {
                adjacency_matrix.push_back(AdjacencyList<Adjacency_t>());
            }
            adjacency_matrix[edge.source_].push_back(edge.adjacency_);
            if (graph_type_ == GraphType::UNDIRECTED) {
                Adjacency_t reverse_edge = edge.adjacency_;
                reverse_edge.dest_ = edge.source_;
                adjacency_matrix[edge.adjacency_.dest_].push_back(reverse_edge);
            }
        }
        return adjacency_matrix;
    }

    // sort and remove duplicate edges from AdjacencyMatrix
    // for duplicate edges with different weights/IDs, this is non-deterministic
    vertex_ID_t SortAndRemoveDuplicates(AdjacencyMatrix<Adjacency_t> &adjacency_matrix) {
        vertex_ID_t num_edges = 0;
        for (AdjacencyList<Adjacency_t> &adjacency_list : adjacency_matrix) {
            std::sort(adjacency_list.begin(), adjacency_list.end(),
                [](const Adjacency_t &a, const Adjacency_t &b) {
                    return a.dest_ < b.dest_;
                });
            auto last = std::unique(adjacency_list.begin(), adjacency_list.end(),
                [](const Adjacency_t &a, const Adjacency_t &b) {
                    return a.dest_ == b.dest_;
                });
            adjacency_list.resize(last - adjacency_list.begin());
            num_edges += adjacency_list.size();
        }
        return num_edges;
    }
    
    // create CSR without vertex stats
    Graph<Adjacency_t, VertexStats_t> MatrixToCSR(vertex_ID_t num_edges, AdjacencyMatrix<Adjacency_t> &adjacency_matrix) {
        using Vertex = typename Graph<Adjacency_t, VertexStats_t>::Vertex;
        Vertex* vertices = (Vertex*)malloc(adjacency_matrix.size() * sizeof(Vertex));
        Adjacency_t* edges = (Adjacency_t*)malloc(num_edges * sizeof(Adjacency_t));

        edge_ID_t edges_index = 0;
        for (vertex_ID_t vertices_index = 0; vertices_index < adjacency_matrix.size(); vertices_index++) {
            new (&vertices[vertices_index]) Vertex(edges + edges_index, adjacency_matrix[vertices_index].size());
            for (int i = 0; i < adjacency_matrix[vertices_index].size(); i++)
                edges[edges_index++] = adjacency_matrix[vertices_index][i];
        }

        return Graph<Adjacency_t, VertexStats_t>(graph_type_, num_edges, edges, adjacency_matrix.size(), vertices);
    }

    // create CSR with vertex stats
    Graph<Adjacency_t, VertexStats_t> MatrixToCSR(vertex_ID_t num_edges,
            AdjacencyMatrix<Adjacency_t> &adjacency_matrix, std::vector<VertexStats_t> &vertex_stats) {
        using Vertex = typename Graph<Adjacency_t, VertexStats_t>::Vertex;
        Vertex* vertices = (Vertex*)malloc(adjacency_matrix.size() * sizeof(Vertex));
        Adjacency_t* edges = (Adjacency_t*)malloc(num_edges * sizeof(Adjacency_t));

        edge_ID_t edges_index = 0;
        for (vertex_ID_t vertices_index = 0; vertices_index < adjacency_matrix.size(); vertices_index++) {
            new (&vertices[vertices_index]) Vertex(vertex_stats[vertices_index], edges + edges_index, adjacency_matrix[vertices_index].size());
            for (int i = 0; i < adjacency_matrix[vertices_index].size(); i++)
                edges[edges_index++] = adjacency_matrix[vertices_index][i];
        }

        return Graph<Adjacency_t, VertexStats_t>(graph_type_, num_edges, edges, adjacency_matrix.size(), vertices);
    }

    

public:
    Builder(GraphType graph_type) : graph_type_(graph_type) {}

    // build Graph with no vertex stats
    template<typename T = VertexStats_t, typename = typename std::enable_if<std::is_same<T, VertexStats>::value>::type>
    Graph<Adjacency_t, VertexStats_t> BuildCSR(EdgeList<Adjacency_t> &edge_list) {
        AdjacencyMatrix<Adjacency_t> adjacency_matrix = EdgeListToMatrix(edge_list);
        vertex_ID_t num_edges = SortAndRemoveDuplicates(adjacency_matrix);
        Graph<Adjacency_t, VertexStats_t> graph = MatrixToCSR(num_edges, adjacency_matrix);
        return graph;
    }

    // build Graph with vertex stats
    template<typename T = VertexStats_t, typename = typename std::enable_if<!std::is_same<T, VertexStats>::value>::type>
    Graph<Adjacency_t, VertexStats_t> BuildCSR(EdgeList<Adjacency_t> &edge_list, std::vector<VertexStats_t> &vertex_stats) {
        AdjacencyMatrix<Adjacency_t> adjacency_matrix = EdgeListToMatrix(edge_list);
        assert(adjacency_matrix.size() <= vertex_stats.size());
        adjacency_matrix.resize(vertex_stats.size());
        vertex_ID_t num_edges = SortAndRemoveDuplicates(adjacency_matrix);
        Graph<Adjacency_t, VertexStats_t> graph = MatrixToCSR(num_edges, adjacency_matrix, vertex_stats);
        return graph;
    }
};



#endif // BUILDER_H_
