#ifndef TRIANGLE_COUNTING_H
#define TRIANGLE_COUNTING_H

#include "../src/cli_dispatch.h"

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
edge_ID_t triangle_counting(Graph<Vertex_t, Edge_t, Graph_t> &graph) {

    // Assumes undirected graph
    if constexpr (Graph_t == GraphType::DIRECTED)
        std::cout << "warning: Triangle counting not intended for directed graph" << std::endl;

    long long triangle_count = 0;

    // This algorithm counts each triangle {u, v, w} exactly once by enforcing
    // an ordering on the vertices, u < v < w.
    // It iterates through each vertex 'u'. For each neighbor 'v' of 'u' where u < v,
    // it then finds common neighbors 'w' of 'u' and 'v'.
    for (vertex_ID_t u_id = 0; u_id < graph.num_vertices(); ++u_id) {
        // Get vertex u.
        auto u = graph[u_id];

        // Iterate through each neighbor v of u.
        for (const auto& edge_uv : u) {
            vertex_ID_t v_id = edge_uv.dest();

            // Enforce the first part of the ordering: u < v.
            // This ensures we only consider each edge once in one direction.
            if (u_id >= v_id) {
                continue;
            }

            // Get vertex v.
            auto v = graph[v_id];

            // Find the intersection of the neighbor lists of u and v.
            // Since both lists are sorted by destination ID, we can do this in linear time.
            auto iter_u = u.begin();
            auto iter_v = v.begin();

            while (iter_u != u.end() && iter_v != v.end()) {
                if (iter_u->dest() < iter_v->dest()) {
                    ++iter_u;
                } else if (iter_v->dest() < iter_u->dest()) {
                    ++iter_v;
                } else {
                    // We found a common neighbor 'w', which forms a triangle (u, v, w).
                    // Check if w > v to ensure u < v < w ordering.
                    // This ensures each triangle is counted exactly once.
                    if (iter_u->dest() > v_id) {
                        triangle_count++;
                    }
                    ++iter_u;
                    ++iter_v;
                }
            }
        }
    }

    return triangle_count;
}

#endif // TRIANGLE_COUNTING_H 