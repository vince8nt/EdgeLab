#ifndef EDGE_LIST_LOADER_H_
#define EDGE_LIST_LOADER_H_

#include "loader_base.h"


class EdgeListLoader : public LoaderBase {
public:
    EdgeListLoader(FileType file_type) : LoaderBase(file_type) {}
    ~EdgeListLoader() = default;

    void load_graph_header(CLIOptions& opts) {
        if (file_.is_open()) {
            std::cerr << "File already open" << std::endl;
            exit(1);
        }
        file_ = std::ifstream(opts.load_file_path);
        if (!file_.is_open()) {
            std::cerr << "Failed to open file: " << opts.load_file_path << std::endl;
            exit(1);
        }

        switch (file_type_) {
            case FileType::EL:
                vertex_type_ = CLIVertexType::UNWEIGHTED;
                edge_type_ = CLIEdgeType::UNWEIGHTED;
                break;
            case FileType::WEL:
                vertex_type_ = CLIVertexType::UNWEIGHTED;
                edge_type_ = CLIEdgeType::WEIGHTED;
                break;
            case FileType::VEL:
                vertex_type_ = CLIVertexType::WEIGHTED;
                edge_type_ = CLIEdgeType::UNWEIGHTED;
                break;
            case FileType::VWEL:
                vertex_type_ = CLIVertexType::WEIGHTED;
                edge_type_ = CLIEdgeType::WEIGHTED;
                break;
            default:
                std::cerr << "Not an edge list file: " << file_type_ << std::endl;
                exit(1);
        }
        graph_type_ = load_head_EL();

        opts.graph_type = graph_type_;
        opts.vertex_type = vertex_type_;
        opts.edge_type = edge_type_;
    }

    // Read EL format
    // Ignore graph_type_ (provided in file header) and use Graph_t instead
    // Ignore vertex_type_ (provided in file header) and use Vertex_t instead
    // Ignore edge_type_ (provided in file header) and use Edge_t instead
    template <VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_graph_body() {
        if (!file_.is_open()) {
            std::cerr << "File not open" << std::endl;
            std::cerr << "Must call load_graph_header before load_graph_body" << std::endl;
            exit(1);
        }

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
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[u].push_back(Edge_t(v, w, typename Edge_t::data_type{}));
                    } else {
                        matrix[u].push_back({v, w});
                    }
                } else {
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[u].push_back(Edge_t(v, w, typename Edge_t::data_type{}));
                    } else {
                        matrix[u].push_back({v, w});
                    }
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
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[u].push_back(Edge_t(v, typename Edge_t::data_type{}));
                    } else {
                        matrix[u].push_back({v});
                    }
                } else {
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[u].push_back(Edge_t(v, typename Edge_t::data_type{}));
                    } else {
                        matrix[u].push_back({v});
                    }
                }
            }
        }
        file_.close();

        // build graph
        Builder<Vertex_t, Edge_t, Graph_t> builder;
        Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
        return graph;
    }

private:

    // read the first line of the file to determine graph type
    // seek back to beginning of file since we may have read an edge
    GraphType load_head_EL() {
        std::string line;
        std::getline(file_, line);
        std::istringstream iss(line);
        std::string first, second;
        if (!(iss >> first >> second)) {
            std::cerr << "Invalid Edge List line: " << line << std::endl;
            exit(1);
        }
        file_.seekg(0, std::ios::beg);
        if (first == "#" and second == "undirected")
            return GraphType::UNDIRECTED;
        return GraphType::DIRECTED;
    }

};

#endif // EDGE_LIST_LOADER_H_
