#ifndef LOADER_BASE_H_
#define LOADER_BASE_H_

#include <fstream>
#include "../graph.h"

// Forward declarations
class EdgeListLoader;
class MetisGraphLoader;
class CompactedGraphLoader;

class LoaderBase {
public:
    LoaderBase(FileType file_type) : file_type_(file_type) {}
    virtual ~LoaderBase() = default;

    // load graph header via CLI options (opts)
    // opens and reads header info from opts.load_file_path
    // sets opts.graph_type, opts.vertex_type, opts.edge_type
    virtual void load_graph_header(CLIOptions& opts) = 0;

    // Template function that dispatches to the correct derived class
    template<VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> load_graph_body();

protected:
    // set by factory based on file extension
    const FileType file_type_;

    // set by specific loader (so file can be opened in binary mode if needed)
    std::ifstream file_;

    // set by file header - may differ from actual graph template parameters
    GraphType graph_type_;
    CLIVertexType vertex_type_;
    CLIEdgeType edge_type_;
};

// Include the implementation in a separate header
#include "loader_base_impl.h"

#endif // LOADER_BASE_H_
