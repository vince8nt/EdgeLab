#ifndef BUILDER_H_
#define BUILDER_H_

#include <util.h>

template<typename Adjacency_t, typename VertexStats_t>
class Builder {
public:
    Builder(GraphType graph_type) : graph_type_(graph_type) {}
    Graph BuildCSR(EdgeList<Adjacency_t> &edge_list) {
        
    }
private:
    AdjacencyMatrix<Adjacency_t> EdgeListToMatrix(EdgeList<Adjacency_t> &edge_list) {
        if (graph_type_ == GraphType::UNDIRECTED)
            return UndirectedEdgeListToMatrix(edge_list);
        AdjacencyMatrix<Adjacency_t> adjacency_matrix();
        for (auto &edge : edge_list) {
            while (edge.source_ >= adjacency_matrix.size())
                adjacency_matrix.push_back(AdjacencyList<Adjacency_t>());
            adjacency_matrix[edge.source_].push_back(edge.adjacency_);
        }
        return adjacency_matrix;
    }
    AdjacencyMatrix<Adjacency_t> UndirectedEdgeListToMatrix(EdgeList<Adjacency_t> &edge_list) {
        AdjacencyMatrix<Adjacency_t> adjacency_matrix();
        for (auto &edge : edge_list) {
            vertex_ID_t max_vertex = std::max(edge.source_, edge.adjacency_.dest_);
            while (max_vertex >= adjacency_matrix.size())
                adjacency_matrix.push_back(AdjacencyList<Adjacency_t>());
            adjacency_matrix[edge.source_].push_back(edge.adjacency_);
            adjacency_matrix[edge.adjacency_.dest_].push_back(edge.source_);
        }
        return adjacency_matrix;
    }
    Graph<Adjacency_t> MatrixToCSR(AdjacencyMatrix<Adjacency_t> &adjacency_matrix) {
        // sort and remove duplicate edges
        // for duplicate edges with different weights/IDs, this is non-deterministic
        edge_ID_t num_edges = 0;
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

        Adjacency_t* edges = (Adjacency_t*)malloc(num_edges * sizeof(Adjacency_t));
        VertexStats_t* vertices = (VertexStats_t*)malloc(adjacency_matrix.size() * sizeof(VertexStats_t));

    }

    GraphType graph_type_;
}



#endif // BUILDER_H_
