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
        
        // Promote vertex and edge types (ensuring consistency)
        auto [vertex_type, edge_type] = promote_vertex_edge_types(
            opts.vertex_type, opts.edge_type, 
            AlgorithmReqsType::vertex_type, AlgorithmReqsType::edge_type
        );
        promoted_opts.vertex_type = vertex_type;
        promoted_opts.edge_type = edge_type;
        
        // Debug output
        std::cerr << "[TypePromoter] Original: vertex=" << static_cast<int>(opts.vertex_type) << ", edge=" << static_cast<int>(opts.edge_type) << std::endl;
        std::cerr << "[TypePromoter] Required: vertex=" << static_cast<int>(AlgorithmReqsType::vertex_type) << ", edge=" << static_cast<int>(AlgorithmReqsType::edge_type) << std::endl;
        std::cerr << "[TypePromoter] Promoted: vertex=" << static_cast<int>(vertex_type) << ", edge=" << static_cast<int>(edge_type) << std::endl;
        
        return promoted_opts;
    }
    
    // Check if promotion is needed (compile-time)
    template<typename AlgorithmReqsType>
    static bool needs_promotion(const CLIOptions& opts) {
        auto [vertex_type, edge_type] = promote_vertex_edge_types(
            opts.vertex_type, opts.edge_type, 
            AlgorithmReqsType::vertex_type, AlgorithmReqsType::edge_type
        );
        
        return opts.graph_type != promote_graph_type(opts.graph_type, AlgorithmReqsType::graph_type) ||
               opts.vertex_type != vertex_type ||
               opts.edge_type != edge_type;
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
    
    // Promote vertex and edge types together to ensure consistency
    static std::pair<CLIVertexType, CLIEdgeType> promote_vertex_edge_types(
        CLIVertexType current_vertex, CLIEdgeType current_edge,
        CLIVertexType required_vertex, CLIEdgeType required_edge
    ) {
        // Check if algorithm requires data types
        bool need_data = (required_vertex == CLIVertexType::UNWEIGHTED_DATA || 
                         required_vertex == CLIVertexType::WEIGHTED_DATA ||
                         required_edge == CLIEdgeType::UNWEIGHTED_DATA || 
                         required_edge == CLIEdgeType::WEIGHTED_DATA);
        
        // Check if algorithm requires weighted types
        bool need_weighted = (required_vertex == CLIVertexType::WEIGHTED || 
                             required_vertex == CLIVertexType::WEIGHTED_DATA ||
                             required_edge == CLIEdgeType::WEIGHTED || 
                             required_edge == CLIEdgeType::WEIGHTED_DATA);
        
        // Check if CLI options have weighted types available
        bool has_weighted = (current_vertex == CLIVertexType::WEIGHTED || 
                            current_vertex == CLIVertexType::WEIGHTED_DATA ||
                            current_edge == CLIEdgeType::WEIGHTED || 
                            current_edge == CLIEdgeType::WEIGHTED_DATA);
        
        if (need_data) {
            // Algorithm requires data types
            if (need_weighted && has_weighted) {
                return {CLIVertexType::WEIGHTED_DATA, CLIEdgeType::WEIGHTED_DATA};
            } else {
                return {CLIVertexType::UNWEIGHTED_DATA, CLIEdgeType::UNWEIGHTED_DATA};
            }
        } else if (need_weighted && has_weighted) {
            // Algorithm requires weighted types (but not data)
            return {CLIVertexType::WEIGHTED, CLIEdgeType::WEIGHTED};
        } else {
            // Algorithm requires unweighted types (demote if necessary)
            return {CLIVertexType::UNWEIGHTED, CLIEdgeType::UNWEIGHTED};
        }
    }
    
    // Legacy functions for backward compatibility (deprecated)
    static CLIVertexType promote_vertex_type(CLIVertexType current, CLIVertexType required) {
        if (required == CLIVertexType::UNWEIGHTED) {
            return CLIVertexType::UNWEIGHTED;
        }
        if (required == CLIVertexType::WEIGHTED) {
            return (current == CLIVertexType::WEIGHTED) ? CLIVertexType::WEIGHTED : CLIVertexType::UNWEIGHTED;
        }
        return current;
    }
    
    static CLIEdgeType promote_edge_type(CLIEdgeType current, CLIEdgeType required) {
        if (required == CLIEdgeType::UNWEIGHTED) {
            return CLIEdgeType::UNWEIGHTED;
        }
        if (required == CLIEdgeType::WEIGHTED) {
            return (current == CLIEdgeType::WEIGHTED) ? CLIEdgeType::WEIGHTED : CLIEdgeType::UNWEIGHTED;
        }
        return current;
    }
};

#endif // TYPE_PROMOTER_H_ 