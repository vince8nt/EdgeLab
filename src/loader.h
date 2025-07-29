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
//  EXT    //    data    |        format        |   graph_t   |   edge_t   |  vertex_t  
// --------++------------+----------------------+-------------+------------+-------------
    EL,    // plain text | edge list            | both        | unweighted | unweighted   
    WEL,   // plain text | edge list            | both        | weighted   | unweighted    
    VEL,   // plain text | edge list            | both        | unweighted | weighted       
    VWEL,  // plain text | edge list            | both        | weighted   | weighted       
    GRAPH, // plain text | prebuilt             | directed    | both       | both           
    ELAB   // binary     | prebuilt/ half built | both        | both       | both         
};
std::ostream& operator<<(std::ostream& os, FileType file_type) {
    switch (file_type) {
        case FileType::EL: os << "EL(Edge List)"; break;
        case FileType::WEL: os << "WEL(Weighted Edge List)"; break;
        case FileType::VEL: os << "VEL(Edge List with Vertex Weights)"; break;
        case FileType::VWEL: os << "VWEL(Weighted Edge List with Vertex Weights)"; break;
        case FileType::GRAPH: os << "GRAPH(METIS Graph)"; break;
        case FileType::ELAB: os << "ELAB(EdgeLab Graph)"; break;
        default: os << "Unknown File Type"; break;
    }
    return os;
}

class Loader {
public:
    Loader() = default;

    // Main entry: takes a file path, checks existence, determines extension, and sets opts accordingly
    void load_graph_header(CLIOptions& opts) {
        file_type_ = GetFileExtension(opts.load_file_path);
        // open file
        file_ = std::ifstream(opts.load_file_path);
        if (!file_.is_open()) {
            std::cerr << "Failed to open file: " << opts.load_file_path << std::endl;
            exit(1);
        }

        // dispatch based on file extension
        switch (file_type_) {
            case FileType::EL:
                load_head_EL(opts);
                break;
            case FileType::WEL:
                opts.edge_type = CLIEdgeType::WEIGHTED;
                load_head_EL(opts);
                break;
            case FileType::VEL:
                opts.vertex_type = CLIVertexType::WEIGHTED;
                load_head_EL(opts);
                break;
            case FileType::VWEL:
                opts.vertex_type = CLIVertexType::WEIGHTED;
                opts.edge_type = CLIEdgeType::WEIGHTED;
                load_head_EL(opts);
                break;
            case FileType::GRAPH:
                load_head_GRAPH(opts);
                break;
            case FileType::ELAB:
                load_head_ELAB(opts);
                break;
            default:
                std::cerr << "Unsupported file extension: " << file_type_ << std::endl;
                exit(1);
        }
    }

    // Load graph vertices and edges from file (after being templated by cli_dispatch)
    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> LoadGraphBody() {
        // dispatch based on file extension
        switch (file_type_) {
            case FileType::EL:
                return load_body_EL<Vertex_t, Edge_t, Graph_t>();
            case FileType::WEL:
                return load_body_EL<Vertex_t, Edge_t, Graph_t>();
            case FileType::VEL:
                return load_body_EL<Vertex_t, Edge_t, Graph_t>();
            case FileType::VWEL:
                return load_body_EL<Vertex_t, Edge_t, Graph_t>();
            case FileType::GRAPH:
                // return load_body_GRAPH<Vertex_t, Edge_t, Graph_t>();
                std::cerr << "GRAPH format not supported yet" << std::endl;
                exit(1);
            case FileType::ELAB:
                return load_body_ELAB<Vertex_t, Edge_t, Graph_t>();
            default:
                std::cerr << "Unsupported file extension: " << file_type_ << std::endl;
                exit(1);
        }
    }

private:
    // File state
    std::ifstream file_;
    FileType file_type_;
    vertex_ID_t num_vertices_;
    edge_ID_t num_edges_;

    // Helper to get file extension
    FileType GetFileExtension(const std::string& filepath) const {
        std::filesystem::path p(filepath);
        std::string ext = p.extension().string();
        if (!ext.empty() && ext[0] == '.') ext = ext.substr(1);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
        if (ext == "el") return FileType::EL; // maybe use an unordered map if we add more file types
        if (ext == "wel") return FileType::WEL;
        if (ext == "vel") return FileType::VEL;
        if (ext == "vwel") return FileType::VWEL;
        if (ext == "graph") return FileType::GRAPH;
        if (ext == "elab") return FileType::ELAB;
        std::cerr << "Unsupported file extension: " << ext << std::endl;
        exit(1);
    }

