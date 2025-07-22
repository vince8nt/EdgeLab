
#include "graph.h"

// verify that Vertex and edge objects are densely packed for optimal memory usage
template<typename Vertex_t, typename Edge_t>
int verify_packing() {
    using Vertex = Graph<Vertex_t, Edge_t, GraphType::UNDIRECTED>::Vertex;

    int exit_code = 0;

    // check Vertex (used directly in CSR)
    size_t exp_v_size = sizeof(Edge_t*);        // EdgeList pointer from Graph::Vertex
    std::string v_name = "Vertex";
    if constexpr (WeightedVertexType<Vertex_t>) { // optional weight
        exp_v_size += sizeof(weight_t);
        v_name += " + weight";
    }
    if constexpr (DataVertexType<Vertex_t>) {     // optional data
        exp_v_size += sizeof(typename Vertex::data_type);
        v_name += " + data of size ";
        v_name += std::to_string(sizeof(typename Vertex::data_type));
    }
    if (sizeof(Vertex) != exp_v_size) {
        std::cerr << "dense packing failed for " << v_name << ":" << std::endl
            << "  - expected " << exp_v_size << " bytes" << std::endl
            << "  - got  " << sizeof(Vertex) << " bytes" << std::endl;
        exit_code |= 1;
    }

    // check Edge_t (used directly in CSR)
    size_t exp_e_size = sizeof(vertex_ID_t);      // dest variable
    std::string e_name = "Edge";
    if constexpr (WeightedEdgeType<Edge_t>) { // optional weight
        exp_e_size += sizeof(weight_t);
        e_name += " + weight";
    }
    if constexpr (DataEdgeType<Edge_t>) {     // optional data
        exp_e_size += sizeof(typename Edge_t::data_type);
        e_name += " + data of size ";
        e_name += std::to_string(sizeof(typename Edge_t::data_type));
    }
    if (sizeof(Edge_t) != exp_e_size) {
        std::cerr << "dense packing failed for " << e_name << ":" << std::endl
            << "  - expected " << exp_e_size << " bytes" << std::endl
            << "  - got  " << sizeof(Edge_t) << " bytes" << std::endl;
        exit_code |= 1;
    }

    return exit_code;
}

int main() {
    int exit_code = 0;
    exit_code |= verify_packing<VertexUW, EdgeUW>(); // no data
    exit_code |= verify_packing<VertexW, EdgeW>();
    exit_code |= verify_packing<VertexUWD<int32_t>, EdgeUWD<int32_t>>(); // 4 bytes
    exit_code |= verify_packing<VertexWD<int32_t>, EdgeWD<int32_t>>();
    exit_code |= verify_packing<VertexUWD<int64_t>, EdgeUWD<int64_t>>(); // 8 bytes
    exit_code |= verify_packing<VertexWD<int64_t>, EdgeWD<int64_t>>();
    exit_code |= verify_packing<VertexUWD<std::vector<int>>, EdgeUWD<std::vector<int>>>(); // 24 bytes
    exit_code |= verify_packing<VertexWD<std::vector<int>>, EdgeWD<std::vector<int>>>();
    
    if (exit_code)
        std::cerr << "Failed with exit code: " << exit_code << std::endl;
    else
        std::cout << "Succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
}

