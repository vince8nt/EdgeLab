#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>

using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = float;

enum class GraphType {
    UNDIRECTED,
    DIRECTED,
    WEIGHTED,
    WEIGHTED_DIRECTED,
}


struct Vertex {
    vertex_ID_t ID_;
    vertex_ID_t degree_;
}
struct VertexW : public Vertex {
    weight_t weight_;
}
template<typename Vertex_t, typename Data_t>
struct VertexData : public Vertex_t {
    Data_t data_;
}


struct Adjacency {
    vertex_ID_t dest_;    
}
struct AdjacencyW : public Adjacency {
    weight_t weight_;
}
template<typename Adjacency_t>
struct AdjacencyID : public Adjacency_t {
    edge_ID_t ID_;
}



#endif
