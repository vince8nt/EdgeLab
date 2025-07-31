#ifndef GRAPH_H_
#define GRAPH_H_

#include "util.h"
#include "graph_comp.h"



template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Graph {

public:

    // Vertex used in flattened CSR Graph
    // Todo (maybe move this to generator class)
    #pragma pack (push, 4)
    struct Vertex : public Vertex_t {
        // For empty vertex - relies on Empty Base Class Optimization (EBCO)
        Vertex(Edge_t* edges_begin) requires EmptyVertexType<Vertex_t> :
                edges_begin_(edges_begin) {}

        // For non empty vertex
        Vertex(const Vertex_t& vertex, Edge_t* edges_begin)
                requires NonEmptyVertexType<Vertex_t> :
                Vertex_t(vertex), edges_begin_(edges_begin) {}

        // starting location of our outgoing edges in graph's edges_ array
        Edge_t* edges_begin_; // requires EmptyVertexType<Vertex_t>;
        // alignas(vertex_ID_t) const Edge_t* edges_begin_ requires NonEmptyVertexType<Vertex_t>;
        // TODO(vince): ensure dense packing of Vertex attributes
    };
    #pragma pack (pop)


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

        // Vertex functionality support
        weight_t weight() const { return csr_loc_->weight(); }
        Vertex::data_type data() const requires DataVertexType<Vertex>
            { return csr_loc_->data(); }
    };


    // constructors / destructor
    Graph(vertex_ID_t num_vertices, Vertex* vertices, edge_ID_t num_edges, Edge_t* edges) :
            num_vertices_(num_vertices), vertices_(vertices),
            num_edges_(num_edges), edges_(edges) {
    }
    Graph(const Graph&) = delete;
    Graph(Graph&& other) noexcept : // only enable move constructor.
        num_vertices_(other.num_vertices_),
        vertices_(other.vertices_),
        num_edges_(other.num_edges_),
        edges_(other.edges_) {
    }
    Graph& operator=(const Graph&) = delete;
    Graph& operator=(Graph&&) noexcept = delete;
    ~Graph() {
        free(const_cast<Edge_t*>(edges_));
        free(const_cast<Vertex*>(vertices_));
    }


    // getters
    edge_ID_t num_edges() const {
        if constexpr (Graph_t == GraphType::UNDIRECTED)
            return num_edges_ / 2;
        else
            return num_edges_;
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
};



#endif // GRAPH_H_
