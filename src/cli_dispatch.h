#ifndef CLI_DISPATCH_H_
#define CLI_DISPATCH_H_

#include "cli.h"
#include "graph_maker.h"

// Generate/load and build Graph via CLI generator options
template <typename Callable, typename V, typename E, GraphType G>
void CLI_create_graph (GraphMaker& maker, Callable&& func) {
    Graph<V, E, G> graph = maker.make_graph<V, E, G>();
    func.template operator()<V, E, G>(graph);
}

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
void dispatch_cli(CLIOptions opts, Callable&& func) {
    AlgorithmReqs reqs;
    dispatch_cli(opts, reqs, std::forward<Callable>(func));
}

template <typename Callable>
void dispatch_cli(CLIOptions opts, AlgorithmReqs reqs, Callable&& func) {
    GraphMaker maker(opts, reqs);

    switch (opts.vertex_type) {
        case CLIVertexType::UNWEIGHTED:
            switch (opts.edge_type) {
                case CLIEdgeType::UNWEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeUW, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeUW, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::BIDIRECTED:
                            // CLI_create_graph<Callable, VertexUW, EdgeUW, GraphType::BIDIRECTED>(maker, std::forward<Callable>(func));
                            break;
                    }
                    break;
                case CLIEdgeType::WEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeW, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeW, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::BIDIRECTED:
                            // CLI_create_graph<Callable, VertexUW, EdgeW, GraphType::BIDIRECTED>(maker, std::forward<Callable>(func));
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
                            CLI_create_graph<Callable, VertexW, EdgeUW, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeUW, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::BIDIRECTED:
                            // CLI_create_graph<Callable, VertexW, EdgeW, GraphType::BIDIRECTED>(maker, std::forward<Callable>(func));
                            break;
                    }
                    break;
                case CLIEdgeType::WEIGHTED:
                    switch (opts.graph_type) {
                        case GraphType::UNDIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeW, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::DIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeW, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
                            break;
                        case GraphType::BIDIRECTED:
                            // CLI_create_graph<Callable, VertexW, EdgeW, GraphType::BIDIRECTED>(maker, std::forward<Callable>(func));
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
