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

#include "loader/loader_factory.h"
#include "loader/edge_list_loader.h"
#include "loader/metis_graph_loader.h"
#include "loader/compacted_graph_loader.h"
#include "generator.h"

// GraphMaker is a class that makes graphs from files or generators
// It CLI options to specify the file / generator used
// It uses algorithm requirements to potentially promote graph type, vertex type, and edge type
class GraphMaker {
public:
    GraphMaker(CLIOptions& opts, AlgorithmReqs& reqs) : opts_(opts), reqs_(reqs) {
        // set opts_.graph_type, opts_.vertex_type, opts_.edge_type based on file header
        if (!opts_.load_file_path.empty()) {
            loader_ = create_loader(opts_.load_file_path);
            loader_->load_graph_header(opts_);
        }

        // promote opts_.graph_type, opts_.vertex_type, opts_.edge_type based on reqs_
        // TODO: add support for promoting vertex type and edge type
        if (opts_.graph_type == GraphType::UNDIRECTED) {
            // can't be promoted, do nothing
        }
        // otherwise, opts_.graph_type==DIRECTED (and can be promoted)
        else if (reqs_.graph_type == GraphType::UNDIRECTED) {
            opts_.graph_type = GraphType::UNDIRECTED;
        }
        else if (reqs_.graph_type == GraphType::BIDIRECTED) {
            opts_.graph_type = GraphType::BIDIRECTED;
        }
    };


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
    AlgorithmReqs &reqs_;
    std::unique_ptr<LoaderBase> loader_;
};

#endif // GRAPH_MAKER_H_

