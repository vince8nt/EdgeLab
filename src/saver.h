#ifndef SAVER_H_
#define SAVER_H_

#include <fstream>
#include <iostream>
#include <filesystem>
#include "util.h"
#include "graph_comp.h"
#include "graph.h"
#include "loader.h"
#include <cassert>

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

        // Open file for writing (binary mode for CG, text mode for others)
        std::ofstream file;
        if (file_type == FileType::CG) {
            file.open(filepath, std::ios::binary);
        } else {
            file.open(filepath);
        }
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
            case FileType::CG:
                save_CG(graph, file);
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
            
            case FileType::CG:
                // CG: supports all types
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

    // Save CG format
    // most space and time efficient way to store and load graphs
    void save_CG(const Graph<Vertex_t, Edge_t, Graph_t>& graph, std::ofstream& file) {
        bool directed = Graph_t == GraphType::DIRECTED;
        bool weighted_vertices = WeightedVertexType<Vertex_t>;
        bool weighted_edges = WeightedEdgeType<Edge_t>;
        bool unused = false;
        vertex_ID_t num_vertices = graph.num_vertices();
        edge_ID_t num_edges = directed ? graph.num_edges() : graph.num_edges() * 2;
        file.write(reinterpret_cast<const char*>(&directed), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&weighted_vertices), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&weighted_edges), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&unused), sizeof(bool));
        file.write(reinterpret_cast<const char*>(&num_vertices), sizeof(vertex_ID_t));
        file.write(reinterpret_cast<const char*>(&num_edges), sizeof(edge_ID_t));
        
        // write vertices
        // TODO: do in chunks for better cache locality and to use less memory
        constexpr size_t v_write_size = sizeof(vertex_ID_t) +
            (WeightedVertexType<Vertex_t> ? sizeof(weight_t) : 0);
        size_t v_write_amount = num_vertices * v_write_size;
        char* v_write_buffer = new char[v_write_amount];
        char* v_write_loc = v_write_buffer;
        for (vertex_ID_t i = 0; i < num_vertices; i++) {
            if constexpr (WeightedVertexType<Vertex_t>) {
                *reinterpret_cast<weight_t*>(v_write_loc) = graph[i].weight();
                v_write_loc += sizeof(weight_t);
            }
            *reinterpret_cast<vertex_ID_t*>(v_write_loc) = graph[i].degree();
            v_write_loc += sizeof(vertex_ID_t);
        }
        file.write(v_write_buffer, v_write_amount);
        delete[] v_write_buffer;
        
        // write edges
        constexpr size_t e_write_size = sizeof(vertex_ID_t) + 
            (WeightedEdgeType<Edge_t> ? sizeof(weight_t) : 0);
        // write edges directly into file (doesn't work for DataEdges)
        if constexpr (e_write_size == sizeof(Edge_t)) {
            if constexpr (Graph_t == GraphType::DIRECTED) {
                file.write(reinterpret_cast<const char*>(graph[0].begin()), e_write_size * num_edges);
            }
            else {
                for (vertex_ID_t i = 0; i < num_vertices; i++) {
                    Edge_t* it = graph[i].begin();
                    while (it != graph[i].end() and it->dest() <= i)
                        it++;
                    if (it == graph[i].end())
                        continue;
                    size_t write_amount = std::distance(it, graph[i].end()) * e_write_size;
                    file.write(reinterpret_cast<char*>(it), write_amount);
                }
            }
        }
        else {
            if constexpr (DataEdgeType<Edge_t>)
                std::cerr << "Data Edge writing currently unsupported by CG" << std::endl;
            else
                std::cerr << "Uncompacted Edge Struct Error" << std::endl;
            exit(1);
        }
    }
};

#endif // SAVER_H_ 