
#include "../src/generator.h"
#include "../src/debug.h"
#include "../src/cli.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Tester {
public:
    void run_test(GenType gen_type, int scale, int degree) {
        // generate edge list
        Generator<Vertex_t, Edge_t, Graph_t> generator(gen_type, scale, degree);
        VectorGraph<Vertex_t, Edge_t> vg = generator.Generate();
        D_.print(vg);

        Builder<Vertex_t, Edge_t, Graph_t> builder;
        Graph<Vertex_t, Edge_t, Graph_t> graph = builder.BuildGraph(vg);
        D_.print(graph);
        D_.print_it(graph);
    }

private:
    Debug<Vertex_t, Edge_t, Graph_t> D_;
};

int main() {

    Tester<VertexUW, EdgeW, GraphType::UNDIRECTED> tester;
    tester.run_test(GenType::ERDOS_RENYI, 4, 2);
}







