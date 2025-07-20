
#include "../src/generator.h"
#include "../src/cli.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
struct Tester {
    void run_test(GenType gen_type, int scale, int degree) {
        // generate Vector Graph
        Generator<Vertex_t, Edge_t, Graph_t> generator(gen_type, scale, degree);
        VectorGraph<Vertex_t, Edge_t> vg = generator.Generate();
        std::cout << "Generated Vector Graph" << std::endl << vg << std::endl;

        // build CSR Graph
        Builder<Vertex_t, Edge_t, Graph_t> builder;
        Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
        std::cout << "CSR Graph" << std::endl << graph << std::endl;
    }
};

int main() {
    Tester<VertexUW, EdgeW, GraphType::UNDIRECTED> tester;
    tester.run_test(GenType::ERDOS_RENYI, 4, 2);
}







