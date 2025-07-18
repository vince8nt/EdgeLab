
#include "../src/generator.h"
#include "../src/debug.h"

template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
class Tester {
public:
    void run_test(GenType gen_type, int scale, int degree) {
        // generate edge list
        Generator<Vertex_t, Edge_t, Graph_t> generator(gen_type, scale, degree);
        SparseRowGraph<Vertex_t, Edge_t> srg = generator.Generate();
        D_.print(srg);
/*
        Builder<Adjacency_t, VertexStats_t> builder(graph_type);
        Graph<Adjacency_t, VertexStats_t> graph = builder.BuildCSR(edge_list);
        D_.print_graph(graph);
        D_.print_graph_it(graph);*/
    }

private:
    Debug<Vertex_t, Edge_t> D_;
};

int main() {

    Tester<VertexW, EdgeUW, GraphType::UNDIRECTED> tester;
    std::cout << "got here" << std::endl;
    tester.run_test(GenType::ERDOS_RENYI, 4, 2);
}







