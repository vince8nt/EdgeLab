#ifndef GENERATOR_H_
#define GENERATOR_H_

#include <random>
#include <util.h>
#include <builder.h>

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

    EdgeList<Adjacency_t> Generate() {
        switch (gen_type_) {
            case GenType::ERDOS_RENYI:
                return GenerateErdosRenyi();
            case GenType::WATTS_STROGATZ:
                break;
            case GenType::BARABASI_ALBERT:
                break;
        }
        cout << "Defaulting to Erdos-Renyi" << endl;
        return GenerateErdosRenyi();
    }

private:
    EdgeList<Adjacency_t> GenerateErdosRenyi() {
        // generate random edges
        std::mt19937 gen;
        gen.seed(seed_);
        std::uniform_int_distribution<vertex_ID_t> dis(0, num_vertices_ - 1);

        EdgeList<Adjacency_t> edge_list;
        edge_list.reserve(num_edges_);
        for (edge_ID_t i = 0; i < num_edges_; i++) {
            // TODO: add weights
            edge_list.push_back({dis(gen), dis(gen)});
        }
        return edge_list;
    }

    Graph GenerateWattsStrogatz();

    Graph GenerateBarabasiAlbert();

    const uint32_t seed_ = 111119;
    const GenType gen_type_;
    const int scale_;
    const int degree_;
    const vertex_ID_t num_vertices_;
    const edge_ID_t num_edges_;
}


#endif // GENERATOR_H_
