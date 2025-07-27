#ifndef TRIANGLE_COUNTING_THREADED_H
#define TRIANGLE_COUNTING_THREADED_H

#include "../src/cli_dispatch.h"
#include <thread>
#include <atomic>
#include <vector>
#include <algorithm>

// program body for generalized VertexType, EdgeType, and GraphType
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
edge_ID_t triangle_counting_threaded(Graph<Vertex_t, Edge_t, Graph_t> &graph, int num_threads=0) {

    // Assumes undirected graph
    if constexpr (Graph_t == GraphType::DIRECTED)
        std::cout << "warning: Triangle counting not intended for directed graph" << std::endl;

    std::atomic<long long> triangle_count{0};

    // Get number of hardware threads
    if (num_threads == 0) {
        num_threads = std::thread::hardware_concurrency();
    }
    if (num_threads == 0) num_threads = 4; // Fallback if hardware_concurrency fails
    
    std::cout << "Triangle counting using " << num_threads << " threads" << std::endl;

    // This algorithm counts each triangle {u, v, w} exactly once by enforcing
    // an ordering on the vertices, u < v < w.
    // It iterates through each vertex 'u'. For each neighbor 'v' of 'u' where u < v,
    // it then finds common neighbors 'w' of 'u' and 'v'.
    
    // Lambda function for each thread to process a range of vertices
    auto process_vertex_range = [&](vertex_ID_t start_vertex, vertex_ID_t end_vertex) {
        long long local_count = 0;
        
        for (vertex_ID_t u_id = start_vertex; u_id < end_vertex; ++u_id) {
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
                            local_count++;
                        }
                        ++iter_u;
                        ++iter_v;
                    }
                }
            }
        }
        
        // Add local count to global atomic counter
        triangle_count.fetch_add(local_count, std::memory_order_relaxed);
    };

    // Create and start threads
    std::vector<std::thread> threads;
    vertex_ID_t vertices_per_thread = graph.num_vertices() / num_threads;
    
    for (int i = 0; i < num_threads; ++i) {
        vertex_ID_t start_vertex = static_cast<vertex_ID_t>(i) * vertices_per_thread;
        vertex_ID_t end_vertex = (i == num_threads - 1) ? graph.num_vertices() : static_cast<vertex_ID_t>(i + 1) * vertices_per_thread;
        
        threads.emplace_back(process_vertex_range, start_vertex, end_vertex);
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }

    return triangle_count.load();
}

#endif // TRIANGLE_COUNTING_THREADED_H 