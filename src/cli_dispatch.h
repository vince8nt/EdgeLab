#ifndef CLI_DISPATCH_H_
#define CLI_DISPATCH_H_

#include "cli.h"
#include "graph_maker.h"

// Clean template dispatcher that eliminates nested switch statements
class TemplateDispatcher {
public:
    // Main dispatch function
    template<typename Callable>
    static void dispatch(CLIOptions opts, AlgorithmReqs reqs, Callable&& func) {
        GraphMaker maker(opts, reqs);
        
        // Use a dispatch table to map CLI types to concrete types
        dispatch_with_table(maker, opts, std::forward<Callable>(func));
    }

private:
    // Dispatch using a cleaner table-based approach
    template<typename Callable>
    static void dispatch_with_table(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        // Map vertex types
        switch (opts.vertex_type) {
            case CLIVertexType::UNWEIGHTED:
                dispatch_vertex_unweighted(maker, opts, std::forward<Callable>(func));
                break;
            case CLIVertexType::WEIGHTED:
                dispatch_vertex_weighted(maker, opts, std::forward<Callable>(func));
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported vertex type in dispatch.\n";
                break;
        }
    }
    
    // Dispatch for unweighted vertices
    template<typename Callable>
    static void dispatch_vertex_unweighted(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.edge_type) {
            case CLIEdgeType::UNWEIGHTED:
                dispatch_edge_unweighted<VertexUW>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED:
                dispatch_edge_weighted<VertexUW>(maker, opts, std::forward<Callable>(func));
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported edge type in dispatch.\n";
                break;
        }
    }
    
    // Dispatch for weighted vertices
    template<typename Callable>
    static void dispatch_vertex_weighted(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.edge_type) {
            case CLIEdgeType::UNWEIGHTED:
                dispatch_edge_unweighted<VertexW>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED:
                dispatch_edge_weighted<VertexW>(maker, opts, std::forward<Callable>(func));
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported edge type in dispatch.\n";
                break;
        }
    }
    
    // Dispatch for unweighted edges
    template<VertexType V, typename Callable>
    static void dispatch_edge_unweighted(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.graph_type) {
            case GraphType::UNDIRECTED:
                dispatch_single<V, EdgeUW, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                break;
            case GraphType::DIRECTED:
                dispatch_single<V, EdgeUW, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
                break;
            case GraphType::BIDIRECTED:
                // TODO: Implement when supported
                std::cerr << "[EdgeLab CLI] Bidirected graphs not yet supported.\n";
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported graph type in dispatch.\n";
                break;
        }
    }
    
    // Dispatch for weighted edges
    template<VertexType V, typename Callable>
    static void dispatch_edge_weighted(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.graph_type) {
            case GraphType::UNDIRECTED:
                dispatch_single<V, EdgeW, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                break;
            case GraphType::DIRECTED:
                dispatch_single<V, EdgeW, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
                break;
            case GraphType::BIDIRECTED:
                // TODO: Implement when supported
                std::cerr << "[EdgeLab CLI] Bidirected graphs not yet supported.\n";
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported graph type in dispatch.\n";
                break;
        }
    }
    
    // Final dispatch to create the graph and call the functor
    template<VertexType V, EdgeType E, GraphType G, typename Callable>
    static void dispatch_single(GraphMaker& maker, Callable&& func) {
        Graph<V, E, G> graph = maker.make_graph<V, E, G>();
        func.template operator()<V, E, G>(graph);
    }
};

// Convenience function for backward compatibility
template<typename Callable>
void dispatch_cli(CLIOptions opts, Callable&& func) {
    AlgorithmReqs reqs;
    TemplateDispatcher::dispatch(opts, reqs, std::forward<Callable>(func));
}

template<typename Callable>
void dispatch_cli(CLIOptions opts, AlgorithmReqs reqs, Callable&& func) {
    TemplateDispatcher::dispatch(opts, reqs, std::forward<Callable>(func));
}

#endif // CLI_DISPATCH_H_
