#ifndef CLI_DISPATCH_H_
#define CLI_DISPATCH_H_

#include "cli.h"
#include "graph_maker.h"
#include "type_promoter.h"

// Clean template dispatcher that eliminates nested switch statements
class TemplateDispatcher {
public:
    // Main dispatch function with compile-time algorithm requirements
    template<typename AlgorithmReqsType, typename Callable>
    static void dispatch(CLIOptions opts, Callable&& func) {
        // Promote types based on algorithm requirements (compile-time)
        CLIOptions promoted_opts = TypePromoter::promote_types<AlgorithmReqsType>(opts);
        GraphMaker maker(promoted_opts);
        
        // Use a dispatch table to map promoted types to concrete types
        dispatch_with_table(maker, promoted_opts, std::forward<Callable>(func));
    }

private:
    // Dispatch using a cleaner table-based approach
    template<typename Callable>
    static void dispatch_with_table(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        // Map vertex types (now using promoted types)
        switch (opts.vertex_type) {
            case CLIVertexType::UNWEIGHTED:
                dispatch_vertex_unweighted(maker, opts, std::forward<Callable>(func));
                break;
            case CLIVertexType::WEIGHTED:
                dispatch_vertex_weighted(maker, opts, std::forward<Callable>(func));
                break;
            case CLIVertexType::UNWEIGHTED_DATA:
                dispatch_vertex_unweighted_data(maker, opts, std::forward<Callable>(func));
                break;
            case CLIVertexType::WEIGHTED_DATA:
                dispatch_vertex_weighted_data(maker, opts, std::forward<Callable>(func));
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
            case CLIEdgeType::UNWEIGHTED_DATA:
                dispatch_edge_unweighted_data<VertexUW>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED_DATA:
                dispatch_edge_weighted_data<VertexUW>(maker, opts, std::forward<Callable>(func));
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
            case CLIEdgeType::UNWEIGHTED_DATA:
                dispatch_edge_unweighted_data<VertexW>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED_DATA:
                dispatch_edge_weighted_data<VertexW>(maker, opts, std::forward<Callable>(func));
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported edge type in dispatch.\n";
                break;
        }
    }
    
    // Dispatch for unweighted vertices with data
    template<typename Callable>
    static void dispatch_vertex_unweighted_data(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.edge_type) {
            case CLIEdgeType::UNWEIGHTED:
                dispatch_edge_unweighted<VertexUWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED:
                dispatch_edge_weighted<VertexUWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::UNWEIGHTED_DATA:
                dispatch_edge_unweighted_data<VertexUWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED_DATA:
                dispatch_edge_weighted_data<VertexUWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            default:
                std::cerr << "[EdgeLab CLI] Unsupported edge type in dispatch.\n";
                break;
        }
    }
    
    // Dispatch for weighted vertices with data
    template<typename Callable>
    static void dispatch_vertex_weighted_data(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.edge_type) {
            case CLIEdgeType::UNWEIGHTED:
                dispatch_edge_unweighted<VertexWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED:
                dispatch_edge_weighted<VertexWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::UNWEIGHTED_DATA:
                dispatch_edge_unweighted_data<VertexWD<int32_t>>(maker, opts, std::forward<Callable>(func));
                break;
            case CLIEdgeType::WEIGHTED_DATA:
                dispatch_edge_weighted_data<VertexWD<int32_t>>(maker, opts, std::forward<Callable>(func));
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
    
    // Dispatch for unweighted edges with data
    template<VertexType V, typename Callable>
    static void dispatch_edge_unweighted_data(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.graph_type) {
            case GraphType::UNDIRECTED:
                dispatch_single<V, EdgeUWD<int32_t>, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                break;
            case GraphType::DIRECTED:
                dispatch_single<V, EdgeUWD<int32_t>, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
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
    
    // Dispatch for weighted edges with data
    template<VertexType V, typename Callable>
    static void dispatch_edge_weighted_data(GraphMaker& maker, const CLIOptions& opts, Callable&& func) {
        switch (opts.graph_type) {
            case GraphType::UNDIRECTED:
                dispatch_single<V, EdgeWD<int32_t>, GraphType::UNDIRECTED>(maker, std::forward<Callable>(func));
                break;
            case GraphType::DIRECTED:
                dispatch_single<V, EdgeWD<int32_t>, GraphType::DIRECTED>(maker, std::forward<Callable>(func));
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
        func(graph);
    }
};

// Convenience function for backward compatibility (without algorithm requirements)
template<typename Callable>
void dispatch_cli(CLIOptions opts, Callable&& func) {
    // Use default AlgorithmReqs
    TemplateDispatcher::dispatch<AlgorithmReqs>(opts, std::forward<Callable>(func));
}

// Main dispatch function with algorithm requirements
template<typename AlgorithmReqsType, typename Callable>
void dispatch_cli(CLIOptions opts, Callable&& func) {
    TemplateDispatcher::dispatch<AlgorithmReqsType>(opts, std::forward<Callable>(func));
}

#endif // CLI_DISPATCH_H_
