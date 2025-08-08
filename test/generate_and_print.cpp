#include "../src/graph_test.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void run_test(GenType gen_type, int scale, int degree) {
    // generate Vector Graph
    Generator<Vertex_t, Edge_t, Graph_t> generator(gen_type, scale, degree);
    VectorGraph<Vertex_t, Edge_t> vg = generator.Generate();
    std::cout << "Generated Vector Graph" << std::endl << vg << std::endl;

    // build CSR Graph
    Builder<Vertex_t, Edge_t, Graph_t> builder;
    Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
    std::cout << "CSR Graph" << std::endl << graph << std::endl;
}

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    run_test<VertexUW, EdgeUW, GraphType::UNDIRECTED>(opts.gen_type, opts.scale, opts.degree);
    return 0;
} 