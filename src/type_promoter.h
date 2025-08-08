#ifndef TYPE_PROMOTER_H_
#define TYPE_PROMOTER_H_

#include "cli.h"

// Type promotion system - handles promoting graph/vertex/edge types based on algorithm requirements
class TypePromoter {
public:
    // Promote types based on algorithm requirements (compile-time)
    template<typename AlgorithmReqsType>
    static CLIOptions promote_types(const CLIOptions& opts) {
        CLIOptions promoted_opts = opts;
        
        // Promote graph type
        promoted_opts.graph_type = promote_graph_type(opts.graph_type, AlgorithmReqsType::graph_type);
        
        // Promote vertex and edge types
        promoted_opts.vertex_type = promote_vertex_type(opts.vertex_type, AlgorithmReqsType::vertex_type);
        promoted_opts.edge_type = promote_edge_type(opts.edge_type, AlgorithmReqsType::edge_type);
        
        return promoted_opts;
    }

private:
    // Promote graph type based on requirements
    static GraphType promote_graph_type(GraphType current, GraphType required) {
        // Undirected -> Undirected
        if (current == GraphType::UNDIRECTED) {
            return current;
        }
        
        // Directed -> Undirected
        if (required == GraphType::UNDIRECTED) {
            return GraphType::UNDIRECTED;
        }
        
        // Directed -> Bidirected
        if (required == GraphType::BIDIRECTED) {
            return GraphType::BIDIRECTED;
        }
        
        // Directed -> Directed
        return current;
    }
    
    // Promote vertex type
    static CLIVertexType promote_vertex_type(CLIVertexType current, CLIVertexType required) {
        // NonData vertices
        if (required == CLIVertexType::UNWEIGHTED) {
            return CLIVertexType::UNWEIGHTED;
        }
        if (required == CLIVertexType::WEIGHTED) {
            if (current == CLIVertexType::WEIGHTED) {
                return CLIVertexType::WEIGHTED;
            }
            return CLIVertexType::UNWEIGHTED;
        }

        // Data vertices
        if (required == CLIVertexType::UNWEIGHTED_DATA) {
            return CLIVertexType::UNWEIGHTED_DATA;
        }
        if (current == CLIVertexType::WEIGHTED) {
            return CLIVertexType::WEIGHTED_DATA;
        }
        return CLIVertexType::UNWEIGHTED_DATA;
    }

    // Promote edge type
    static CLIEdgeType promote_edge_type(CLIEdgeType current, CLIEdgeType required) {
        // NonData edges
        if (required == CLIEdgeType::UNWEIGHTED) {
            return CLIEdgeType::UNWEIGHTED;
        }
        if (required == CLIEdgeType::WEIGHTED) {
            if (current == CLIEdgeType::WEIGHTED) {
                return CLIEdgeType::WEIGHTED;
            }
            return CLIEdgeType::UNWEIGHTED;
        }

        // Data edges
        if (required == CLIEdgeType::UNWEIGHTED_DATA) {
            return CLIEdgeType::UNWEIGHTED_DATA;
        }
        if (current == CLIEdgeType::WEIGHTED) {
            return CLIEdgeType::WEIGHTED_DATA;
        }
        return CLIEdgeType::UNWEIGHTED_DATA;
    }
};

#endif // TYPE_PROMOTER_H_ 