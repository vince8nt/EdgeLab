#include "triangle_counting.h"


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        auto timer = timer_start();
        edge_ID_t triangles = triangle_counting_threaded<V, E, G>(graph);
        auto time = timer_stop(timer);
        std::cout << "Multithreaded triangle counting returned: " << triangles << " in " << time << " seconds" << std::endl;
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, Dispatcher());
    return 0;
}
