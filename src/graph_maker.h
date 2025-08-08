#ifndef GRAPH_MAKER_H_
#define GRAPH_MAKER_H_

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

#include "loader/loader_factory_impl.h"
#include "generator.h"

// GraphMaker is a class that makes graphs from files or generators
// It uses CLI options to specify the file / generator used
// Type promotion is handled by the dispatch system, not here
class GraphMaker {
public:
    GraphMaker(CLIOptions& opts) : opts_(opts) {
        // Set opts_.graph_type, opts_.vertex_type, opts_.edge_type based on file header
        if (!opts_.load_file_path.empty()) {
            loader_ = create_loader(opts_.load_file_path);
            loader_->load_graph_header(opts_);
        }
    }

    template<VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
    Graph<Vertex_t, Edge_t, Graph_t> make_graph() {
        if (loader_) {
            return loader_->load_graph_body<Vertex_t, Edge_t, Graph_t>();
        }
        else {
            Generator<Vertex_t, Edge_t, Graph_t> gen(opts_.gen_type, opts_.scale, opts_.degree);
            VectorGraph<Vertex_t, Edge_t> vg = gen.Generate();
            Builder<Vertex_t, Edge_t, Graph_t> builder;
            return builder.BuildGraph(vg);
        }
    }

private:
    CLIOptions &opts_;
    std::unique_ptr<LoaderBase> loader_;
};

#endif // GRAPH_MAKER_H_

