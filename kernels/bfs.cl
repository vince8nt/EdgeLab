// OpenCL kernel for parallel breadth-first search
// Uses CSR format: vertex_offsets array and edge_destinations array

// Kernel for initializing BFS data structures
__kernel void bfs_init(
    __global int* distances,
    __global int* visited,
    const uint num_vertices,
    const uint source
) {
    uint global_id = get_global_id(0);
    
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

// Kernel for processing one level of BFS
__kernel void bfs_level(
    __global const uint* vertex_offsets,
    __global const uint* edge_destinations,
    __global const uint* current_level,
    __global uint* next_level,
    __global int* next_level_size,
    __global int* distances,
    __global int* visited,
    const uint num_vertices,
    const int current_distance
) {
    uint global_id = get_global_id(0);
    
    // Bounds check - critical for preventing GPU crashes
    if (global_id >= num_vertices) {
        return;
    }
    
    // Check if this vertex is in the current level
    if (distances[global_id] == current_distance) {
        // Get the range of edges for this vertex
        uint start = vertex_offsets[global_id];
        uint end = vertex_offsets[global_id + 1];
        
        // Process all edges from this vertex
        for (uint i = start; i < end; i++) {
            uint neighbor = edge_destinations[i];
            
            // Critical bounds check
            if (neighbor >= num_vertices) {
                continue;
            }
            
            // Try to visit neighbor atomically
            int expected = -1;
            int desired = current_distance + 1;
            
            // Atomic compare and exchange
            int old_val = atomic_cmpxchg(&distances[neighbor], expected, desired);
            
            if (old_val == -1) {
                // Successfully visited this neighbor
                atomic_xchg(&visited[neighbor], 1);
                
                // Add to next level
                uint next_idx = atomic_inc((__global uint*)next_level_size);
                if (next_idx < num_vertices) {
                    next_level[next_idx] = neighbor;
                }
            }
        }
    }
}

// Kernel for checking if destination is reached
__kernel void check_destination(
    __global const int* distances,
    __global int* found,
    __global int* result_distance,
    const uint destination
) {
    uint global_id = get_global_id(0);
    
    // Only the work item with global_id == destination should check
    if (global_id == destination) {
        if (distances[global_id] != -1) {
            atomic_xchg(found, 1);
            atomic_xchg(result_distance, distances[global_id]);
        }
    }
} 