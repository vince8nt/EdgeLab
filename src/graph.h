#ifndef GRAPH_H_
#define GRAPH_H_

#include "util.h"
#include "graph_comp.h"
#include <span>

// Forward declaration for Vertex specialization
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
struct CSR_Vertex;

// Vertex specialization for DIRECTED and UNDIRECTED graphs
template<typename Vertex_t, typename Edge_t>
struct CSR_Vertex<Vertex_t, Edge_t, GraphType::DIRECTED> : public Vertex_t {
    CSR_Vertex(Edge_t* edges_begin) requires EmptyVertexType<Vertex_t> : edges_begin_(edges_begin) {}
    CSR_Vertex(const Vertex_t& vertex, Edge_t* edges_begin) requires NonEmptyVertexType<Vertex_t> :
            Vertex_t(vertex), edges_begin_(edges_begin) {}
    Edge_t* edges_begin_;
};

template<typename Vertex_t, typename Edge_t>
struct CSR_Vertex<Vertex_t, Edge_t, GraphType::UNDIRECTED> : public Vertex_t {
    CSR_Vertex(Edge_t* edges_begin) requires EmptyVertexType<Vertex_t> : edges_begin_(edges_begin) {}
    CSR_Vertex(const Vertex_t& vertex, Edge_t* edges_begin) requires NonEmptyVertexType<Vertex_t> :
            Vertex_t(vertex), edges_begin_(edges_begin) {}
    Edge_t* edges_begin_;
};

