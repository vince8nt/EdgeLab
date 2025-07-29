#ifndef SAVER_H_
#define SAVER_H_

#include <fstream>
#include <iostream>
#include <filesystem>
#include "util.h"
#include "graph_comp.h"
#include "graph.h"
#include "loader.h"

template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
class Saver {
public:
    Saver() = default;

    void save_to_file(const Graph<Vertex_t, Edge_t, Graph_t>& graph, const std::string& filepath) {
        // Determine file type from extension
        FileType file_type = GetFileExtension(filepath);
        
        // Check compatibility
        if (!is_compatible(graph, file_type)) {
            std::cerr << "Error: Graph type is not compatible with " << file_type << " format" << std::endl;
            std::cerr << "  - Graph: " << Graph_t << ", " 
                      << (WeightedVertexType<Vertex_t> ? "weighted" : "unweighted") << " vertices, "
                      << (WeightedEdgeType<Edge_t> ? "weighted" : "unweighted") << " edges" << std::endl;
            exit(1);
        }

        // Open file for writing
        std::ofstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file for writing: " << filepath << std::endl;
            exit(1);
        }

        // Dispatch based on file type
        switch (file_type) {
            case FileType::EL:
                save_EL(graph, file);
                break;
            case FileType::WEL:
                save_WEL(graph, file);
                break;
            case FileType::VEL:
                save_VEL(graph, file);
                break;
            case FileType::VWEL:
                save_VWEL(graph, file);
                break;
            case FileType::GRAPH:
                std::cerr << "GRAPH format saving not implemented yet" << std::endl;
                exit(1);
            case FileType::ELAB:
                save_ELAB(graph, file);
                break;
            default:
                std::cerr << "Unsupported file type: " << file_type << std::endl;
                exit(1);
        }

        file.close();
    }

