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
#include <cassert>




class Loader {
public:
    Loader() = default;

    // Main entry: takes a file path, checks existence, determines extension, and sets opts accordingly
    void load_graph_header(CLIOptions& opts) {
        file_type_ = GetFileExtension(opts.load_file_path);
            // open file in binary mode for CG format
    if (file_type_ == FileType::CG) {
            file_ = std::ifstream(opts.load_file_path, std::ios::binary);
        } else {
            file_ = std::ifstream(opts.load_file_path);
        }
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
            case FileType::CG:
                load_head_CG(opts);
                break;
            default:
                std::cerr << "Unsupported file extension: " << file_type_ << std::endl;
                exit(1);
        }

        opts.graph_type = graph_type_;
        opts.vertex_type = vertex_type_;
        opts.edge_type = edge_type_;
    }

    // Load graph vertices and edges from file (after being templated by cli_dispatch)
    // Vertex_t is (vertex_type_) potentially with data
        // vertex_type_==VertexUW -> Vertex_t==VertexUW or VertexUWD
        // vertex_type_==VertexW -> Vertex_t==VertexW or VertexWD
    // Edge_t is (edge_type_) potentially with data
        // edge_type_==EdgeUW -> Edge_t==EdgeUW or EdgeUWD
        // edge_type_==EdgeW -> Edge_t==EdgeW or EdgeWD
    // Graph_t is a potentially promoted (graph_type_)
        // graph_type_==UNDIRECTED -> Graph_t==UNDIRECTED
        // graph_type_==DIRECTED -> Graph_t==DIRECTED or BIDIRECTED or UNDIRECTED
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
            case FileType::CG:
                return load_body_CG<Vertex_t, Edge_t, Graph_t>();
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
    GraphType graph_type_;
    CLIVertexType vertex_type_;
    CLIEdgeType edge_type_;

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

    void load_head_CG(CLIOptions& opts) {
        bool directed, v_weights, e_weights, unused;
        file_.read(reinterpret_cast<char*>(&directed), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&v_weights), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&e_weights), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&unused), sizeof(bool));
        graph_type_ = directed ? GraphType::DIRECTED : GraphType::UNDIRECTED;
        vertex_type_ = v_weights ? CLIVertexType::WEIGHTED : CLIVertexType::UNWEIGHTED;
        edge_type_ = e_weights ? CLIEdgeType::WEIGHTED : CLIEdgeType::UNWEIGHTED;
        file_.read(reinterpret_cast<char*>(&num_vertices_), sizeof(vertex_ID_t));
        file_.read(reinterpret_cast<char*>(&num_edges_), sizeof(edge_ID_t));
    }


    // Body readers ----------------------------------------------------------------------------------------------

    // Read EL format
    // Ignore graph_type_ (provided in file header) and use Graph_t instead
    // Ignore vertex_type_ (provided in file header) and use Vertex_t instead
    // Ignore edge_type_ (provided in file header) and use Edge_t instead
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
                // Ensure matrix is large enough for both vertices
                while (matrix.size() <= std::max(u, v)) {
                    matrix.push_back({});
                    if constexpr (NonEmptyVertexType<Vertex_t>)
                        vg.vertices.push_back(Vertex_t());
                }
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (u == v)
                        continue;
                    if (u > v)
                        std::swap(u, v);
                    matrix[u].push_back({v, w});
                } else {
                    matrix[u].push_back({v, w});
                }
            } else {
                if (!(iss >> v)) {
                    std::cerr << "Invalid edge line (expected src dest): " << line << std::endl;
                    exit(1);
                }
                // Ensure matrix is large enough for both vertices
                while (matrix.size() <= std::max(u, v)) {
                    matrix.push_back({});
                    if constexpr (NonEmptyVertexType<Vertex_t>)
                        vg.vertices.push_back(Vertex_t());
                }
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (u == v)
                        continue;
                    if (u > v)
                        std::swap(u, v);
                    matrix[u].push_back({v});
                } else {
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

    // Load CG format
    // most space and time efficient way to store and load graphs
    // Ignore vertex_type_ (provided in file header) and use Vertex_t instead
    // Ignore edge_type_ (provided in file header) and use Edge_t instead
        // Data edges will not allow for directly reading into CSR
    // graph_type_ (provided in file header) matters a lot
        // special cases for conversion (all cases use O(num vertices + num edges) time):
            // graph_type_==UNDIRECTED -> Graph_t==UNDIRECTED:
                // O(1) extra space, O(num vertices) read calls, O(num edges) random memory access
                    // read in vertices, and set edges_start
                    // read in edges 1 vertex at a time
                        // update edges_start for this vertex (by the number of read edges)
                        // add inverse edge to each dest (and increment edges_start for that dest)
                    // sweep vertices and reset edges_start in place
            // graph_type_==DIRECTED -> Graph_t==DIRECTED:
                // O(1) extra space, O(1) read calls, O(1) random memory access
                    // simple direct loading
            // graph_type_==DIRECTED -> Graph_t==BIDIRECTED:
                // O(1) extra space, O(1) read calls, O(num edges) random memory access
                    // read in vertices, and set edges_start
                        // also set inverse_start=32bit zero (use to store inverse degrees)
                    // read in edges
                    // sweep edges and count inverse degrees
                    // sweep vertices and set inverse_start in place
                    // sweep edges, adding inverse edge and incrementing inverse_start for each
                    // sweep vertices and reset inverse_start in place
            // graph_type_==DIRECTED -> Graph_t==UNDIRECTED:
                // least efficient way to load
                // usually will add edges via symmetrization
                // but need to account for a reduction in edges from the removal of self loops
                // O(num vertices + num edges) extra space, O(1) read calls, O(num edges) random memory access
                    // read in vertices, and set edges_start
                    // allocate temp memory for directed edges (and read them into this)
                    // allocate temp vector for undirected degree
                    
                        // preallocate memory for symmetrized edges (permanent memory)
                        // allocate inverse edges_offset vector (temp memory)
                        // allocate consensed inverse edges vector (temp memory)
                        // read in vertices
                            // calculate edges_start (pointing to placement in top half)
                        // read directed edges into top half of CSRedges
                            // (compute inverse degrees during this)
                        // translate inverse degrees to inverse edges_offset in place
                        // go through vertices (edges +inverse) and merge into bottom half of CSR
                            // remove duplicates and update edges_start for each
                    // 2. compute this on load (extra memory during loading)
                
    template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_body_CG() {
        using Vertex = CSR_Vertex<Vertex_t, Edge_t, Graph_t>;
        
        // permenant graph memory allocation
        size_t vertices_size = (num_vertices_ + 1) * sizeof(Vertex);
        size_t edges_size = num_edges_ * sizeof(Edge_t);
        Vertex* vertices = (Vertex*)malloc(vertices_size);
        Edge_t* edges = (Edge_t*)malloc(edges_size);

        // read vertex data into the upper portion of CSR
        // then overwrite with full vertex instantiations
        // TODO: change this to happen in chunks for better cache locality
        constexpr size_t v_read_size = sizeof(vertex_ID_t) +
            (WeightedVertexType<Vertex_t> ? sizeof(weight_t) : 0);
        size_t v_read_amount = num_vertices_ * v_read_size;
        char* v_read_loc = reinterpret_cast<char*>(vertices) + vertices_size - v_read_amount;
        file_.read(v_read_loc, v_read_amount);
        Edge_t* edges_offset = edges;
        for (vertex_ID_t i = 0; i < num_vertices_; i++) {
            vertex_ID_t degree;
            if constexpr (WeightedVertexType<Vertex_t>) {
                weight_t weight = *reinterpret_cast<weight_t*>(v_read_loc);
                v_read_loc += sizeof(weight_t);
                degree = *reinterpret_cast<vertex_ID_t*>(v_read_loc);
                v_read_loc += sizeof(vertex_ID_t);
                new (&vertices[i]) Vertex(weight, edges_offset);
            }
            else {
                degree = *reinterpret_cast<vertex_ID_t*>(v_read_loc);
                v_read_loc += sizeof(vertex_ID_t);
                new (&vertices[i]) Vertex(edges_offset);
            }
            edges_offset += degree;
        }
        // add ending vertex
        if constexpr (WeightedVertexType<Vertex_t>) {
            new (&vertices[num_vertices_]) Vertex(0, edges_offset);
        }
        else {
            new (&vertices[num_vertices_]) Vertex(edges_offset);
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
                // temporary memory used for symmetrizing undirected graphs
                std::vector<vertex_ID_t> pre_added_edges(num_vertices_);
                // TODO: test out modifying edge offsets in place and then correcting in a second pass
                    // less memory usage, but more time
                for (vertex_ID_t i = 0; i < num_vertices_; i++) {
                    Edge_t* edges_current = vertices[i].edges_begin_ + pre_added_edges[i];
                    Edge_t* edges_end = vertices[i + 1].edges_begin_;
                    vertex_ID_t read_amount = std::distance(edges_current, edges_end) * e_read_size;
                    if (read_amount == 0)
                        continue;
                    file_.read(reinterpret_cast<char*>(edges_current), read_amount);
                    for (Edge_t* it = edges_current; it != edges_end; it++) {
                        Edge_t &e = *it;
                        vertex_ID_t dest = e.dest();
                        vertices[dest].edges_begin_[pre_added_edges[dest]++] = e.inverse(i);
                    }
                }
            }
        }
        else {
            if constexpr (DataEdgeType<Edge_t>)
                std::cerr << "Data Edge loading currently unsupported by CG" << std::endl;
            else
                std::cerr << "Uncompacted Edge Struct Error" << std::endl;
            exit(1);
        }

        file_.close();
        return Graph<Vertex_t, Edge_t, Graph_t>(num_vertices_, vertices, num_edges_, edges);
    }

    // Add more as needed

};

#endif // LOADER_H_

