#ifndef LOADER_H_
#define LOADER_H_

#include "graph_comp.h"

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <tuple>
#include <algorithm>

enum class FileType {
    EL,
    WEL,
    SG,
    S6
};
std::ostream& operator<<(std::ostream& os, FileType file_type) {
    switch (file_type) {
        case FileType::EL: os << "EL"; break;
        case FileType::WEL: os << "WEL"; break;
        case FileType::SG: os << "SG"; break;
        case FileType::S6: os << "S6"; break;
        default: os << "Unknown File Type"; break;
    }
    return os;
}

class Loader {
public:
    Loader() = default;

    // Main entry: takes a file path, checks existence, determines extension, and sets opts accordingly
    void load_graph_header(CLIOptions& opts) {
        file_type = GetFileExtension(opts.load_file_path);
        // open file
        file = std::ifstream(opts.load_file_path);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << opts.load_file_path << std::endl;
            exit(1);
        }

        // dispatch based on file extension
        switch (file_type) {
            case FileType::EL:
                opts.edge_type = CLIEdgeType::UNWEIGHTED;
                break;
            case FileType::WEL:
                opts.edge_type = CLIEdgeType::WEIGHTED;
                break;
            case FileType::SG:
                bool directed;
                file.read(reinterpret_cast<char*>(&directed), sizeof(bool));
                opts.graph_type = directed ? GraphType::DIRECTED : GraphType::UNDIRECTED;
                break;
            // case FileType::S6:
                // load_head_Sparse6(file, opts);
                // break;
            default:
                std::cerr << "Unsupported file extension: " << file_type << std::endl;
                exit(1);
        }
    }

    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    VectorGraph<Vertex_t, Edge_t> LoadGraphBody() {
        // dispatch based on file extension
        switch (file_type) {
            case FileType::EL:
                return load_body_EL<Vertex_t, Edge_t, Graph_t>();
            case FileType::WEL:
                return load_body_EL<Vertex_t, Edge_t, Graph_t>();
            case FileType::SG:
                return load_body_SG<Vertex_t, Edge_t, Graph_t>();
            // case "s6":
                // load_head_Sparse6(file, opts);
                // break;
            default:
                std::cerr << "Unsupported file extension: " << file_type << std::endl;
                exit(1);
        }
    }

private:
    
    // Helper to get file extension
    FileType GetFileExtension(const std::string& filepath) const {
        std::filesystem::path p(filepath);
        std::string ext = p.extension().string();
        if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
        if (ext == "el") return FileType::EL; // maybe use an unordered map if we add more file types
        if (ext == "wel") return FileType::WEL;
        if (ext == "sg") return FileType::SG;
        if (ext == "s6") return FileType::S6;
        std::cerr << "Unsupported file extension: " << ext << std::endl;
        exit(1);
    }

    // Header readers 
    void load_head_Sparse6(CLIOptions& opts); // .S6, .s6 // implement later

    // Body readers
    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    VectorGraph<Vertex_t, Edge_t> load_body_EL() {
        VectorGraph<Vertex_t, Edge_t> vg;
        AdjacencyMatrix<Edge_t> &matrix = vg.matrix;
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream iss(line); // parse line
            vertex_ID_t u, v;
            float w = 1.0f;
            if constexpr (WeightedEdgeType<Edge_t>) {
                if (!(iss >> u >> v >> w)) {
                    std::cerr << "Invalid edge line (expected src dest weight): " << line << std::endl;
                    continue;
                }
                if (v < u)
                    std::swap(u, v);
                while (matrix.size() <= u)
                    matrix.push_back({});
                matrix[u].push_back({v, w});
            } else {
                if (!(iss >> u >> v)) {
                    std::cerr << "Invalid edge line (expected src dest): " << line << std::endl;
                    continue;
                }
                if (v < u)
                    std::swap(u, v);
                while (matrix.size() <= u)
                    matrix.push_back({});
                matrix[u].push_back({v});
            }
        }
        return vg;
    }

    // TODO: make this directly return a graph object (bypassing Builder)
    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    VectorGraph<Vertex_t, Edge_t> load_body_SG() {
        VectorGraph<Vertex_t, Edge_t> vg;
        AdjacencyMatrix<Edge_t> &matrix = vg.matrix;
        size_t num_edges, num_nodes;
        file.read(reinterpret_cast<char*>(&num_edges), sizeof(size_t));
        file.read(reinterpret_cast<char*>(&num_nodes), sizeof(size_t));
        matrix.resize(num_nodes);
        
        return vg;
    }

    // Add more as needed

    // File state
    std::ifstream file;
    FileType file_type;
};

#endif // LOADER_H_
