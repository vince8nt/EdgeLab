#include "../src/cli_dispatch.h"
#include "../src/thread_safe.h"
#include <queue>
#include <vector>
#include <atomic>
#include <pthread.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <algorithm>

// Thread arguments structure
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
struct ThreadArgs {
    Graph<Vertex_t, Edge_t, Graph_t>* graph;
    ThreadSafeQueue<vertex_ID_t>* current_level;
    ThreadSafeQueue<vertex_ID_t>* next_level;
    ThreadSafeDistances* distances;
    vertex_ID_t dest;
    std::atomic<bool>* found_dest;
    std::atomic<long long>* result_distance;
    pthread_mutex_t* result_mutex;
    int thread_id;
    int num_threads;
};

// Thread function for processing vertices at current level
template<typename Vertex_t, typename Edge_t, GraphType Graph_t>
void* process_level_thread(void* arg) {
    auto* args = static_cast<ThreadArgs<Vertex_t, Edge_t, Graph_t>*>(arg);
    
    vertex_ID_t vertex;
    while (args->current_level->try_pop(vertex)) {
        // Check if we found the destination
        if (vertex == args->dest) {
            long long dist = args->distances->get_distance(vertex);
            pthread_mutex_lock(args->result_mutex);
            if (!args->found_dest->load()) {
                args->found_dest->store(true);
                args->result_distance->store(dist);
            }
            pthread_mutex_unlock(args->result_mutex);
            return nullptr;
        }

        // Process all neighbors of current vertex
        for (const auto& edge : (*args->graph)[vertex]) {
            vertex_ID_t neighbor = edge.dest();
            
            // Try to visit this neighbor
            long long current_dist = args->distances->get_distance(vertex);
            if (args->distances->try_visit(neighbor, current_dist + 1)) {
                args->next_level->push(neighbor);
            }
        }
    }
    
    return nullptr;
}

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

    // Determine number of threads (use hardware concurrency or default to 4)
    int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 4;
    
    // Limit threads based on graph size to avoid overhead
    num_threads = std::min(num_threads, static_cast<int>(graph.num_vertices()));
    num_threads = std::max(num_threads, 1);

    std::cout << "Starting Threaded Breadth-First Search from " << src << " to " << dest 
              << " using " << num_threads << " threads..." << std::endl;

    // Thread-safe data structures
    ThreadSafeQueue<vertex_ID_t> current_level, next_level;
    ThreadSafeDistances distances(graph.num_vertices());
    
    // Result tracking
    std::atomic<bool> found_dest(false);
    std::atomic<long long> result_distance(-1);
    pthread_mutex_t result_mutex = PTHREAD_MUTEX_INITIALIZER;

    // Initialize source vertex
    distances.try_visit(src, 0);
    current_level.push(src);

    // Thread management
    std::vector<pthread_t> threads(num_threads);
    std::vector<ThreadArgs<Vertex_t, Edge_t, Graph_t>> thread_args(num_threads);

    // Main BFS loop - process level by level
    while (!current_level.empty() && !found_dest.load()) {
        // Prepare thread arguments
        for (int i = 0; i < num_threads; ++i) {
            thread_args[i] = {
                &graph,
                &current_level,
                &next_level,
                &distances,
                dest,
                &found_dest,
                &result_distance,
                &result_mutex,
                i,
                num_threads
            };
        }

        // Create and start threads
        for (int i = 0; i < num_threads; ++i) {
            if (pthread_create(&threads[i], nullptr, 
                              process_level_thread<Vertex_t, Edge_t, Graph_t>, 
                              &thread_args[i]) != 0) {
                throw std::runtime_error("Failed to create thread");
            }
        }

        // Wait for all threads to complete
        for (int i = 0; i < num_threads; ++i) {
            pthread_join(threads[i], nullptr);
        }

        // Check if we found the destination
        if (found_dest.load()) {
            long long result = result_distance.load();
            pthread_mutex_destroy(&result_mutex);
            std::cout << "Shortest path found with distance: " << result << std::endl;
            return result;
        }

        // Swap levels for next iteration
        using std::swap;
        swap(current_level, next_level);
        
        // If next level is empty, no path exists
        if (current_level.empty()) {
            break;
        }
    }

    pthread_mutex_destroy(&result_mutex);
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
            std::cout << "Threaded BFS returned: " << dist << std::endl;
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