private:
    // Check if graph type is compatible with file format
    bool is_compatible(const Graph<Vertex_t, Edge_t, Graph_t>& graph, FileType file_type) {
        switch (file_type) {
            case FileType::EL:
                // EL: unweighted edges and vertices, both directed/undirected
                return !WeightedVertexType<Vertex_t> && !WeightedEdgeType<Edge_t>;
            
            case FileType::WEL:
                // WEL: weighted edges, unweighted vertices, both directed/undirected
                return !WeightedVertexType<Vertex_t> && WeightedEdgeType<Edge_t>;
            
            case FileType::VEL:
                // VEL: unweighted edges, weighted vertices, both directed/undirected
                return WeightedVertexType<Vertex_t> && !WeightedEdgeType<Edge_t>;
            
            case FileType::VWEL:
                // VWEL: weighted edges and vertices, both directed/undirected
                return WeightedVertexType<Vertex_t> && WeightedEdgeType<Edge_t>;
            
            case FileType::GRAPH:
                // GRAPH: both weighted/unweighted, directed only
                return Graph_t == GraphType::DIRECTED;
            
            case FileType::ELAB:
                // ELAB: supports all types
                return true;
            
            default:
                return false;
        }
    }

    // Save EL format (unweighted edges and vertices)
    void save_EL(const Graph<Vertex_t, Edge_t, Graph_t>& graph, std::ofstream& file) {
        // Write header comment for graph type
        if constexpr (Graph_t == GraphType::DIRECTED) {
            file << "# directed" << std::endl;
        }
        
        // Write edges
        for (vertex_ID_t u = 0; u < graph.num_vertices(); u++) {
            for (const auto& edge : graph[u]) {
                vertex_ID_t v = edge.dest();
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    // For undirected, only write edges where u <= v to avoid duplicates
                    if (u <= v) {
                        file << u << " " << v << std::endl;
                    }
                } else {
                    // For directed, write all edges
                    file << u << " " << v << std::endl;
                }
            }
        }
    }

    // Save WEL format (weighted edges, unweighted vertices)
    void save_WEL(const Graph<Vertex_t, Edge_t, Graph_t>& graph, std::ofstream& file) {
        // Write header comment for graph type
        if constexpr (Graph_t == GraphType::DIRECTED) {
            file << "# directed" << std::endl;
        }
        
        // Write edges with weights
        for (vertex_ID_t u = 0; u < graph.num_vertices(); u++) {
            for (const auto& edge : graph[u]) {
                vertex_ID_t v = edge.dest();
                weight_t w = edge.weight();
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    // For undirected, only write edges where u <= v to avoid duplicates
                    if (u <= v) {
                        file << u << " " << v << " " << w << std::endl;
                    }
                } else {
                    // For directed, write all edges
                    file << u << " " << v << " " << w << std::endl;
                }
            }
        }
    }

    // Save VEL format (unweighted edges, weighted vertices)
    void save_VEL(const Graph<Vertex_t, Edge_t, Graph_t>& graph, std::ofstream& file) {
        // Write header comment for graph type
        if constexpr (Graph_t == GraphType::DIRECTED) {
            file << "# directed" << std::endl;
        }
        
        // Write vertex weights
        for (vertex_ID_t v = 0; v < graph.num_vertices(); v++) {
            weight_t w = graph[v].weight();
            file << "v " << v << " " << w << std::endl;
        }
        
        // Write edges
        for (vertex_ID_t u = 0; u < graph.num_vertices(); u++) {
            for (const auto& edge : graph[u]) {
                vertex_ID_t v = edge.dest();
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    // For undirected, only write edges where u <= v to avoid duplicates
                    if (u <= v) {
                        file << u << " " << v << std::endl;
                    }
                } else {
                    // For directed, write all edges
                    file << u << " " << v << std::endl;
                }
            }
        }
    }

    // Save VWEL format (weighted edges and vertices)
    void save_VWEL(const Graph<Vertex_t, Edge_t, Graph_t>& graph, std::ofstream& file) {
        // Write header comment for graph type
        if constexpr (Graph_t == GraphType::DIRECTED) {
            file << "# directed" << std::endl;
        }
        
        // Write vertex weights
        for (vertex_ID_t v = 0; v < graph.num_vertices(); v++) {
            weight_t w = graph[v].weight();
            file << "v " << v << " " << w << std::endl;
        }
        
        // Write edges with weights
        for (vertex_ID_t u = 0; u < graph.num_vertices(); u++) {
            for (const auto& edge : graph[u]) {
                vertex_ID_t v = edge.dest();
                weight_t w = edge.weight();
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    // For undirected, only write edges where u <= v to avoid duplicates
                    if (u <= v) {
                        file << u << " " << v << " " << w << std::endl;
                    }
                } else {
                    // For directed, write all edges
                    file << u << " " << v << " " << w << std::endl;
                }
            }
        }
    }

    // Save ELAB format (binary format)
    void save_ELAB(const Graph<Vertex_t, Edge_t, Graph_t>& graph, std::ofstream& file) {
        // Write header: flags(3 bytes) + num_vertices(4 bytes) + num_edges(8 bytes)
        bool directed = (Graph_t == GraphType::DIRECTED);
        bool v_weights = WeightedVertexType<Vertex_t>;
        bool e_weights = WeightedEdgeType<Edge_t>;
        
        file.write(reinterpret_cast<const char*>(&directed), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&v_weights), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&e_weights), sizeof(bool));
        
        vertex_ID_t num_vertices = graph.num_vertices();
        edge_ID_t num_edges = graph.num_edges();
        if constexpr (Graph_t == GraphType::UNDIRECTED) {
            // graph returns num directed edges / 2
            // we will save this number of edges (1 direction for each undirected edge)
            // but the 'num_edges' value we save is actually
                // the total number of directed edges there will be after symmetrizing
            num_edges = num_edges * 2;
        }
        
        file.write(reinterpret_cast<const char*>(&num_vertices), sizeof(vertex_ID_t));
        file.write(reinterpret_cast<const char*>(&num_edges), sizeof(edge_ID_t));
        
        // Write vertex information
        edge_ID_t cumulative_edges = 0;
        for (vertex_ID_t v = 0; v <= num_vertices; v++) {
            if constexpr (WeightedVertexType<Vertex_t>) {
                weight_t weight = (v < num_vertices) ? graph[v].weight() : 0;
                file.write(reinterpret_cast<const char*>(&weight), sizeof(weight_t));
            }
            
            edge_ID_t edges_offset = (v < num_vertices) ? cumulative_edges : num_edges;
            file.write(reinterpret_cast<const char*>(&edges_offset), sizeof(edge_ID_t));
            
            if (v < num_vertices) {
                cumulative_edges += graph[v].degree();
            }
        }
        
        // Write edge information
        for (vertex_ID_t v = 0; v < num_vertices; v++) {
            for (const auto& edge : graph[v]) {
                vertex_ID_t dest = edge.dest();
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    // For undirected, only write edges where v < dest to avoid duplicates
                    if (v < dest) {
                        if constexpr (WeightedEdgeType<Edge_t>) {
                            weight_t weight = edge.weight();
                            file.write(reinterpret_cast<const char*>(&weight), sizeof(weight_t));
                        }
                        file.write(reinterpret_cast<const char*>(&dest), sizeof(vertex_ID_t));
                    }
                } else {
                    // For directed, write all edges
                    if constexpr (WeightedEdgeType<Edge_t>) {
                        weight_t weight = edge.weight();
                        file.write(reinterpret_cast<const char*>(&weight), sizeof(weight_t));
                    }
                    file.write(reinterpret_cast<const char*>(&dest), sizeof(vertex_ID_t));
                }
            }
        }
    }
};

#endif // SAVER_H_ 