#include "../src/graph_test.h"

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int test_iteration(Graph<Vertex_t, Edge_t, Graph_t> &graph) {

    // check that Graph iterators are consistent with indexing
    vertex_ID_t v_index = 0;
    for (auto v = graph.begin(); v != graph.end(); v++) {
        if (v_index >= graph.num_vertices()) {
            std::cerr << "Error: Vertex index " << v_index << " out of range" << std::endl;
            return 1;
        }
        if (v_index != graph.ID(v)) {
            std::cerr << "Error: ID mismatch [" << v_index << ", " << graph.ID(v) << "]" << std::endl;
            return 1;
        }
        vertex_ID_t e_index = 0;
        for (auto & e : v) {
            if (e_index >= v.degree()) {
                std::cerr << "Error: Edge index " << e_index << " out of range for vertex " << v_index << std::endl;
                return 1;
            }
            if (e.dest() != v[e_index].dest()) {
                std::cerr << "Error: Edge mismatch [" << v_index << "->" << v[e_index].dest()
                    << "], [" << v_index << "->" << e.dest() << std::endl;
                return 1;
            }
            e_index++;
        }
        if (e_index != v.degree()) {
            std::cerr << "Error: Edge index " << e_index << " not at size for vertex " << v_index << std::endl;
            return 1;
        }
        v_index++;
    }
    if (v_index != graph.num_vertices()) {
        std::cerr << "Error: Vertex index " << v_index << " not at num_vertices" << std::endl;
        return 1;
    }

    return 0;
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    int &exit_code;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        exit_code = test_iteration<V, E, G>(graph);
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