// Vertex specialization for BIDIRECTED graphs
template<typename Vertex_t, typename Edge_t>
struct CSR_Vertex<Vertex_t, Edge_t, GraphType::BIDIRECTED> : public Vertex_t {
    CSR_Vertex(Edge_t* edges_begin, Edge_t* edges_in_begin) requires EmptyVertexType<Vertex_t> : 
            edges_begin_(edges_begin), edges_in_begin_(edges_in_begin) {}
    CSR_Vertex(const Vertex_t& vertex, Edge_t* edges_begin, Edge_t* edges_in_begin) requires NonEmptyVertexType<Vertex_t> :
            Vertex_t(vertex), edges_begin_(edges_begin), edges_in_begin_(edges_in_begin) {}
    Edge_t* edges_begin_;
    Edge_t* edges_in_begin_;
};

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Graph {
public:
    using Vertex = CSR_Vertex<Vertex_t, Edge_t, Graph_t>;

    // Vertex wrapper exposed to user (A single Vertex Pointer)
    struct VertexRef {
        // TODO(vince): find a more elagant way to do this
        Vertex* csr_loc_; // only for use within Graph class

        // VertexRef(Vertex* csr_loc) : csr_loc_(csr_loc) {}
        VertexRef(const Vertex* csr_loc) : csr_loc_(const_cast<Vertex*>(csr_loc)) {}

        // Vertex iteration support
        // bool operator==(Vertex* csr_loc) const { return csr_loc == csr_loc_; } // kinda jank
        bool operator==(VertexRef vr) const {return vr.csr_loc_ == csr_loc_; }          // kinda jank
        VertexRef operator+(std::ptrdiff_t n) const { return {csr_loc_ + n}; }
        VertexRef operator++(int) { VertexRef temp = *this; csr_loc_++; return temp; } // Postfix Increment
        VertexRef& operator++() { csr_loc_++; return *this; } // Prefix Increment
        VertexRef& operator+=(std::ptrdiff_t n) { csr_loc_ += n; return *this; }
        VertexRef operator-(std::ptrdiff_t n) const { return {csr_loc_ - n}; }
        VertexRef operator--(int) { VertexRef temp = *this; csr_loc_--; return temp; } // Postfix Decrement
        VertexRef& operator--() { csr_loc_--; return *this; } // Prefix Decrement
        VertexRef& operator-=(std::ptrdiff_t n) { csr_loc_ -= n; return *this; }

        // get Vertex ID (kinda annoying that we need to pass in graph)
        vertex_ID_t ID(Graph &graph) const { return csr_loc_ - graph.begin().csr_loc_; }
        // vertex_ID_t ID() const { return csr_loc_ - vertices_; } // doesn't have access to nonstatic Graph vars

        // Edge iteration support
        Edge_t* begin() const { return csr_loc_->edges_begin_; }
        Edge_t* end() const { return (csr_loc_ + 1)->edges_begin_; }

        // Edge indexing support
        vertex_ID_t degree() const { return end() - begin(); }
        Edge_t operator[](vertex_ID_t i) const { return begin()[i]; }

        // Edge lookup support
        bool has_edge_to(vertex_ID_t target_id) const {
            return std::binary_search(begin(), end(), target_id,
            [](const Edge_t& edge, vertex_ID_t target) {
                return edge.dest() < target;
            });
        }
        Edge_t* get_edge_to(vertex_ID_t target_id) const {
            auto it = std::lower_bound(begin(), end(), target_id,
                [](const Edge_t& edge, vertex_ID_t target) {
                    return edge.dest() < target;
                });
            if (it != end() and it->dest() == target_id)
                return it;
            return end();
        }

        // Incoming Edge iteration support (only for BIDIRECTED and UNDIRECTED graphs)
        std::span<Edge_t> in() const requires (Graph_t == GraphType::BIDIRECTED) {
            return {csr_loc_->edges_in_begin_, (csr_loc_ + 1)->edges_in_begin_};
        }
        std::span<Edge_t> in() const requires (Graph_t == GraphType::UNDIRECTED) {
            return {csr_loc_->edges_begin_, (csr_loc_ + 1)->edges_begin_};
        }

        // Incoming Edge indexing support (only for BIDIRECTED and UNDIRECTED graphs)
        vertex_ID_t in_degree() const
                requires (Graph_t == GraphType::BIDIRECTED or Graph_t == GraphType::UNDIRECTED) {
            return in().size();
        }
        Edge_t in_edge(vertex_ID_t i) const
                requires (Graph_t == GraphType::BIDIRECTED or Graph_t == GraphType::UNDIRECTED) {
            return in()[i];
        }

        // Incoming Edge lookup support (only for BIDIRECTED and UNDIRECTED graphs)
        bool has_edge_from(vertex_ID_t source_id) const
                requires (Graph_t == GraphType::BIDIRECTED or Graph_t == GraphType::UNDIRECTED) {
            return std::binary_search(in().begin(), in().end(), source_id,
            [](const Edge_t& edge, vertex_ID_t source) {
                return edge.dest() < source;
            });
        }
        Edge_t* get_edge_from(vertex_ID_t source_id) const
                requires (Graph_t == GraphType::BIDIRECTED or Graph_t == GraphType::UNDIRECTED) {
            auto it = std::lower_bound(in().begin(), in().end(), source_id,
                [](const Edge_t& edge, vertex_ID_t source) {
                    return edge.dest() < source;
                });
            if (it != in().end() and it->dest() == source_id)
                return it;
            return in().end();
        }

        // Vertex functionality support
        weight_t weight() const { return csr_loc_->weight(); }
        Vertex::data_type data() const requires DataVertexType<Vertex>
            { return csr_loc_->data(); }
    };


    // constructors / destructor
    Graph(vertex_ID_t num_vertices, Vertex* vertices, edge_ID_t num_edges, Edge_t* edges)
            requires (Graph_t != GraphType::BIDIRECTED) :
            num_vertices_(num_vertices), vertices_(vertices),
            num_edges_(num_edges), edges_(edges), edges_in_(nullptr) {
    }
    Graph(vertex_ID_t num_vertices, Vertex* vertices, edge_ID_t num_edges, Edge_t* edges, Edge_t* edges_in)
            requires (Graph_t == GraphType::BIDIRECTED) :
            num_vertices_(num_vertices), vertices_(vertices),
            num_edges_(num_edges), edges_(edges), edges_in_(edges_in) {
    }
    Graph(const Graph&) = delete;
    Graph(Graph&& other) noexcept :
        num_vertices_(other.num_vertices_),
        vertices_(other.vertices_),
        num_edges_(other.num_edges_),
        edges_(other.edges_),
        edges_in_(other.edges_in_) {
    }
    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) noexcept = delete;
    ~Graph() {
        if constexpr (Graph_t == GraphType::BIDIRECTED)
            free(const_cast<Edge_t*>(edges_in_));
        free(const_cast<Edge_t*>(edges_));
        free(const_cast<Vertex*>(vertices_));
    }


    // getters
    edge_ID_t num_edges() const {
        // Always return the total number of directed edges in the CSR
        return num_edges_;
    }

    // Get the number of unique undirected edges (only available for undirected graphs)
    edge_ID_t num_undirected_edges() const requires (Graph_t == GraphType::UNDIRECTED) {
        return num_edges_ / 2;
    }

    // indexing suport for vertices
    vertex_ID_t num_vertices() const { return num_vertices_; }
    const VertexRef operator[](vertex_ID_t i) const { return VertexRef(vertices_ + i); }

    // Iterator support for vertices
    const VertexRef begin() const { return VertexRef(vertices_); }
    const VertexRef end() const { return VertexRef(vertices_ + num_vertices_); }

    // get Vertex ID (kinda annoying that it needs to be called from graph)
    vertex_ID_t ID(const VertexRef vr) { return vr.csr_loc_ - vertices_; }

    // printing functionality (for testing/debugging)
    friend std::ostream& operator<<(std::ostream& os, const Graph<Vertex_t, Edge_t, Graph_t>& g) {
        for (vertex_ID_t i = 0; i < g.num_vertices(); i++) {
            if constexpr (WeightedVertexType<Vertex_t>)
                os << "["<< i << " " << std::setprecision(3) << g[i].weight() << "]: ";
            else
                os << i << ": ";
            for (auto &e : g[i]) {
                if constexpr (WeightedEdgeType<Edge_t>)
                    os << "[" << e.dest() << " " << std::setprecision(3) << e.weight() << "] ";
                else
                    os << e.dest() << " ";
            }
            os << std::endl;
        }
        return os;
    }

private:
    const vertex_ID_t num_vertices_;
    const Vertex* vertices_;
    const edge_ID_t num_edges_;
    const Edge_t* const edges_;
    const Edge_t* const edges_in_;
};

#endif // GRAPH_H_
