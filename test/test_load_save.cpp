#include "../src/cli_dispatch.h"
#include "../src/saver.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void save_load_compare(Graph<Vertex_t, Edge_t, Graph_t> &graph, const std::string& filepath) {
    // save graph
    Saver<Vertex_t, Edge_t, Graph_t> saver;
    saver.save_to_file(graph, filepath);

    // load the saved graph (should be the same)
    Loader loader;
    CLIOptions opts;
    opts.load_file_path = filepath;
    loader.load_graph_header(opts);
    Graph<Vertex_t, Edge_t, Graph_t> loaded_graph = loader.LoadGraphBody<Vertex_t, Edge_t, Graph_t>();

    // compare the graphs to verify equality
    if (graph.num_vertices() != loaded_graph.num_vertices()) {
        std::cerr << "Graphs have different number of vertices" << std::endl;
        std::cerr << "Initial: " << graph.num_vertices()
            << " Loaded: " << loaded_graph.num_vertices() << std::endl;
        exit(1);
    }
    if (graph.num_edges() != loaded_graph.num_edges()) {
        std::cerr << "Graphs have different number of edges" << std::endl;
        std::cerr << "Initial: " << graph.num_edges()
            << " Loaded: " << loaded_graph.num_edges() << std::endl;
        exit(1);
    }
    for (vertex_ID_t i = 0; i < graph.num_vertices(); i++) {
        if (graph[i].degree() != loaded_graph[i].degree()) {
            std::cerr << "Graphs have different degrees for vertex " << i << std::endl;
            std::cerr << "Initial: " << graph[i].degree()
                << " Loaded: " << loaded_graph[i].degree() << std::endl;
            exit(1);
        }
        if constexpr (WeightedVertexType<Vertex_t>) {
            if (graph[i].weight() != loaded_graph[i].weight()) {
                std::cerr << "Graphs have different weights for vertex " << i << std::endl;
                std::cerr << "Initial: " << graph[i].weight()
                    << " Loaded: " << loaded_graph[i].weight() << std::endl;
                exit(1);
            }
        }
        for (edge_ID_t j = 0; j < graph[i].degree(); j++) {
            if (graph[i][j].dest() != loaded_graph[i][j].dest()) {
                std::cerr << "Graphs have different destinations for edge " << j
                    << " of vertex " << i << std::endl;
                std::cerr << "Initial: " << graph[i][j].dest()
                    << " Loaded: " << loaded_graph[i][j].dest() << std::endl;
                exit(1);
            }
            if constexpr (WeightedEdgeType<Edge_t>) {
                if (graph[i][j].weight() != loaded_graph[i][j].weight()) {
                    std::cerr << "Graphs have different weights for edge " << j
                        << " of vertex " << i << std::endl;
                    std::cerr << "Initial: " << graph[i][j].weight()
                        << " Loaded: " << loaded_graph[i][j].weight() << std::endl;
                    exit(1);
                }
            }
        }
    }
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    std::string filepath;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        save_load_compare<V, E, G>(graph, filepath);
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    if (opts.save_file_path.empty()) {
        std::cerr << "Error: No output file path provided" << std::endl;
        exit(1);
    }
    dispatch_cli(opts, Dispatcher{opts.save_file_path});
    return 0;
}