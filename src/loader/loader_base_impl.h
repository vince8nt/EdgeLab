#ifndef LOADER_BASE_IMPL_H_
#define LOADER_BASE_IMPL_H_

#include "edge_list_loader.h"
#include "metis_graph_loader.h"
#include "compacted_graph_loader.h"

template<VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
Graph<Vertex_t, Edge_t, Graph_t> LoaderBase::load_graph_body() {
    switch (file_type_) {
        case FileType::EL:
        case FileType::WEL:
        case FileType::VEL:
        case FileType::VWEL:
            return dynamic_cast<EdgeListLoader*>(this)->load_graph_body<Vertex_t, Edge_t, Graph_t>();
        case FileType::GRAPH:
            return dynamic_cast<MetisGraphLoader*>(this)->load_graph_body<Vertex_t, Edge_t, Graph_t>();
        case FileType::CG:
            return dynamic_cast<CompactedGraphLoader*>(this)->load_graph_body<Vertex_t, Edge_t, Graph_t>();
        default:
            throw std::runtime_error("Invalid file type");
    }
}

#endif // LOADER_BASE_IMPL_H_ 