
#include "../src/generator.h"
#include "../src/debug.h"
#include "../src/cli.h"

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int verify_undirected(GenType gen_type, int scale, int degree) {
    if constexpr (Graph_t == GraphType::DIRECTED)
        std::cout << "warning: graph type mismatch" << std::endl;
    
    // Debug<Vertex_t, Edge_t, Graph_t> D_; // Debug for printing graph

    // generate edge list
    Generator<Vertex_t, Edge_t, Graph_t> generator(gen_type, scale, degree);
    VectorGraph<Vertex_t, Edge_t> vg = generator.Generate();
    // D_.print(vg);

    // generaate CLI Graph
    Builder<Vertex_t, Edge_t, Graph_t> builder;
    Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
    // D_.print(graph);
    // D_.print_it(graph);

    vertex_ID_t v_id = 0;
    for (auto v : graph) { // TODO: find a way to get vertex ID from iterator
        for (auto e : v.edges()) {
            
            auto dest_v = graph[e.dest()];
            auto it = dest_v.get_edge_to(v_id);
            if (it == dest_v.edges().end())
                return 1;
            if constexpr (WeightedEdgeType<Edge_t>) {
                if (it->weight() != e.weight())
                    return 1;
            }
        }
        v_id++;
    }

    return 0;
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    const CLIOptions& opts;
    int &exit_code;
    template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
    void operator()() const {
        exit_code = verify_undirected<Vertex_t, Edge_t, Graph_t>(opts.gen_type, opts.scale, opts.degree);
    }
};

int main(int argc, char** argv) {
    const std::string name = "verify_undirected";
    CLIOptions opts = parse_cli(argc, argv);
    int exit_code = 0;
    dispatch_types(opts, Dispatcher{opts, exit_code});
    if (exit_code)
        std::cerr << name << " failed with exit code: " << exit_code << std::endl;
    else
        std::cout << name << " succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
}
