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
        return EdgeList<Adjacency_t>();
    }

    Graph GenerateWattsStrogatz();

    Graph GenerateBarabasiAlbert();

    const GenType gen_type_;
    const int scale_;
    const int degree_;
    const vertex_ID_t num_vertices_;
    edge_ID_t num_edges_; // not const because it's only an estimate before generation
}


#endif // GENERATOR_H_
