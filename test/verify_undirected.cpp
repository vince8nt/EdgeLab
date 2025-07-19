
#include "../src/generator.h"
#include "../src/debug.h"
#include "../src/cli.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Tester {
public:
    void run_test(GenType gen_type, int scale, int degree) {
        // generate edge list
        Generator<Vertex_t, Edge_t, Graph_t> generator(gen_type, scale, degree);
        VectorGraph<Vertex_t, Edge_t> vg = generator.Generate();
        D_.print(vg);

        Builder<Vertex_t, Edge_t, Graph_t> builder;
        Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
        D_.print(graph);
        D_.print_it(graph);
    }

private:
    Debug<Vertex_t, Edge_t, Graph_t> D_;
};

// Functor for running the tester
struct TesterRunner {
    const CLIOptions& opts;
    template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
    void operator()() const {
        std::cout << "[EdgeLab CLI] Running Tester with Vertex: " << typeid(Vertex_t).name()
                  << ", Edge: " << typeid(Edge_t).name()
                  << ", GraphType: " << (Graph_t == GraphType::UNDIRECTED ? "UNDIRECTED" : "DIRECTED") << std::endl;
        Tester<Vertex_t, Edge_t, Graph_t> tester;
        tester.run_test(opts.gen_type, opts.scale, opts.degree);
        std::cout << "[EdgeLab CLI] Tester finished.\n";
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_types(opts, TesterRunner{opts});
    return 0;
}