    // Header readers -------------------------------------------------------------------------------------------
    void load_head_EL(CLIOptions& opts) {
        std::string line;
        std::getline(file_, line);
        std::istringstream iss(line);
        std::string first, second;
        if (!(iss >> first >> second)) {
            std::cerr << "Invalid Edge List line: " << line << std::endl;
            exit(1);
        }
        if (first == "#" and second == "directed")
            opts.graph_type = GraphType::DIRECTED;
        // seek back to beginning of file
        file_.seekg(0, std::ios::beg);
    }

    void load_head_GRAPH(CLIOptions& opts) {
        opts.graph_type = GraphType::UNDIRECTED;
        std::string line;
        std::getline(file_, line);
        std::istringstream iss(line);
        std::string num_vertices, num_edges, flags;
        file_.seekg(0, std::ios::beg);
        if (!(iss >> num_vertices >> num_edges)) {
            std::cerr << "Invalid graph header: " << line << std::endl;
            exit(1);
        }
        if (iss >> flags) {
            if (flags.size() > 0 and flags[0] == '1')
                opts.vertex_type = CLIVertexType::WEIGHTED;
            if (flags.size() > 1 and flags[1] == '1')
                opts.edge_type = CLIEdgeType::WEIGHTED;
        }
        num_vertices_ = std::stoi(num_vertices);
        num_edges_ = std::stoi(num_edges);
    }

