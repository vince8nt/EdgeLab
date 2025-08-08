#include "breadth_first_search.h"
#include "../src/cli_dispatch.h"
#include "../src/cli.h"

// Functor for dispatching templated function via CLI options
struct Dispatcher {
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        vertex_ID_t src = 0;
        vertex_ID_t dest = graph.num_vertices() - 1;
        auto timer = timer_start();
        long long dist = breadth_first_search<V, E, G>(graph, src, dest);
        auto time = timer_stop(timer);
        std::cout << "BFS returned: " << dist << " in " << time << " seconds" << std::endl;
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli<BFSAlgorithmReqs>(opts, Dispatcher{});
    return 0;
}
