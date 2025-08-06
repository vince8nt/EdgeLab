#ifndef LOADER_BASE_H_
#define LOADER_BASE_H_

#include <fstream>
#include "../graph.h"


class LoaderBase {
public:
    LoaderBase(FileType file_type) : file_type_(file_type) {}
    virtual ~LoaderBase() = default;

    // load graph header via CLI options (opts)
    // opens and reads header info from opts.load_file_path
    // sets opts.graph_type, opts.vertex_type, opts.edge_type
    virtual void load_graph_header(CLIOptions& opts) = 0;

    // load graph body
    // reads graph body from already opened file
    // closes file and returns graph
    // Note: This is implemented by derived classes as template functions
    // The base class does not provide an implementation since template functions cannot be virtual

    // each call to load_graph_body must be preceded by a distinct call to load_graph_header
    // load_graph_header is not templated, but load_graph_body is
    // the CLI options are updated by load_graph_header
    // the CLI options are thenused (in combinitation with an algorithm's specified graph type)
        // to dispatch templates to load_graph_body (done by cli_dispatch.h)

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

#endif // LOADER_BASE_H_