    void load_head_ELAB(CLIOptions& opts) {
        bool directed, v_weights, e_weights;
        file_.read(reinterpret_cast<char*>(&directed), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&v_weights), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&e_weights), sizeof(bool));
        opts.graph_type = directed ? GraphType::DIRECTED : GraphType::UNDIRECTED;
        opts.vertex_type = v_weights ? CLIVertexType::WEIGHTED : CLIVertexType::UNWEIGHTED;
        opts.edge_type = e_weights ? CLIEdgeType::WEIGHTED : CLIEdgeType::UNWEIGHTED;
        file_.read(reinterpret_cast<char*>(&num_vertices_), sizeof(vertex_ID_t));
        file_.read(reinterpret_cast<char*>(&num_edges_), sizeof(edge_ID_t));
    }


    // Body readers ----------------------------------------------------------------------------------------------
    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_body_EL() {
        VectorGraph<Vertex_t, Edge_t> vg;
        AdjacencyMatrix<Edge_t> &matrix = vg.matrix;
        std::string line;
        while (std::getline(file_, line)) {
            std::istringstream iss(line); // parse line
            std::string first;
            if (!(iss >> first)) continue;
            if (first[0] == '#') continue;
            vertex_ID_t v;
            weight_t w;
            if constexpr (WeightedVertexType<Vertex_t>) {
                if (first == "v") {
                    if (!(iss >> v >> w)) {
                        std::cerr << "Invalid vertex line (expected id weight): " << line << std::endl;
                        exit(1);
                    }
                    while (matrix.size() <= v) {
                        matrix.push_back({});
                        vg.vertices.push_back(Vertex_t());
                    }
                    new (&vg.vertices[v]) Vertex_t(w);
                    continue;
                }
            }
            vertex_ID_t u = atoi(first.c_str());
            if constexpr (WeightedEdgeType<Edge_t>) {
                if (!(iss >> v >> w)) {
                    std::cerr << "Invalid edge line (expected src dest weight): " << line << std::endl;
                    exit(1);
                }
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (u == v)
                        continue;
                    if (u > v)
                        std::swap(u, v);
                    while (matrix.size() <= v) {
                        matrix.push_back({});
                        if constexpr (NonEmptyVertexType<Vertex_t>)
                            vg.vertices.push_back(Vertex_t());
                    }
                    matrix[u].push_back({v, w});
                }
            } else {
                if (!(iss >> v)) {
                    std::cerr << "Invalid edge line (expected src dest): " << line << std::endl;
                    exit(1);
                }
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (u == v)
                        continue;
                    if (u > v)
                        std::swap(u, v);
                    while (matrix.size() <= v) {
                        matrix.push_back({});
                        if constexpr (NonEmptyVertexType<Vertex_t>)
                            vg.vertices.push_back(Vertex_t());
                    }
                    matrix[u].push_back({v});
                }
            }
        }
        file_.close();

        // build graph
        Builder<Vertex_t, Edge_t, Graph_t> builder;
        Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
        return graph;
    }

    // Load ELAB format
    // most space and time efficient way to store and load graphs
    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_body_ELAB() {
        using Vertex = typename Graph<Vertex_t, Edge_t, Graph_t>::Vertex;
        
        Vertex* vertices = (Vertex*)malloc((num_vertices_ + 1) * sizeof(Vertex));
        Edge_t* edges = (Edge_t*)malloc(num_edges_ * sizeof(Edge_t));
        std::vector<Edge_t*> offsets; // used for symmetrizing undirected graphs
        offsets.reserve(num_vertices_ + 1);

        // read vertices
        constexpr size_t v_read_size = sizeof(edge_ID_t) + 
            (WeightedVertexType<Vertex_t> ? sizeof(weight_t) : 0);
        // read vertices directly into CSR (doesn't work for DataVertices)
        if constexpr (v_read_size == sizeof(Vertex)) {
            file_.read(reinterpret_cast<char*>(vertices), v_read_size * (num_vertices_ + 1));
            // set edge offsets
            for (vertex_ID_t i = 0; i < num_vertices_ + 1; i++) {
                vertices[i].edges_begin_ = edges + reinterpret_cast<edge_ID_t>(vertices[i].edges_begin_);
                if constexpr (Graph_t == GraphType::UNDIRECTED)
                    offsets.push_back(const_cast<Edge_t*>(vertices[i].edges_begin_));
            }
        }
        // otherwise read 1 at a time (change this later to a buffered approach)
        else {
            for (vertex_ID_t i = 0; i < num_vertices_ + 1; i++) {
                static_assert(DataVertexType<Vertex_t>, "Uncompacted Vertex");
                edge_ID_t edges_offset;
                if constexpr (WeightedVertexType<Vertex_t>) {
                    weight_t w;
                    file_.read(reinterpret_cast<char*>(&w), sizeof(weight_t));
                    file_.read(reinterpret_cast<char*>(&edges_offset), sizeof(edge_ID_t));
                    new (&vertices[i]) Vertex(w, edges + edges_offset);
                } else {
                    file_.read(reinterpret_cast<char*>(&edges_offset), sizeof(edge_ID_t));
                    new (&vertices[i]) Vertex(edges + edges_offset);
                }
                if constexpr (Graph_t == GraphType::UNDIRECTED)
                    offsets.push_back(const_cast<Edge_t*>(vertices[i].edges_begin_));
            }
        }

        // read edges
        constexpr size_t e_read_size = sizeof(vertex_ID_t) + 
            (WeightedEdgeType<Edge_t> ? sizeof(weight_t) : 0);
        // read edges directly into CSR (doesn't work for DataEdges)
        if constexpr (e_read_size == sizeof(Edge_t)) {
            if constexpr (Graph_t == GraphType::DIRECTED) {
                file_.read(reinterpret_cast<char*>(edges), e_read_size * num_edges_);
            }
            else {
                for (vertex_ID_t i = 0; i < num_vertices_; i++) {
                    vertex_ID_t read_amount =
                        std::distance(offsets[i], const_cast<Edge_t*>(vertices[i].edges_begin_));
                    file_.read(reinterpret_cast<char*>(offsets[i]), read_amount);
                    for (vertex_ID_t j = 0; j < read_amount; j++) {
                        Edge_t edge = *offsets[i+j];
                        *(offsets[edge.dest()]++) = edge.inverse(i);
                    }
                }
            }
        }
        // otherwise read 1 at a time (change this later to a buffered approach)
        else {
            static_assert(DataEdgeType<Edge_t>, "Uncompacted Edge");
            if constexpr (Graph_t == GraphType::DIRECTED) {
                for (edge_ID_t i = 0; i < num_edges_; i++) {
                    vertex_ID_t dest;
                    weight_t w;
                    file_.read(reinterpret_cast<char*>(&dest), sizeof(vertex_ID_t));
                    if constexpr (WeightedEdgeType<Edge_t>) {
                        file_.read(reinterpret_cast<char*>(&w), sizeof(weight_t));
                        new (&edges[i]) Edge_t(dest, w);
                    }
                    else {
                        new (&edges[i]) Edge_t(dest);
                    }
                }
            }
            
            else {
                for (vertex_ID_t i = 0; i < num_vertices_; i++) {
                    vertex_ID_t read_amount =
                        std::distance(offsets[i], const_cast<Edge_t*>(vertices[i].edges_begin_));
                    for (vertex_ID_t j = 0; j < read_amount; j++) {
                        vertex_ID_t dest;
                        weight_t w;
                        file_.read(reinterpret_cast<char*>(&dest), sizeof(vertex_ID_t));
                        if constexpr (WeightedEdgeType<Edge_t>) {
                            file_.read(reinterpret_cast<char*>(&w), sizeof(weight_t));
                            new (offsets[i]++) Edge_t(dest, w);
                        }
                        else {
                            new (offsets[i]++) Edge_t(dest);
                        }
                        Edge_t edge = *offsets[i+j];
                        *(offsets[edge.dest()]++) = edge.inverse(i);
                    }
                }
            }
        }


        file_.close();
        return Graph<Vertex_t, Edge_t, Graph_t>(num_vertices_, vertices, num_edges_, edges);
    }

    // Add more as needed

};

#endif // LOADER_H_

