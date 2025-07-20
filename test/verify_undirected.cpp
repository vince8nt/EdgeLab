
#include "../src/generator.h"
#include "../src/debug.h"
#include "../src/cli.h"

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
int verify_undirected(GenType gen_type, int scale, int degree) {
    using Generator = Generator<Vertex_t, Edge_t, Graph_t>;
    using VectorGraph = VectorGraph<Vertex_t, Edge_t>;
    using Builder = Builder<Vertex_t, Edge_t, Graph_t>;
    using Graph = Graph<Vertex_t, Edge_t, Graph_t>;

    if constexpr (Graph_t == GraphType::DIRECTED)
        std::cout << "warning: graph type mismatch" << std::endl;

    // generate edge list
    Generator generator(gen_type, scale, degree);
    VectorGraph vg = generator.Generate();

    // generaate CLI Graph
    Builder builder;
    Graph graph = builder.BuildGraph(vg);

    for (vertex_ID_t v_id = 0; v_id < graph.num_vertices(); v_id++) {
        for (auto e : graph[v_id].edges()) {

            auto dest_v = graph[e.dest()];
            auto it = dest_v.get_edge_to(v_id);
            if (it == dest_v.edges().end()) {
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
    const CLIOptions& opts;
    int &exit_code;
    template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
    void operator()() const {
        exit_code = verify_undirected<Vertex_t, Edge_t, Graph_t>(opts.gen_type, opts.scale, opts.degree);
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
