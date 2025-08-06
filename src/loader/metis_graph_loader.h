#ifndef METIS_GRAPH_LOADER_H_
#define METIS_GRAPH_LOADER_H_

#include "loader_base.h"


class MetisGraphLoader : public LoaderBase {
public:
    MetisGraphLoader(FileType file_type) : LoaderBase(file_type) {}
    ~MetisGraphLoader() = default;

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

        graph_type_ = GraphType::UNDIRECTED;
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
                vertex_type_ = CLIVertexType::WEIGHTED;
            if (flags.size() > 1 and flags[1] == '1')
                edge_type_ = CLIEdgeType::WEIGHTED;
        }
        num_vertices_ = std::stoi(num_vertices);
        num_edges_ = std::stoi(num_edges);

        opts.graph_type = graph_type_;
        opts.vertex_type = vertex_type_;
        opts.edge_type = edge_type_;
    }

    // Read GRAPH format
    // implement later
    template <VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_graph_body() {
        Builder<Vertex_t, Edge_t, Graph_t> builder;
        VectorGraph<Vertex_t, Edge_t> vg;
        return builder.BuildGraph(vg);
    }

private:
    vertex_ID_t num_vertices_;
    edge_ID_t num_edges_;
};

#endif // METIS_GRAPH_LOADER_H_
