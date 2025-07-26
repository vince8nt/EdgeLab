// OpenCL kernel for parallel breadth-first search
// Assumes CSR (Compressed Sparse Row) format

typedef unsigned int vertex_ID_t;
typedef unsigned long edge_ID_t;
typedef float weight_t;

// Structure for vertex data in CSR format
typedef struct {
    edge_ID_t edges_begin;
    edge_ID_t edges_end;
} Vertex;

// Structure for edge data
typedef struct {
    vertex_ID_t dest;
    weight_t weight;
} Edge;

// Kernel for processing one level of BFS
__kernel void bfs_level(
    __global const Vertex* vertices,
    __global const Edge* edges,
    __global const vertex_ID_t* current_level,
    __global vertex_ID_t* next_level,
    __global int* next_level_size,
    __global int* distances,
    __global int* visited,
    const vertex_ID_t num_vertices,
    const int current_distance
) {
    int global_id = get_global_id(0);
    int local_id = get_local_id(0);
    int group_size = get_local_size(0);
    
    // Shared memory for local work
    __local int local_next_size;
    __local vertex_ID_t local_next_vertices[256]; // Adjust size as needed
    
    if (local_id == 0) {
        local_next_size = 0;
    }
    barrier(CLK_LOCAL_MEM_FENCE);
    
    // Process vertices from current level
    if (global_id < num_vertices) {
        if (distances[global_id] == current_distance) {
            // Get vertex edges
            Vertex vertex = vertices[global_id];
            edge_ID_t start = vertex.edges_begin;
            edge_ID_t end = vertex.edges_end;
            
            // Process all edges from this vertex
            for (edge_ID_t i = start; i < end; i++) {
                Edge edge = edges[i];
                vertex_ID_t neighbor = edge.dest;
                
                // Try to visit neighbor atomically
                int expected = -1;
                int desired = current_distance + 1;
                
                // Atomic compare and exchange
                int old_val = atomic_cmpxchg(&distances[neighbor], expected, desired);
                
                if (old_val == -1) {
                    // Successfully visited this neighbor
                    atomic_xchg(&visited[neighbor], 1);
                    
                    // Add to next level (local)
                    int local_idx = atomic_inc(&local_next_size);
                    if (local_idx < 256) {
                        local_next_vertices[local_idx] = neighbor;
                    }
                }
            }
        }
    }
    
    barrier(CLK_LOCAL_MEM_FENCE);
    
    // Copy local results to global next level
    if (local_id == 0 && local_next_size > 0) {
        int global_offset = atomic_add(next_level_size, local_next_size);
        for (int i = 0; i < local_next_size && (global_offset + i) < num_vertices; i++) {
            next_level[global_offset + i] = local_next_vertices[i];
        }
    }
}

// Kernel for initializing BFS data structures
__kernel void bfs_init(
    __global int* distances,
    __global int* visited,
    const vertex_ID_t num_vertices,
    const vertex_ID_t source
) {
    int global_id = get_global_id(0);
    
    if (global_id < num_vertices) {
        if (global_id == source) {
            distances[global_id] = 0;
            visited[global_id] = 1;
        } else {
            distances[global_id] = -1;
            visited[global_id] = 0;
        }
    }
}

// Kernel for checking if destination is reached
__kernel void check_destination(
    __global const int* distances,
    __global int* found,
    __global int* result_distance,
    const vertex_ID_t destination
) {
    int global_id = get_global_id(0);
    
    if (global_id == destination && distances[global_id] != -1) {
        atomic_xchg(found, 1);
        atomic_xchg(result_distance, distances[global_id]);
    }
} 