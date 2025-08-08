#ifndef BREADTH_FIRST_SEARCH_H_
#define BREADTH_FIRST_SEARCH_H_

#include "../src/graph_core.h"
#include <queue>

// BFS algorithm requirements - compile-time constants
struct BFSAlgorithmReqs {
    static constexpr GraphType graph_type = GraphType::DIRECTED;  // BFS works on directed graphs
    static constexpr CLIVertexType vertex_type = CLIVertexType::UNWEIGHTED;  // BFS doesn't need vertex weights
    static constexpr CLIEdgeType edge_type = CLIEdgeType::UNWEIGHTED;  // BFS doesn't need edge weights
};

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
                    return distances[v_id];
                }
            }
        }
    }

    // If the queue becomes empty and we haven't found the destination,
    // then no path exists.
    return -1;
}

#endif // BREADTH_FIRST_SEARCH_H_ 