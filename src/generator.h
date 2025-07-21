#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <random>
#include "util.h"
#include "graph_comp.h"
#include "builder.h"

enum class GenType {
    ERDOS_RENYI,     // Erdos-Renyi-Gilbert
    WATTS_STROGATZ,  // Watts-Strogatz
    BARABASI_ALBERT, // Barabasi-Albert
};

template<NonDataVertexType Vertex_t, NonDataEdgeType Edge_t, GraphType Graph_t>
class Generator {
public:
    Generator(GenType gen_type, int scale, int degree) :
            gen_type_(gen_type), scale_(scale), degree_(degree),
            num_vertices_(static_cast<vertex_ID_t>(1) << scale),
            num_edges_(static_cast<edge_ID_t>(num_vertices_) * degree) {
    }

    VectorGraph<Vertex_t, Edge_t> Generate() {
        switch (gen_type_) {
            case GenType::ERDOS_RENYI:
                return GenerateErdosRenyi();
            case GenType::WATTS_STROGATZ:
                break;
            case GenType::BARABASI_ALBERT:
                break;
        }
        std::cout << "Defaulting to Erdos-Renyi" << std::endl;
        return GenerateErdosRenyi();
    }

private:
    VectorGraph<Vertex_t, Edge_t> GenerateErdosRenyi() {
        std::mt19937 gen;
        gen.seed(seed_);
        std::uniform_int_distribution<vertex_ID_t> vertex_dist(0, num_vertices_ - 1);
        std::uniform_real_distribution<weight_t> weight_dist(0.0, 2.0);
        VectorGraph<Vertex_t, Edge_t> vg(num_vertices_);

        // generate random edges
        auto &matrix = vg.matrix;
        for (edge_ID_t i = 0; i < num_edges_; i++) {
            vertex_ID_t src = vertex_dist(gen);
            vertex_ID_t dest = vertex_dist(gen);
            // if (src == dest) continue; // disable self loops
            if constexpr (WeightedEdgeType<Edge_t>) {
                weight_t weight = 2 - weight_dist(gen);
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (src <= dest)
                        matrix[src].push_back({dest, weight});
                    else
                        matrix[dest].push_back({src, weight});
                }
                else
                    matrix[src].push_back({dest, weight});
            }
            else {
                if constexpr (Graph_t == GraphType::UNDIRECTED) {
                    if (src <= dest)
                        matrix[src].push_back({dest});
                    else
                        matrix[dest].push_back({src});
                }
                else
                    matrix[src].push_back({dest});
            }
        }

        // generate random (weighted) vertices
        if constexpr (WeightedVertexType<Vertex_t>) {
            auto &vertices = vg.vertices;
            vertices.reserve(num_vertices_);
            for (vertex_ID_t v = 0; v < num_vertices_; v++) {
                vertices.push_back(2 - weight_dist(gen));
            }
        }

        std::cout << "generation finished" << std::endl;
        return vg;
    }

    // Graph GenerateWattsStrogatz();

    // Graph GenerateBarabasiAlbert();

    const uint32_t seed_ = 111119;
    const GenType gen_type_;
    const int scale_;
    const int degree_;
    const vertex_ID_t num_vertices_;
    const edge_ID_t num_edges_;
};


#endif // GENERATOR_H_
