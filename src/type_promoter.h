#ifndef TYPE_PROMOTER_H_
#define TYPE_PROMOTER_H_

#include "cli.h"

// Type promotion system - handles promoting graph/vertex/edge types based on algorithm requirements
class TypePromoter {
public:
    // Promote types based on algorithm requirements
    static CLIOptions promote_types(const CLIOptions& opts, const AlgorithmReqs& reqs) {
        CLIOptions promoted_opts = opts;
        
        // Promote graph type
        promoted_opts.graph_type = promote_graph_type(opts.graph_type, reqs.graph_type);
        
        // TODO: Add vertex and edge type promotion when needed
        // promoted_opts.vertex_type = promote_vertex_type(opts.vertex_type, reqs.vertex_type);
        // promoted_opts.edge_type = promote_edge_type(opts.edge_type, reqs.edge_type);
        
        return promoted_opts;
    }
    
    // Check if promotion is needed
    static bool needs_promotion(const CLIOptions& opts, const AlgorithmReqs& reqs) {
        return opts.graph_type != promote_graph_type(opts.graph_type, reqs.graph_type);
        // TODO: Add vertex and edge type checks
    }

private:
    // Promote graph type based on requirements
    static GraphType promote_graph_type(GraphType current, GraphType required) {
        if (current == GraphType::UNDIRECTED) {
            // Can't be promoted further
            return current;
        }
        
        if (required == GraphType::UNDIRECTED) {
            return GraphType::UNDIRECTED;
        }
        
        if (required == GraphType::BIDIRECTED) {
            return GraphType::BIDIRECTED;
        }
        
        // No promotion needed
        return current;
    }
    
    // TODO: Add vertex and edge type promotion functions
    // static CLIVertexType promote_vertex_type(CLIVertexType current, CLIVertexType required);
    // static CLIEdgeType promote_edge_type(CLIEdgeType current, CLIEdgeType required);
};

#endif // TYPE_PROMOTER_H_ 