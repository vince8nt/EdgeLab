#ifndef COMPACTED_GRAPH_LOADER_H_
#define COMPACTED_GRAPH_LOADER_H_

// include c++20 endian headers
#include <bit>
#include "loader_base.h"

class CompactedGraphLoader : public LoaderBase {
public:
    CompactedGraphLoader(FileType file_type) : LoaderBase(file_type) {}

    void load_graph_header(CLIOptions& opts) {
        if (file_.is_open()) {
            std::cerr << "File already open" << std::endl;
            exit(1);
        }
        file_ = std::ifstream(opts.load_file_path, std::ios::binary);
        if (!file_.is_open()) {
            std::cerr << "Failed to open file: " << opts.load_file_path << std::endl;
            exit(1);
        }

        bool undirected, v_weights, e_weights, little_endian;
        file_.read(reinterpret_cast<char*>(&undirected), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&v_weights), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&e_weights), sizeof(bool));
        file_.read(reinterpret_cast<char*>(&little_endian), sizeof(bool));
        // check endianness for serialized data (greater than 1 byte in length)
        if ((little_endian and std::endian::native != std::endian::little) or
            (!little_endian and std::endian::native != std::endian::big)) {
            std::cerr << "System endianness does not match file endianness" << std::endl;
            std::cerr << "  Cannot load file" << std::endl;
            std::cerr << "  Use locally saved CG files to avoid this issue" << std::endl;
            exit(1);
            // TODO: add loading of data in reverse endianness
        }
        graph_type_ = undirected ? GraphType::UNDIRECTED : GraphType::DIRECTED;
        vertex_type_ = v_weights ? CLIVertexType::WEIGHTED : CLIVertexType::UNWEIGHTED;
        edge_type_ = e_weights ? CLIEdgeType::WEIGHTED : CLIEdgeType::UNWEIGHTED;
        file_.read(reinterpret_cast<char*>(&num_vertices_), sizeof(vertex_ID_t));
        file_.read(reinterpret_cast<char*>(&num_edges_), sizeof(edge_ID_t));
        if (!undirected)
            file_.read(reinterpret_cast<char*>(&num_symmetrized_edges_), sizeof(edge_ID_t));

        opts.graph_type = graph_type_;
        opts.vertex_type = vertex_type_;
        opts.edge_type = edge_type_;
    }

    template <VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
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
                    // allocate temp memory for directed edges
                    // allocate permanent memory for symmetrized edges
                    // read in vertices, and set edges_start (pointing to temp memory)
                    // allocate inverse edges_start vector (but treat as inverse degree vector for now)
                    // read in directed edges into temp memory (and compute inverse degrees)
                        // inverse degrees should exclude self loops
                    // sweep inverse degrees to translate to inverse edges_start in place
                        // this should point to the top half of the CSR
                    // go through vertices (edges + inverse edges) and merge into bottom half of CSR
                        // remove duplicates and update edges_start for each
    Graph<Vertex_t, Edge_t, Graph_t> load_graph_body() {
        if (!file_.is_open()) {
            std::cerr << "File not open" << std::endl;
            std::cerr << "Must call load_graph_header before load_graph_body" << std::endl;
            exit(1);
        }
        using Vertex = CSR_Vertex<Vertex_t, Edge_t, Graph_t>;
        
        // permenant graph memory allocation
        size_t vertices_size = (num_vertices_ + 1) * sizeof(Vertex);
        size_t edges_size = num_edges_ * sizeof(Edge_t);
        Vertex* vertices = (Vertex*)malloc(vertices_size);
        Edge_t* edges = (Edge_t*)malloc(edges_size);

        load_CG_vertices(vertices, vertices_size, edges);
        
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

private:
    // extra data specific to CG format set by load_graph_header
    vertex_ID_t num_vertices_;
    edge_ID_t num_edges_;
    edge_ID_t num_symmetrized_edges_;

    // load CG vertices (same for all vertex/edge/graph types)
    template<VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
    void load_CG_vertices(CSR_Vertex<Vertex_t, Edge_t, Graph_t>* vertices, size_t vertices_size,
            Edge_t* edges) {
        using Vertex = CSR_Vertex<Vertex_t, Edge_t, Graph_t>;
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
                if constexpr (DataVertexType<Vertex_t>) {
                    new (&vertices[i]) Vertex(Vertex_t(weight, typename Vertex_t::data_type{}), edges_offset);
                } else {
                    new (&vertices[i]) Vertex(weight, edges_offset);
                }
            }
            else {
                degree = *reinterpret_cast<vertex_ID_t*>(v_read_loc);
                v_read_loc += sizeof(vertex_ID_t);
                if constexpr (DataVertexType<Vertex_t>) {
                    new (&vertices[i]) Vertex(Vertex_t(typename Vertex_t::data_type{}), edges_offset);
                } else {
                    new (&vertices[i]) Vertex(edges_offset);
                }
            }
            edges_offset += degree;
        }
        // add ending vertex
        if constexpr (WeightedVertexType<Vertex_t>) {
            if constexpr (DataVertexType<Vertex_t>) {
                new (&vertices[num_vertices_]) Vertex(Vertex_t(0, typename Vertex_t::data_type{}), edges_offset);
            } else {
                new (&vertices[num_vertices_]) Vertex(0, edges_offset);
            }
        } else {
            if constexpr (DataVertexType<Vertex_t>) {
                new (&vertices[num_vertices_]) Vertex(Vertex_t(typename Vertex_t::data_type{}), edges_offset);
            } else {
                new (&vertices[num_vertices_]) Vertex(edges_offset);
            }
        }
    }

};


#endif // COMPACTED_GRAPH_LOADER_H_