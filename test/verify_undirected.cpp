#include "../src/cli_dispatch.h"

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int verify_undirected(Graph<Vertex_t, Edge_t, Graph_t> &graph) {

    if constexpr (Graph_t == GraphType::DIRECTED)
        std::cout << "warning: graph type mismatch" << std::endl;

    for (vertex_ID_t v_id = 0; v_id < graph.num_vertices(); v_id++) {
        for (auto e : graph[v_id]) {

            auto dest_v = graph[e.dest()];
            auto it = dest_v.get_edge_to(v_id);
            if (it == dest_v.end()) {
                std::cerr << "Error: No inverse of Edge ["
                    << v_id << "->" << e.dest() << "]" << std::endl;
                return 1;
            }
            if constexpr (WeightedEdgeType<Edge_t>) {
                if (it->weight() != e.weight()) {
                    std::cerr << "Error: Edge weight mismatch ["
                    << v_id << "->" << e.dest() << "](w:" << e.weight() << ") != ["
                    << e.dest() << "->" << it->dest() << "](w:" << it->weight() << ")" << std::endl;
                    return 1;
                }
            }
            if constexpr (DataEdgeType<Edge_t>) {
                if (it->data() != e.data()) {
                    std::cerr << "Error: Edge data mismatch ["
                    << v_id << "->" << e.dest() << "](w:" << e.data() << ") != ["
                    << e.dest() << "->" << it->dest() << "](w:" << it->data() << ")" << std::endl;
                    return 1;
                }
            }
        }
    }

    return 0;
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    int &exit_code;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        exit_code = verify_undirected<V, E, G>(graph);
    }
};

int main(int argc, char** argv) {
    int exit_code = 0;
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, Dispatcher{exit_code});
    if (exit_code)
        std::cerr << "Failed with exit code: " << exit_code << std::endl;
    else
        std::cout << "Succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
}
