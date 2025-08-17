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
    template <VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_graph_body() {
        using Vertex = CSR_Vertex<Vertex_t, Edge_t, Graph_t>;
        // permenant graph memory allocation
        size_t vertices_size = (num_vertices_ + 1) * sizeof(Vertex);
        size_t edges_size = num_edges_ * sizeof(Edge_t);
        Vertex* vertices = (Vertex*)malloc(vertices_size);
        Edge_t* edges = (Edge_t*)malloc(edges_size);

        Edge_t* edges_current = edges;
        std::string line;
        for (vertex_ID_t i = 0; i < num_vertices_; i++) {
            std::getline(file_, line);
            std::istringstream iss(line);
            if constexpr (WeightedVertexType<Vertex_t>) {
                weight_t weight;
                iss >> weight;
                new (&vertices[i]) Vertex(weight, edges_current);
            }
            else {
                if constexpr (DataVertexType<Vertex_t>) {
                    new (&vertices[i]) Vertex(Vertex_t(typename Vertex_t::data_type{}), edges_current);
                } else {
                    new (&vertices[i]) Vertex(edges_current);
                }
            }
            if constexpr (WeightedEdgeType<Edge_t>) {
                weight_t weight;
                vertex_ID_t dest;
                while (iss >> weight >> dest) {
                    if constexpr (DataEdgeType<Edge_t>) {
                        new (&(edges_current++)[0]) Edge_t(dest-1, weight, typename Edge_t::data_type{});
                    } else {
                        new (&(edges_current++)[0]) Edge_t(dest-1, weight);
                    }
                }
            }
            else {
                vertex_ID_t dest;
                while (iss >> dest) {
                    if constexpr (DataEdgeType<Edge_t>) {
                        new (&(edges_current++)[0]) Edge_t(dest-1, typename Edge_t::data_type{});
                    } else {
                        new (&(edges_current++)[0]) Edge_t(dest-1);
                    }
                }
            }
        }
        if constexpr (DataVertexType<Vertex_t>) {
            new (&vertices[num_vertices_]) Vertex(Vertex_t(typename Vertex_t::data_type{}), edges_current);
        } else {
            if constexpr (WeightedVertexType<Vertex_t>) {
                new (&vertices[num_vertices_]) Vertex(0, edges_current);
            } else {
                new (&vertices[num_vertices_]) Vertex(edges_current);
            }
        }

        file_.close();
        return Graph<Vertex_t, Edge_t, Graph_t>(num_vertices_, vertices, num_edges_, edges);
    }

private:
    vertex_ID_t num_vertices_;
    edge_ID_t num_edges_;
};

#endif // METIS_GRAPH_LOADER_H_
