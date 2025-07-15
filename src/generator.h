#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <graph.h>
#include <util.h>

enum class GenType {
    ERDOS_RENYI,     // Erdos-Renyi-Gilbert
    WATTS_STROGATZ,  // Watts-Strogatz
    BARABASI_ALBERT, // Barabasi-Albert
}

template<typename Adjacency_t>
class Generator {
public:
    Generator(GenType gen_type, int scale, int degree) :
            gen_type_(gen_type), scale_(scale), degree_(degree),
            num_vertices_(static_cast<vertex_ID_t>(1) << scale),
            num_edges_(static_cast<edge_ID_t>(num_vertices_) * degree) {
    }

    Graph<Adjacency_t> Generate() {
        switch (gen_type_) {
            case GenType::ERDOS_RENYI:
                return GenerateErdosRenyi();
            case GenType::WATTS_STROGATZ:
                break;
            case GenType::BARABASI_ALBERT:
                break;
        }
    }

private:
    Graph<Adjacency_t> GenerateErdosRenyi() {
        Adjacency_t* edges = (Adjacency_t*)malloc(num_vertices_ * num_edges_ * sizeof(Adjacency_t));
        return Graph<Adjacency_t>(edges);
    }

    Graph GenerateWattsStrogatz();

    Graph GenerateBarabasiAlbert();

    GenType gen_type_;
    int scale_;
    int degree_;
    vertex_ID_t num_vertices_;
    edge_ID_t num_edges_;
}


