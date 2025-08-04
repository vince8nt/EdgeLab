#ifndef LOADER_BASE_H_
#define LOADER_BASE_H_

class LoaderBase {
public:
    virtual ~LoaderBase() = default;
    virtual Graph<Vertex_t, Edge_t, Graph_t> LoadGraph() = 0;
    virtual void LoadHeader(CLIOptions& opts) = 0;
    virtual void LoadBody() = 0;

private:
    // File state information
    // may differ from actual graph state (template parameters)
    std::ifstream file_;
    FileType file_type_;
    GraphType graph_type_;
    CLIVertexType vertex_type_;
    CLIEdgeType edge_type_;
    vertex_ID_t num_vertices_;
    edge_ID_t num_edges_;
    edge_ID_t num_symmetrized_edges_;






};

#endif // LOADER_BASE_H_