#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <random>
#include "util.h"
#include "graph_comp.h"
#include "builder.h"


template<VertexType Vertex_t, EdgeType Edge_t, GraphType Graph_t>
class Generator {
public:
    Generator(GenType gen_type, int scale, int degree) :
            gen_type_(gen_type), scale_(scale), degree_(degree),
            num_vertices_(static_cast<vertex_ID_t>(1) << scale),
            num_edges_(static_cast<edge_ID_t>(num_vertices_) * degree) {
    }

    // Generate a VectorGraph with the given parameters
    // For undirected graphs, only edges where dest > src are generated
    // For both directed and bidirected graphs, any edge (including self-loops) can be generated
    // Directed and bidirected graphs are treated exactly the same for generation purposes
    VectorGraph<Vertex_t, Edge_t> Generate() {
        std::cout << "Generating " << Graph_t << " " << gen_type_ << " graph: "
            << scale_ << " degree: " << degree_ << std::endl;
        auto timer = timer_start();

        VectorGraph<Vertex_t, Edge_t> vg;
        switch (gen_type_) {
            case GenType::ERDOS_RENYI:
                vg = GenerateErdosRenyi();
                break;
            case GenType::WATTS_STROGATZ:
                vg = GenerateWattsStrogatz();
                break;
            case GenType::BARABASI_ALBERT:
                vg = GenerateBarabasiAlbert();
                break;
            default:
                std::cout << "Defaulting to Erdos-Renyi" << std::endl;
                vg = GenerateErdosRenyi();
        }

        auto time = timer_stop(timer);
        std::cout << "  - Vector Graph generation time: " << time << " seconds" << std::endl;
        return vg;
    }

private:
    // generate random (weighted) vertices
    void GenerateVertexWeights(VectorGraph<Vertex_t, Edge_t> &vg,
            std::mt19937 &gen, std::uniform_int_distribution<weight_t> &weight_dist) {
        if constexpr (WeightedVertexType<Vertex_t>) {
            vg.vertices.reserve(num_vertices_);
            for (vertex_ID_t v = 0; v < num_vertices_; v++) {
                vg.vertices.push_back(weight_dist(gen));
            }
        }
    }

