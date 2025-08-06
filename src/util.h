#ifndef UTIL_H_
#define UTIL_H_

#include <cstdint>
#include <vector>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <type_traits>
#include <algorithm>
#include <concepts>
#include <chrono>
#include <memory>
#include <filesystem>
#include <cctype>

#define DEBUG 1 // 1 for debug, 0 for release



// Graph capacity specifications
// use unsigned integer types so that bit layout is well defined (for saving/loading binary files)
using vertex_ID_t = uint32_t;
using edge_ID_t = uint64_t;
using weight_t = uint32_t;
constexpr weight_t default_weight = 1;

// timer for benchmarking
// Use steady_clock if high_resolution_clock is not steady, otherwise use high_resolution_clock
inline auto timer_start() {
    if constexpr (std::chrono::high_resolution_clock::is_steady) {
        return std::chrono::high_resolution_clock::now();
    } else {
        return std::chrono::steady_clock::now();
    }
}

inline double timer_stop(const auto &start) {
    if constexpr (std::chrono::high_resolution_clock::is_steady) {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
    } else {
        auto end = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
    }
}

// specification of graph types
enum class GraphType {
    UNDIRECTED,
    DIRECTED,
    BIDIRECTED
};

inline std::ostream& operator<<(std::ostream& os, GraphType Graph_t) {
    switch (Graph_t) {
        case GraphType::UNDIRECTED:
            os << "Undirected";
            break;
        case GraphType::DIRECTED:
            os << "Directed";
            break;
        case GraphType::BIDIRECTED:
            os << "Bidirected";
            break;
        default:
            os << "Unknown Graph Type";
            break;
    }
    return os;
}

// specification of generation types
enum class GenType {
    ERDOS_RENYI,     // Erdos-Renyi-Gilbert
    WATTS_STROGATZ,  // Watts-Strogatz
    BARABASI_ALBERT, // Barabasi-Albert
};

inline std::ostream& operator<<(std::ostream& os, GenType Gen_t) {
    switch (Gen_t) {
        case GenType::ERDOS_RENYI:
            os << "Erdos-Renyi";
            break;
        case GenType::WATTS_STROGATZ:
            os << "Watts-Strogatz";
            break;
        case GenType::BARABASI_ALBERT:
            os << "Barabasi-Albert";
            break;
        default:
            os << "Unknown Generation Type";
            break;
    }
    return os;
}

// Enum definitions for CLI
enum class CLIVertexType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA
};

enum class CLIEdgeType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA
};

struct CLIOptions {
    GraphType graph_type = GraphType::UNDIRECTED;
    CLIVertexType vertex_type = CLIVertexType::UNWEIGHTED;
    CLIEdgeType edge_type = CLIEdgeType::UNWEIGHTED;
    int scale;
    int degree;
    GenType gen_type;
    std::string load_file_path; // Path to load file, mutually exclusive
    std::string save_file_path; // Path to save file (optional)
};

struct AlgorithmReqs {
    GraphType graph_type = GraphType::DIRECTED;
    CLIVertexType vertex_type = CLIVertexType::UNWEIGHTED;
    CLIEdgeType edge_type = CLIEdgeType::UNWEIGHTED;
};

// File types
enum class FileType {
//  EXT    //    data    |        format        |   graph_t   |   edge_t   |  vertex_t  
// --------++------------+----------------------+-------------+------------+-------------
    EL,    // plain text | edge list            | both        | unweighted | unweighted   
    WEL,   // plain text | edge list            | both        | weighted   | unweighted    
    VEL,   // plain text | edge list            | both        | unweighted | weighted       
    VWEL,  // plain text | edge list            | both        | weighted   | weighted       
    GRAPH, // plain text | prebuilt             | directed    | both       | both           
    CG     // binary     | prebuilt/ half built | both        | both       | both         
};
inline std::ostream& operator<<(std::ostream& os, FileType file_type) {
    switch (file_type) {
        case FileType::EL: os << "EL(Edge List)"; break;
        case FileType::WEL: os << "WEL(Weighted Edge List)"; break;
        case FileType::VEL: os << "VEL(Edge List with Vertex Weights)"; break;
        case FileType::VWEL: os << "VWEL(Weighted Edge List with Vertex Weights)"; break;
        case FileType::GRAPH: os << "GRAPH(METIS Graph)"; break;
        case FileType::CG: os << "CG(Compacted Graph)"; break;
        default: os << "Unknown File Type"; break;
    }
    return os;
}
// Helper to get file extension
inline FileType GetFileExtension(const std::string& filepath) {
    std::filesystem::path p(filepath);
    std::string ext = p.extension().string();
    if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    if (ext == "el") return FileType::EL;
    if (ext == "wel") return FileType::WEL;
    if (ext == "vel") return FileType::VEL;
    if (ext == "vwel") return FileType::VWEL;
    if (ext == "graph") return FileType::GRAPH;
    if (ext == "cg") return FileType::CG;
    std::cerr << "Unsupported file extension: " << ext << std::endl;
    exit(1);
}


/*
enum class VertexType {
    UNWEIGHTED,
    WEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED_DATA,
};
enum class EdgeType {
    UNWEIGHTED,
    UNWEIGHTED_DATA,
    WEIGHTED,
    WEIGHTED_DATA
};
*/


#endif // UTIL_H_
