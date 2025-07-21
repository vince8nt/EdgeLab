#include "../src/cli_dispatch.h"
#include <queue>

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
long long breadth_first_search(Graph<Vertex_t, Edge_t, Graph_t> &graph, vertex_ID_t src, vertex_ID_t dest) {
    // Validate source and destination vertices
    if (src >= graph.num_vertices() || dest >= graph.num_vertices()) {
        throw std::invalid_argument("Source or destination vertex is out of bounds.");
    }

    // If source and destination are the same, distance is 0
    if (src == dest) {
        return 0;
    }

    // A queue for the vertices to visit in BFS order
    std::queue<vertex_ID_t> q;
    // A vector to store the distance of each vertex from the source.
    // Initialized to -1 to indicate that a vertex has not been visited.
    std::vector<long long> distances(graph.num_vertices(), -1);

    // Start BFS from the source vertex
    q.push(src);
    distances[src] = 0;
    std::cout << "Starting Breadth-First Search from " << src << " to " << dest << "..." << std::endl;

    while (!q.empty()) {
        vertex_ID_t u_id = q.front();
        q.pop();

        // Get the VertexRef for the current vertex u_id.
        // The VertexRef is directly iterable to get its edges.
        for (const auto& edge_uv : graph[u_id]) {
            vertex_ID_t v_id = edge_uv.dest();

            // If the neighbor has not been visited yet
            if (distances[v_id] == -1) {
                // Mark it as visited and set its distance
                distances[v_id] = distances[u_id] + 1;
                q.push(v_id);

                // If we found the destination, return its distance immediately
                if (v_id == dest) {
                    std::cout << "Shortest path found with distance: " << distances[v_id] << std::endl;
                    return distances[v_id];
                }
            }
        }
    }

    // If the queue becomes empty and we haven't found the destination,
    // then no path exists.
    throw std::runtime_error("No path exists between source and destination.");
}


// Functor for dispatching templated function via CLI options
struct Dispatcher {
    int &exit_code;
    template<typename V, typename E, GraphType G>
    void operator()(Graph<V, E, G> &graph) const {
        vertex_ID_t src = 0;
        vertex_ID_t dest = graph.num_vertices() - 1;
        try {
            long long dist = breadth_first_search<V, E, G>(graph, src, dest);
            std::cout << "BFS returned: " << dist << std::endl;
        }
        catch (std::exception &e) {
            std::cerr << "Caught BFS exception: " << e.what() << std::endl;
            exit_code = 1;
        }
    }
};

int main(int argc, char** argv) {
    int exit_code = 0;
    CLIOptions opts = parse_cli(argc, argv);
    dispatch_cli(opts, Dispatcher{exit_code});
    if (exit_code)
        std::cerr << "Failed with exit code: " << exit_code << std::endl;
    else
        std::cout << "Succeeded with exit code: " << exit_code << std::endl;
    return exit_code;
}