    VectorGraph<Vertex_t, Edge_t> GenerateErdosRenyi() {
        std::mt19937 gen;
        gen.seed(seed_);
        std::uniform_int_distribution<vertex_ID_t> vertex_dist(0, num_vertices_ - 1);
        std::uniform_int_distribution<weight_t> weight_dist(1, 256);
        VectorGraph<Vertex_t, Edge_t> vg(num_vertices_);

        // generate random edges
        auto &matrix = vg.matrix;
        for (edge_ID_t i = 0; i < num_edges_; i++) {
            vertex_ID_t src = vertex_dist(gen);
            vertex_ID_t dest = vertex_dist(gen);
            if constexpr (Graph_t == GraphType::UNDIRECTED) {
                if (src == dest) continue; // disable self loops for undirected graphs
            }
            if constexpr (WeightedEdgeType<Edge_t>) {
                weight_t weight = weight_dist(gen);
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (src <= dest) {
                        if constexpr (DataEdgeType<Edge_t>) {
                            matrix[src].push_back(Edge_t(dest, weight, typename Edge_t::data_type{}));
                        } else {
                            matrix[src].push_back({dest, weight});
                        }
                    } else {
                        if constexpr (DataEdgeType<Edge_t>) {
                            matrix[dest].push_back(Edge_t(src, weight, typename Edge_t::data_type{}));
                        } else {
                            matrix[dest].push_back({src, weight});
                        }
                    }
                } else {
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[src].push_back(Edge_t(dest, weight, typename Edge_t::data_type{}));
                    } else {
                        matrix[src].push_back({dest, weight});
                    }
                }
            } else {
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (src <= dest) {
                        if constexpr (DataEdgeType<Edge_t>) {
                            matrix[src].push_back(Edge_t(dest, typename Edge_t::data_type{}));
                        } else {
                            matrix[src].push_back({dest});
                        }
                    } else {
                        if constexpr (DataEdgeType<Edge_t>) {
                            matrix[dest].push_back(Edge_t(src, typename Edge_t::data_type{}));
                        } else {
                            matrix[dest].push_back({src});
                        }
                    }
                } else {
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[src].push_back(Edge_t(dest, typename Edge_t::data_type{}));
                    } else {
                        matrix[src].push_back({dest});
                    }
                }
            }
        }

        GenerateVertexWeights(vg, gen, weight_dist);
        return vg;
    }

    VectorGraph<Vertex_t, Edge_t> GenerateWattsStrogatz() {
        std::mt19937 gen;
        gen.seed(seed_);
        std::uniform_real_distribution<float> prob_dist(0.0f, 1.0f);
        std::uniform_int_distribution<vertex_ID_t> vertex_dist(0, num_vertices_ - 1);
        std::uniform_int_distribution<weight_t> weight_dist(1, 256);
        
        VectorGraph<Vertex_t, Edge_t> vg(num_vertices_);
        auto &matrix = vg.matrix;
        
        // Watts-Strogatz parameters
        const float rewire_probability = 0.1f; // 10% probability of rewiring
        const int k = degree_; // Each vertex connects to k/2 neighbors on each side
        
        // Step 1: Create regular ring lattice
        for (vertex_ID_t v = 0; v < num_vertices_; v++) {
            for (int j = 1; j <= k / 2; j++) {
                vertex_ID_t u = (v + j) % num_vertices_;
                
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    // For undirected graphs, only add edges where v < u to avoid duplicates
                    if (v < u) {
                        if constexpr (WeightedEdgeType<Edge_t>) {
                            weight_t weight = weight_dist(gen);
                            if constexpr (DataEdgeType<Edge_t>) {
                                matrix[v].push_back(Edge_t(u, weight, typename Edge_t::data_type{}));
                            } else {
                                matrix[v].push_back({u, weight});
                            }
                        } else {
                            if constexpr (DataEdgeType<Edge_t>) {
                                matrix[v].push_back(Edge_t(u, typename Edge_t::data_type{}));
                            } else {
                                matrix[v].push_back({u});
                            }
                        }
                    }
                } else {
                    // For directed graphs, add all edges
                    if constexpr (WeightedEdgeType<Edge_t>) {
                        weight_t weight = weight_dist(gen);
                        if constexpr (DataEdgeType<Edge_t>) {
                            matrix[v].push_back(Edge_t(u, weight, typename Edge_t::data_type{}));
                        } else {
                            matrix[v].push_back({u, weight});
                        }
                    } else {
                        if constexpr (DataEdgeType<Edge_t>) {
                            matrix[v].push_back(Edge_t(u, typename Edge_t::data_type{}));
                        } else {
                            matrix[v].push_back({u});
                        }
                    }
                }
            }
        }
        
        // Step 2: Randomly rewire edges with probability rewire_probability
        for (vertex_ID_t v = 0; v < num_vertices_; v++) {
            auto &edges = matrix[v];
            for (auto &edge : edges) {
                if (prob_dist(gen) < rewire_probability) {
                    // Rewire this edge to a random vertex
                    vertex_ID_t new_dest = vertex_dist(gen);
                    
                    // Avoid self-loops and duplicate edges
                    bool valid_rewire = (new_dest != v);
                    
                    if constexpr (Graph_t == GraphType::UNDIRECTED) {
                        // For undirected graphs, ensure we don't create duplicate edges
                        // and maintain the v < u constraint
                        if (v < new_dest) {
                            // Check if edge already exists
                            for (const auto &existing_edge : matrix[v]) {
                                if (existing_edge.dest() == new_dest) {
                                    valid_rewire = false;
                                    break;
                                }
                            }
                        } else {
                            // For v >= new_dest, we need to check if the reverse edge exists
                            for (const auto &existing_edge : matrix[new_dest]) {
                                if (existing_edge.dest() == v) {
                                    valid_rewire = false;
                                    break;
                                }
                            }
                        }
                    } else {
                        // For directed graphs, just check for self-loops and duplicates
                        for (const auto &existing_edge : matrix[v]) {
                            if (existing_edge.dest() == new_dest) {
                                valid_rewire = false;
                                break;
                            }
                        }
                    }
                    
                    if (valid_rewire) {
                        if constexpr (Graph_t == GraphType::UNDIRECTED) {
                            // For undirected graphs, only rewire if the new destination is greater than source
                            if (v < new_dest) {
                                if constexpr (WeightedEdgeType<Edge_t>) {
                                    weight_t weight = weight_dist(gen);
                                    if constexpr (DataEdgeType<Edge_t>) {
                                        edge = Edge_t(new_dest, weight, typename Edge_t::data_type{});
                                    } else {
                                        edge = Edge_t(new_dest, weight);
                                    }
                                } else {
                                    if constexpr (DataEdgeType<Edge_t>) {
                                        edge = Edge_t(new_dest, typename Edge_t::data_type{});
                                    } else {
                                        edge = Edge_t(new_dest);
                                    }
                                }
                            }
                            // If v >= new_dest, skip the rewiring to maintain the dest > src constraint
                        } else {
                            // For directed graphs, just update the edge
                            if constexpr (WeightedEdgeType<Edge_t>) {
                                weight_t weight = weight_dist(gen);
                                if constexpr (DataEdgeType<Edge_t>) {
                                    edge = Edge_t(new_dest, weight, typename Edge_t::data_type{});
                                } else {
                                    edge = Edge_t(new_dest, weight);
                                }
                            } else {
                                if constexpr (DataEdgeType<Edge_t>) {
                                    edge = Edge_t(new_dest, typename Edge_t::data_type{});
                                } else {
                                    edge = Edge_t(new_dest);
                                }
                            }
                        }
                    }
                }
            }
        }
        
        GenerateVertexWeights(vg, gen, weight_dist);
        return vg;
    }

    VectorGraph<Vertex_t, Edge_t> GenerateBarabasiAlbert() {
        std::mt19937 gen;
        gen.seed(seed_);
        std::uniform_int_distribution<weight_t> weight_dist(1, 256);
        
        VectorGraph<Vertex_t, Edge_t> vg(num_vertices_);
        auto &matrix = vg.matrix;
        
        // Barabási-Albert parameters
        const vertex_ID_t m0 = std::min(static_cast<vertex_ID_t>(degree_), num_vertices_); // Initial clique size
        const int m = std::max(1, degree_ / 2); // Number of edges to add per new vertex
        
        // Step 1: Start with a complete graph of m0 vertices
        for (vertex_ID_t v = 0; v < m0; v++) {
            for (vertex_ID_t u = v + 1; u < m0; u++) {
                if constexpr (WeightedEdgeType<Edge_t>) {
                    weight_t weight = weight_dist(gen);
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[v].push_back(Edge_t(u, weight, typename Edge_t::data_type{}));
                    } else {
                        matrix[v].push_back({u, weight});
                    }
                } else {
                    if constexpr (DataEdgeType<Edge_t>) {
                        matrix[v].push_back(Edge_t(u, typename Edge_t::data_type{}));
                    } else {
                        matrix[v].push_back({u});
                    }
                }
            }
        }
        
        // Step 2: Add remaining vertices one by one with preferential attachment
        for (vertex_ID_t new_vertex = m0; new_vertex < num_vertices_; new_vertex++) {
            // Calculate degree distribution for preferential attachment
            std::vector<vertex_ID_t> degree_sequence;
            for (vertex_ID_t v = 0; v < new_vertex; v++) {
                int degree = static_cast<int>(matrix[v].size());
                // Add vertex v to the sequence 'degree' times (proportional to its degree)
                for (int i = 0; i < degree; i++) {
                    degree_sequence.push_back(v);
                }
            }
            
            // Add m edges from new_vertex to existing vertices
            std::vector<vertex_ID_t> connected_vertices;
            for (int edge_count = 0; edge_count < m && !degree_sequence.empty(); edge_count++) {
                // Select a vertex with probability proportional to its degree
                std::uniform_int_distribution<size_t> seq_dist(0, degree_sequence.size() - 1);
                size_t selected_index = seq_dist(gen);
                vertex_ID_t selected_vertex = degree_sequence[selected_index];
                
                // Check if we already connected to this vertex
                bool already_connected = false;
                for (vertex_ID_t connected : connected_vertices) {
                    if (connected == selected_vertex) {
                        already_connected = true;
                        break;
                    }
                }
                
                if (!already_connected) {
                    connected_vertices.push_back(selected_vertex);
                    
                    if constexpr (Graph_t == GraphType::UNDIRECTED) {
                        // For undirected graphs, only add edges where new_vertex < selected_vertex
                        // to satisfy the builder's constraint that dest > src
                        if (new_vertex < selected_vertex) {
                            if constexpr (WeightedEdgeType<Edge_t>) {
                                weight_t weight = weight_dist(gen);
                                if constexpr (DataEdgeType<Edge_t>) {
                                    matrix[new_vertex].push_back(Edge_t(selected_vertex, weight, typename Edge_t::data_type{}));
                                } else {
                                    matrix[new_vertex].push_back({selected_vertex, weight});
                                }
                            } else {
                                if constexpr (DataEdgeType<Edge_t>) {
                                    matrix[new_vertex].push_back(Edge_t(selected_vertex, typename Edge_t::data_type{}));
                                } else {
                                    matrix[new_vertex].push_back({selected_vertex});
                                }
                            }
                        } else {
                            // If new_vertex >= selected_vertex, add the edge to the selected_vertex instead
                            if constexpr (WeightedEdgeType<Edge_t>) {
                                weight_t weight = weight_dist(gen);
                                if constexpr (DataEdgeType<Edge_t>) {
                                    matrix[selected_vertex].push_back(Edge_t(new_vertex, weight, typename Edge_t::data_type{}));
                                } else {
                                    matrix[selected_vertex].push_back({new_vertex, weight});
                                }
                            } else {
                                if constexpr (DataEdgeType<Edge_t>) {
                                    matrix[selected_vertex].push_back(Edge_t(new_vertex, typename Edge_t::data_type{}));
                                } else {
                                    matrix[selected_vertex].push_back({new_vertex});
                                }
                            }
                        }
                    } else {
                        // For directed graphs, add edge from new_vertex to selected_vertex
                        if constexpr (WeightedEdgeType<Edge_t>) {
                            weight_t weight = weight_dist(gen);
                            if constexpr (DataEdgeType<Edge_t>) {
                                matrix[new_vertex].push_back(Edge_t(selected_vertex, weight, typename Edge_t::data_type{}));
                            } else {
                                matrix[new_vertex].push_back({selected_vertex, weight});
                            }
                        } else {
                            if constexpr (DataEdgeType<Edge_t>) {
                                matrix[new_vertex].push_back(Edge_t(selected_vertex, typename Edge_t::data_type{}));
                            } else {
                                matrix[new_vertex].push_back({selected_vertex});
                            }
                        }
                    }
                    
                    // Remove the selected vertex from degree_sequence to avoid duplicates
                    degree_sequence.erase(degree_sequence.begin() + selected_index);
                }
            }
        }
        
        GenerateVertexWeights(vg, gen, weight_dist);
        return vg;
    }

    const uint32_t seed_ = 111119;
    const GenType gen_type_;
    const int scale_;
    const int degree_;
    const vertex_ID_t num_vertices_;
    const edge_ID_t num_edges_;
};


#endif // GENERATOR_H_
