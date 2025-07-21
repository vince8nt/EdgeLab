#include "../src/generator.h"
#include "../src/cli.h"

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int test_iteration(GenType gen_type, int scale, int degree) {
    using Generator = Generator<Vertex_t, Edge_t, Graph_t>;
    using VectorGraph = VectorGraph<Vertex_t, Edge_t>;
    using Builder = Builder<Vertex_t, Edge_t, Graph_t>;
    using Graph = Graph<Vertex_t, Edge_t, Graph_t>;

    // generate edge list
    Generator generator(gen_type, scale, degree);
    VectorGraph vg = generator.Generate();

    // generaate CLI Graph
    Builder builder;
    Graph graph = builder.BuildGraph(vg);

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
    const CLIOptions& opts;
    int &exit_code;
    template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
    void operator()() const {
        exit_code = test_iteration<Vertex_t, Edge_t, Graph_t>(opts.gen_type, opts.scale, opts.degree);
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    int exit_code = 0;
    dispatch_types(opts, Dispatcher{opts, exit_code});
    if (exit_code)
        std::cerr << "Failed with exit code: " << exit_code << std::endl;
    else
        std::cout << "Succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
}
