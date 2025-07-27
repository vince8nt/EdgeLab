#include "triangle_counting.h"

// Functor for dispatching templated function via CLI options
struct OpenCLTriangleDispatcher {
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        auto timer = timer_start();
        long long triangles = triangle_counting_opencl<V, E, G>(graph);
        auto time = timer_stop(timer);
        std::cout << "OpenCL Triangle Counting returned: " << triangles << " in " << time << " seconds" << std::endl;
    }
};

int main(int argc, char** argv) {
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, OpenCLTriangleDispatcher());
    return 0;
} 