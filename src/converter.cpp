#include "graph_test.h"
#include "saver.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void save_graph(Graph<Vertex_t, Edge_t, Graph_t> &graph, const std::string& filepath) {
    Saver<Vertex_t, Edge_t, Graph_t> saver;
    saver.save_to_file(graph, filepath);
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    std::string filepath;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        save_graph<V, E, G>(graph, filepath);
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    if (opts.save_file_path.empty()) {
        std::cerr << "Error: No output file path provided" << std::endl;
        exit(1);
    }
    opts.auto_uw_promotion = false;
    dispatch_cli(opts, Dispatcher{opts.save_file_path});
    return 0;
}