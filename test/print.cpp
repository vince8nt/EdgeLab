#include "../src/graph_test.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void run_test(Graph<Vertex_t, Edge_t, Graph_t> &graph) {
    std::cout << "CSR Graph" << std::endl << graph << std::endl;
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        run_test<V, E, G>(graph);
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, Dispatcher{});
    return 0;
}