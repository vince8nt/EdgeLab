
#include "../src/generator.h"
#include "../src/debug.h"

template<typename Adjacency_t, typename VertexStats_t>
class Tester {
    Debug<Adjacency_t, VertexStats_t> D_;
public:
    void run_test(GraphType graph_type, GenType gen_type, int scale, int degree) {
        // generate edge list
        Generator<Adjacency_t> generator(gen_type, scale, degree);
        EdgeList<Adjacency_t> edge_list = generator.Generate();
        D_.print_edge_list(edge_list);

        Builder<Adjacency_t, VertexStats_t> builder(graph_type);
        Graph<Adjacency_t, VertexStats_t> graph = builder.BuildCSR(edge_list);
        D_.print_graph(graph);
        D_.print_graph_it(graph);
    }


};

int main() {

    Tester<Adjacency, VertexStats> tester;
    tester.run_test(GraphType::UNDIRECTED, GenType::ERDOS_RENYI, 4, 2);
}







