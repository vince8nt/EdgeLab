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
        // add data if algorithm specifies it
        // make vertices/edges unweighted if weights would go unused (auto_uw_promotion)
            // set auto_uw_promotion to false to disable this
        promoted_opts.vertex_type = promote_vertex_type(opts, AlgorithmReqsType::vertex_type);
        promoted_opts.edge_type = promote_edge_type(opts, AlgorithmReqsType::edge_type);
        
        return promoted_opts;
    }

private:
    // Promote graph type
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
    static CLIVertexType promote_vertex_type(const CLIOptions &opts, CLIVertexType required) {
        CLIVertexType current = opts.vertex_type;
        bool auto_uw_promotion = opts.auto_uw_promotion;
        if (required == CLIVertexType::UNWEIGHTED or required == CLIVertexType::WEIGHTED) {
            // non-data vertices
            if (!auto_uw_promotion) {
                return current;
            }
            if (required == CLIVertexType::WEIGHTED and current == CLIVertexType::WEIGHTED) {
                return CLIVertexType::WEIGHTED;
            }
            return CLIVertexType::UNWEIGHTED;
        }
        else {
            // data vertices
            if (!auto_uw_promotion) {
                if (current == CLIVertexType::WEIGHTED)
                    return CLIVertexType::WEIGHTED_DATA;
                return CLIVertexType::UNWEIGHTED_DATA;
            }
            if (required == CLIVertexType::WEIGHTED_DATA and current == CLIVertexType::WEIGHTED) {
                return CLIVertexType::WEIGHTED_DATA;
            }
            return CLIVertexType::UNWEIGHTED_DATA;
        }
    }

    // Promote edge type
    static CLIEdgeType promote_edge_type(const CLIOptions &opts, CLIEdgeType required) {
        CLIEdgeType current = opts.edge_type;
        bool auto_uw_promotion = opts.auto_uw_promotion;
        if (required == CLIEdgeType::UNWEIGHTED or required == CLIEdgeType::WEIGHTED) {
            // non-data edges
            if (!auto_uw_promotion) {
                return current;
            }
            if (required == CLIEdgeType::WEIGHTED and current == CLIEdgeType::WEIGHTED) {
                return CLIEdgeType::WEIGHTED;
            }
            return CLIEdgeType::UNWEIGHTED;
        }
        else {
            // data edges
            if (!auto_uw_promotion) {
                if (current == CLIEdgeType::WEIGHTED)
                    return CLIEdgeType::WEIGHTED_DATA;
                return CLIEdgeType::UNWEIGHTED_DATA;
            }
            if (required == CLIEdgeType::WEIGHTED_DATA and current == CLIEdgeType::WEIGHTED) {
                return CLIEdgeType::WEIGHTED_DATA;
            }
            return CLIEdgeType::UNWEIGHTED_DATA;
        }
    }

};

#endif // TYPE_PROMOTER_H_ 