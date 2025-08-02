#ifndef CLI_DISPATCH_H_
#define CLI_DISPATCH_H_

#include "cli.h"

// Generate/load and build Graph via CLI generator options
template <typename Callable, typename V, typename E, GraphType G>
void CLI_create_graph (const CLIOptions& opts, Callable&& func) {
    if (!opts.load_file_path.empty()) {
        // load graph from file
        Graph<V, E, G> graph = opts.loader->LoadGraphBody<V, E, G>();
        // template and call func with graph
        func.template operator()<V, E, G>(graph);
    } else {
        // generate vector graph
        Generator<V, E, G> generator(opts.gen_type, opts.scale, opts.degree);
        VectorGraph<V, E> vg = generator.Generate();
        // build graph
        Builder<V, E, G> builder;
        Graph<V, E, G> graph = builder.BuildGraph(vg);
        // template and call func with graph
        func.template operator()<V, E, G>(graph);
    }
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
void dispatch_cli(CLIOptions& opts, Callable&& func) {
    dispatch_cli<Callable, GraphType::DIRECTED>(opts, std::forward<Callable>(func));
}

template <typename Callable, GraphType RequiredGraphType>
void dispatch_cli(CLIOptions& opts, Callable&& func) {
    // call loader to load graph header and set:
    // - graph type
    // - vertex type
    // - edge type
    if (!opts.load_file_path.empty()) {
        opts.loader = std::make_unique<Loader>();
        opts.loader->load_graph_header(opts);
    }

    // promote graph type to required graph type
    if (opts.graph_type == GraphType::UNDIRECTED or RequiredGraphType == GraphType::UNDIRECTED)
        opts.graph_type = GraphType::UNDIRECTED;
    else if (RequiredGraphType == GraphType::BIDIRECTED)
        opts.graph_type = GraphType::BIDIRECTED;

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
                        case GraphType::BIDIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeUW, GraphType::BIDIRECTED>(opts, std::forward<Callable>(func));
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
                        case GraphType::BIDIRECTED:
                            CLI_create_graph<Callable, VertexUW, EdgeW, GraphType::BIDIRECTED>(opts, std::forward<Callable>(func));
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
                        case GraphType::BIDIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeW, GraphType::BIDIRECTED>(opts, std::forward<Callable>(func));
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
                        case GraphType::BIDIRECTED:
                            CLI_create_graph<Callable, VertexW, EdgeW, GraphType::BIDIRECTED>(opts, std::forward<Callable>(func));
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
