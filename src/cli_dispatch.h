#ifndef CLI_DISPATCH_H_
#define CLI_DISPATCH_H_

#include "cli.h"

// Generic type dispatcher for any test program:
//   Generates and Builds a Graph via CLI options.
//   Calls templated functor with the Graph as an argument.
//
// Usage:
//   dispatch_types(opts, MyFunctor{});
//   or with a lambda (C++20):
//   dispatch_types(opts, [&](auto Vertex_t, auto Edge_t, auto G) { ... });
//
// The callable must have a templated operator()(Graph<...> &g) or be a generic lambda.
// Kind of ugly, but this is necessary for dynamic instantiantion of templated functions
template <typename Callable>
void dispatch_cli(const CLIOptions& opts, Callable&& func) {
    // call loader to load graph header to determine all other options
    if (!opts.load_file_path.empty()) {
        Loader loader;
        loader.LoadGraphHeader(opts.load_file_path, opts);
    }

    switch (opts.vertex_type) {
        case CLIVertexType::UNWEIGHTED:
            switch (opts.edge_type) {
                case CLIEdgeType::UNWEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeUW, GraphType::UNDIRECTED>(opts, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeUW, GraphType::DIRECTED>(opts, std::forward<Callable>(func));
                            break;
                    }
                    break;
                case CLIEdgeType::WEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeW, GraphType::UNDIRECTED>(opts, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeW, GraphType::DIRECTED>(opts, std::forward<Callable>(func));
                            break;
                    }
                    break;
                default:
                    std::cerr << "[EdgeLab CLI] Only unweighted/weighted edge types are supported in dispatch_types.\n";
                    break;
            }
            break;
        case CLIVertexType::WEIGHTED:
            switch (opts.edge_type) {
                case CLIEdgeType::UNWEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeUW, GraphType::UNDIRECTED>(opts, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeUW, GraphType::DIRECTED>(opts, std::forward<Callable>(func));
                            break;
                    }
                    break;
                case CLIEdgeType::WEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeW, GraphType::UNDIRECTED>(opts, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeW, GraphType::DIRECTED>(opts, std::forward<Callable>(func));
                            break;
                    }
                    break;
                default:
                    std::cerr << "[EdgeLab CLI] Only unweighted/weighted edge types are supported in dispatch_types.\n";
                    break;
            }
            break;
        default:
            std::cerr << "[EdgeLab CLI] Only unweighted/weighted vertex types are supported in dispatch_types.\n";
            break;
    }
}



#endif // CLI_DISPATCH_H_
